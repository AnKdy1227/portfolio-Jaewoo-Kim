#include "stm32f4xx_hal.h"
#include "ssd1306.h"        /* OLED 드라이버 (I2C 기반)            */
#include "ps2_controller.h" /* PS2 컨트롤러 드라이버 (GPIO 비트뱅) */
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* ==========================================================================
 *  1. 매크로 / 상수 정의
 * ========================================================================*/

/* --- 초음파 센서 (HC-SR04) ---------------------------------------------- */
#define US_TIMEOUT_US        30000U   /* echo 핀 타임아웃 (30ms)            */
#define US_INVALID_DIST      9999U    /* 측정 실패 시 반환값                */
#define US_FILTER_SAMPLES    3U       /* 3회 측정 평균 필터                 */

/* --- 거리 임계값 (cm) : 2단계 분리 ------------------------------------- */
#define DIST_WARNING_CM      45U      /* 경고 거리 → 감속 진입             */
#define DIST_DANGER_CM       25U      /* 위험 거리 → 정지 후 회피          */
#define DIST_REAR_SAFE_CM    20U      /* 후진 가능 최소 후방 여유          */

/* --- 상태 유지 시간 (ms) : 회피 동작 안정화 ---------------------------- */
#define STATE_HOLD_TURN_MS   600U     /* 좌/우회전 최소 유지 시간          */
#define STATE_HOLD_BACK_MS   500U     /* 후진 최소 유지 시간                */
#define STATE_HOLD_STOP_MS   150U     /* 정지(전이) 최소 유지 시간          */

/* --- 모터 PWM duty (0~999, TIM PWM ARR 기준) ---------------------------- */
#define PWM_SPEED_FORWARD    600U
#define PWM_SPEED_SLOW       300U     /* 감속 전진 duty                     */
#define PWM_SPEED_TURN       700U
#define PWM_SPEED_BACK       450U

/* --- 경로 기록 버퍼 ----------------------------------------------------- */
#define PATH_BUFFER_SIZE     256U

/* ==========================================================================
 *  2. 타입 정의
 * ========================================================================*/

/* 주행 상태 머신 : 자소서에 명시된 6개 상태 ------------------------------ */
typedef enum {
    STATE_STOP = 0,
    STATE_FORWARD,
    STATE_SLOW_FORWARD,   /* 경고 거리 진입 시 감속 전진           */
    STATE_TURN_LEFT,
    STATE_TURN_RIGHT,
    STATE_BACKWARD,
} drive_state_t;

/* 동작 모드 -------------------------------------------------------------- */
typedef enum {
    MODE_MANUAL = 0,      /* PS2 컨트롤러 수동 조종                */
    MODE_AUTO,            /* 자율주행                              */
    MODE_RETURN,          /* 경로 복귀                             */
} robot_mode_t;

/* 4방향 거리 측정 결과 --------------------------------------------------- */
typedef struct {
    uint16_t front;
    uint16_t rear;
    uint16_t left;
    uint16_t right;
} distance_t;

/* 경로 기록 버퍼 (자율주행 중 수행한 명령을 순서대로 저장) -------------- */
typedef struct {
    drive_state_t buf[PATH_BUFFER_SIZE];
    uint16_t      idx;
} path_log_t;

/* ==========================================================================
 *  3. 전역 변수
 * ========================================================================*/

static drive_state_t g_state       = STATE_STOP;
static drive_state_t g_prev_state  = STATE_STOP;
static uint32_t      g_state_enter_tick = 0;     /* 상태 진입 시각 */

static robot_mode_t  g_mode        = MODE_MANUAL;
static bool          g_speaker_on  = false;
static bool          g_brush_on    = false;

static path_log_t    g_path        = { .idx = 0 };

/* HAL 핸들 (CubeMX 생성 가정) ------------------------------------------- */
extern I2C_HandleTypeDef  hi2c1;   /* OLED                            */
extern TIM_HandleTypeDef  htim2;   /* 주행 모터 PWM (ch1~ch4)         */
extern TIM_HandleTypeDef  htim3;   /* 브러시 모터 PWM                 */
extern TIM_HandleTypeDef  htim5;   /* us 단위 타이머 (초음파 측정용)  */

/* ==========================================================================
 *  4. 저수준 유틸 : 마이크로초 지연 / 타이머 기반 거리 측정
 * ========================================================================*/

static inline uint32_t micros(void)
{
    return __HAL_TIM_GET_COUNTER(&htim5);
}

static void delay_us(uint32_t us)
{
    uint32_t start = micros();
    while ((micros() - start) < us) { /* busy wait */ }
}

/* 초음파 1회 측정. 실패 시 US_INVALID_DIST 반환 ------------------------- */
static uint16_t ultrasonic_measure_once(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                                        GPIO_TypeDef *echo_port, uint16_t echo_pin)
{
    /* 1) Trig 10us 펄스 ------------------------------------------------- */
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

    /* 2) Echo HIGH 대기 (타임아웃 포함) -------------------------------- */
    uint32_t t_wait = micros();
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_RESET) {
        if ((micros() - t_wait) > US_TIMEOUT_US) return US_INVALID_DIST;
    }

    /* 3) Echo HIGH 폭 측정 (타임아웃 포함) ----------------------------- */
    uint32_t t_start = micros();
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET) {
        if ((micros() - t_start) > US_TIMEOUT_US) return US_INVALID_DIST;
    }
    uint32_t pulse_us = micros() - t_start;

    /* 4) 거리 환산 : 음속 340m/s 기준 cm 단위 -------------------------- */
    /*    distance(cm) = pulse_us * 0.0343 / 2 = pulse_us / 58            */
    return (uint16_t)(pulse_us / 58U);
}

/* ==========================================================================
 *  5. 3회 측정 평균 필터 (타임아웃 실패값은 평균에서 제외)
 *     - 자소서: "초음파 센서값에 3회 측정 평균 필터를 적용해
 *               노이즈에 의한 오판단 감소, 측정이 실패한 값은 평균에서 제외"
 * ========================================================================*/
static uint16_t ultrasonic_read_filtered(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                                         GPIO_TypeDef *echo_port, uint16_t echo_pin)
{
    uint32_t sum   = 0;
    uint8_t  valid = 0;

    for (uint8_t i = 0; i < US_FILTER_SAMPLES; i++) {
        uint16_t d = ultrasonic_measure_once(trig_port, trig_pin, echo_port, echo_pin);
        if (d != US_INVALID_DIST && d > 0 && d < 400U) {
            sum   += d;
            valid += 1;
        }
        HAL_Delay(5);   /* 다음 트리거 전 에코 잔향 해소 */
    }

    if (valid == 0) return US_INVALID_DIST;   /* 모두 실패 → 무한대 취급 */
    return (uint16_t)(sum / valid);
}

/* 4방향 거리 한 번에 읽기 ----------------------------------------------- */
static void read_all_distances(distance_t *d)
{
    d->front = ultrasonic_read_filtered(TRIG_F_PORT, TRIG_F_PIN, ECHO_F_PORT, ECHO_F_PIN);
    d->rear  = ultrasonic_read_filtered(TRIG_R_PORT, TRIG_R_PIN, ECHO_R_PORT, ECHO_R_PIN);
    d->left  = ultrasonic_read_filtered(TRIG_L_PORT, TRIG_L_PIN, ECHO_L_PORT, ECHO_L_PIN);
    d->right = ultrasonic_read_filtered(TRIG_RT_PORT, TRIG_RT_PIN, ECHO_RT_PORT, ECHO_RT_PIN);
}

/* ==========================================================================
 *  6. 모터 제어 (TIM PWM 기반)
 *     - htim2 CH1/CH2 : 좌측 주행 모터 (정방향/역방향 듀얼 채널)
 *     - htim2 CH3/CH4 : 우측 주행 모터
 *     - htim3 CH1     : 브러시 모터
 * ========================================================================*/
static void motor_set_left(int16_t duty)   /* duty > 0 전진, < 0 후진 */
{
    if (duty >= 0) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    } else {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, -duty);
    }
}
static void motor_set_right(int16_t duty)
{
    if (duty >= 0) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, duty);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
    } else {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, -duty);
    }
}
static void brush_set(bool on)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, on ? 700 : 0);
    g_brush_on = on;
}

/* ==========================================================================
 *  7. 상태 머신 : 6개 상태에 대한 모터 출력 매핑
 * ========================================================================*/
static void apply_state_output(drive_state_t s)
{
    switch (s) {
        case STATE_FORWARD:
            motor_set_left ( (int16_t)PWM_SPEED_FORWARD);
            motor_set_right( (int16_t)PWM_SPEED_FORWARD);
            break;
        case STATE_SLOW_FORWARD:
            motor_set_left ( (int16_t)PWM_SPEED_SLOW);
            motor_set_right( (int16_t)PWM_SPEED_SLOW);
            break;
        case STATE_TURN_LEFT:                  /* 제자리 좌회전 */
            motor_set_left (-(int16_t)PWM_SPEED_TURN);
            motor_set_right( (int16_t)PWM_SPEED_TURN);
            break;
        case STATE_TURN_RIGHT:                 /* 제자리 우회전 */
            motor_set_left ( (int16_t)PWM_SPEED_TURN);
            motor_set_right(-(int16_t)PWM_SPEED_TURN);
            break;
        case STATE_BACKWARD:
            motor_set_left (-(int16_t)PWM_SPEED_BACK);
            motor_set_right(-(int16_t)PWM_SPEED_BACK);
            break;
        case STATE_STOP:
        default:
            motor_set_left (0);
            motor_set_right(0);
            break;
    }
}

/* 자율주행 중 상태로 진입할 때 호출되는 공통 함수
 *  - 같은 상태로의 재진입은 무시 (튀는 전환 방지)
 *  - 진입 시각 기록 → state hold 판정 기준
 *  - 경로 기록은 MODE_AUTO에서만 누적                                    */
static void transition_to(drive_state_t next)
{
    if (next == g_state) return;            /* 동일 상태 재진입 차단 */

    g_prev_state       = g_state;
    g_state            = next;
    g_state_enter_tick = HAL_GetTick();

    apply_state_output(next);

    /* 경로 기록 (자율주행 중에만, 정지/감속전진은 제외) -------------- */
    if (g_mode == MODE_AUTO && g_path.idx < PATH_BUFFER_SIZE) {
        if (next == STATE_FORWARD || next == STATE_BACKWARD ||
            next == STATE_TURN_LEFT || next == STATE_TURN_RIGHT) {
            g_path.buf[g_path.idx++] = next;
        }
    }
}

/* 현재 상태가 최소 유지 시간을 충족했는지 확인
 *  - 자소서: "한 번 회피 동작을 시작하면 일정 시간 동안 해당 상태를
 *             유지하도록 해 회피 동작이 완료되기 전에 다른 방향으로
 *             전환되는 일이 없도록 함"                                 */
static bool state_hold_satisfied(void)
{
    uint32_t elapsed = HAL_GetTick() - g_state_enter_tick;
    switch (g_state) {
        case STATE_TURN_LEFT:
        case STATE_TURN_RIGHT:  return elapsed >= STATE_HOLD_TURN_MS;
        case STATE_BACKWARD:    return elapsed >= STATE_HOLD_BACK_MS;
        case STATE_STOP:        return elapsed >= STATE_HOLD_STOP_MS;
        default:                return true;     /* 전진/감속 전진은 항상 전환 가능 */
    }
}

/* ==========================================================================
 *  8. 자율주행 알고리즘
 *     - 거리 2단계 분리 (DANGER → 회피, WARNING → 감속 전진, 그 외 전진)
 *     - 후진 동작은 후방 센서 선확인
 *     - 좌우 비교로 더 넓은 공간 쪽으로 회전
 * ========================================================================*/
static void autonomous_step(const distance_t *d)
{
    /* 현재 회피 동작이 hold 시간 안에 있으면 절대 전환하지 않음 ------- */
    if (!state_hold_satisfied()) return;

    bool danger_front  = (d->front  < DIST_DANGER_CM);
    bool danger_left   = (d->left   < DIST_DANGER_CM);
    bool danger_right  = (d->right  < DIST_DANGER_CM);
    bool warn_front    = (d->front  < DIST_WARNING_CM);

    /* ---------- (1) 위험 거리 : 정지 후 회피 ------------------------- */
    if (danger_front || danger_left || danger_right) {

        /* 회피 동작 진입 직전 짧은 정지 (관성 제거) ------------------- */
        if (g_state == STATE_FORWARD || g_state == STATE_SLOW_FORWARD) {
            transition_to(STATE_STOP);
            return;
        }

        /* 좌우 비교 → 더 넓은 쪽으로 회전 (자소서 명시 사항) --------- */
        if (danger_front || (danger_left && danger_right)) {

            bool left_open  = (d->left  > d->right) && !danger_left;
            bool right_open = (d->right > d->left ) && !danger_right;

            if (left_open) {
                transition_to(STATE_TURN_LEFT);
            } else if (right_open) {
                transition_to(STATE_TURN_RIGHT);
            } else {
                /* 좌우 모두 막힘 → 후진 진입 전 후방 안전 선확인 ----- */
                if (d->rear > DIST_REAR_SAFE_CM) {
                    transition_to(STATE_BACKWARD);
                } else {
                    /* 후방도 막힘 → 멈춰서 다음 사이클 재판단 --------- */
                    transition_to(STATE_STOP);
                }
            }
        }
        else if (danger_left) {       /* 좌측만 막힘 → 우회전          */
            transition_to(STATE_TURN_RIGHT);
        }
        else if (danger_right) {      /* 우측만 막힘 → 좌회전          */
            transition_to(STATE_TURN_LEFT);
        }
        return;
    }

    /* ---------- (2) 경고 거리 : 감속 전진 ---------------------------- */
    if (warn_front) {
        transition_to(STATE_SLOW_FORWARD);
        return;
    }

    /* ---------- (3) 모두 안전 : 정상 전진 ---------------------------- */
    transition_to(STATE_FORWARD);
}

/* ==========================================================================
 *  9. 경로 복귀 모드
 *     - 자율주행 중 기록한 명령을 역순으로 꺼내 "반대 명령"으로 실행
 *     - 한 명령당 충분한 시간 유지 후 다음 명령으로 진행
 * ========================================================================*/
static drive_state_t inverse_of(drive_state_t s)
{
    switch (s) {
        case STATE_FORWARD:    return STATE_BACKWARD;
        case STATE_BACKWARD:   return STATE_FORWARD;
        case STATE_TURN_LEFT:  return STATE_TURN_RIGHT;
        case STATE_TURN_RIGHT: return STATE_TURN_LEFT;
        default:               return STATE_STOP;
    }
}

static void return_step(void)
{
    if (!state_hold_satisfied()) return;     /* 현재 명령 충분히 수행 */

    if (g_path.idx == 0) {                   /* 출발 지점 복귀 완료    */
        transition_to(STATE_STOP);
        g_mode = MODE_MANUAL;                /* 복귀 후 수동 모드      */
        return;
    }

    drive_state_t recorded = g_path.buf[--g_path.idx];
    transition_to(inverse_of(recorded));
}

/* ==========================================================================
 *  10. 수동 모드 (PS2 컨트롤러 입력)
 * ========================================================================*/
static void manual_step(const ps2_input_t *p)
{
    if      (p->btn_up)      transition_to(STATE_FORWARD);
    else if (p->btn_down)    transition_to(STATE_BACKWARD);
    else if (p->btn_left)    transition_to(STATE_TURN_LEFT);
    else if (p->btn_right)   transition_to(STATE_TURN_RIGHT);
    else                     transition_to(STATE_STOP);

    if (p->btn_triangle)     brush_set(true);
    if (p->btn_cross)        brush_set(false);
}

/* ==========================================================================
 *  11. 경고 모드 (조도/근접 기반 부저)
 *     - 어느 방향이든 위험 거리 이내 진입 시 부저 ON
 * ========================================================================*/
static void warning_step(const distance_t *d)
{
    if (!g_speaker_on) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        return;
    }
    bool near = (d->front < DIST_DANGER_CM) || (d->rear  < DIST_DANGER_CM) ||
                (d->left  < DIST_DANGER_CM) || (d->right < DIST_DANGER_CM);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, near ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ==========================================================================
 *  12. OLED 출력 : 4방향 거리 + 현재 모드/상태
 * ========================================================================*/
static const char *state_name(drive_state_t s)
{
    switch (s) {
        case STATE_FORWARD:      return "FORWARD";
        case STATE_SLOW_FORWARD: return "SLOW   ";
        case STATE_TURN_LEFT:    return "LEFT   ";
        case STATE_TURN_RIGHT:   return "RIGHT  ";
        case STATE_BACKWARD:     return "BACK   ";
        default:                 return "STOP   ";
    }
}
static const char *mode_name(robot_mode_t m)
{
    switch (m) {
        case MODE_AUTO:   return "AUTO";
        case MODE_RETURN: return "RTN ";
        default:          return "MAN ";
    }
}
static void oled_render(const distance_t *d)
{
    char line[24];
    ssd1306_Fill(Black);

    snprintf(line, sizeof(line), "MODE:%s ST:%s", mode_name(g_mode), state_name(g_state));
    ssd1306_SetCursor(0, 0);  ssd1306_WriteString(line, Font_6x8, White);

    snprintf(line, sizeof(line), "F:%3u  B:%3u", d->front, d->rear);
    ssd1306_SetCursor(0, 16); ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "L:%3u  R:%3u", d->left, d->right);
    ssd1306_SetCursor(0, 30); ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "PATH:%3u BRUSH:%s",
             g_path.idx, g_brush_on ? "ON" : "OFF");
    ssd1306_SetCursor(0, 50); ssd1306_WriteString(line, Font_6x8, White);

    ssd1306_UpdateScreen();
}

/* ==========================================================================
 *  13. PS2 버튼으로 모드 전환 (엣지 검출)
 * ========================================================================*/
static void mode_switch_handler(const ps2_input_t *p)
{
    static bool prev_select = false;
    static bool prev_l1     = false;
    static bool prev_start  = false;

    /* SELECT : MANUAL ↔ AUTO ----------------------------------------- */
    if (p->btn_select && !prev_select) {
        if (g_mode == MODE_AUTO) {
            g_mode = MODE_MANUAL;
            transition_to(STATE_STOP);
        } else {
            g_mode = MODE_AUTO;
            g_path.idx = 0;        /* 새 자율주행 시작 시 경로 초기화 */
        }
    }
    /* L1 : 복귀 모드 시작 -------------------------------------------- */
    if (p->btn_l1 && !prev_l1) {
        if (g_path.idx > 0) {
            g_mode = MODE_RETURN;
            transition_to(STATE_STOP);
        }
    }
    /* START : 경고 부저 on/off --------------------------------------- */
    if (p->btn_start && !prev_start) {
        g_speaker_on = !g_speaker_on;
    }

    prev_select = p->btn_select;
    prev_l1     = p->btn_l1;
    prev_start  = p->btn_start;
}

/* ==========================================================================
 *  14. main loop : 통합 관리 (자율주행 / 수동 / 복귀 / 경고)
 * ========================================================================*/
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* CubeMX init 함수들 (생성 코드) ---------------------------------- */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM5_Init();

    /* PWM/타이머 시작 ------------------------------------------------- */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_Base_Start(&htim5);              /* us 카운터 */

    /* 주변 드라이버 초기화 ------------------------------------------- */
    ssd1306_Init();
    ps2_init();

    transition_to(STATE_STOP);

    /* ---------------- 메인 루프 ------------------------------------- */
    while (1) {
        /* (1) 입력 수집 ---------------------------------------------- */
        ps2_input_t  p;
        distance_t   d;
        ps2_read(&p);
        read_all_distances(&d);

        /* (2) 모드 전환 입력 처리 ------------------------------------ */
        mode_switch_handler(&p);

        /* (3) 모드별 동작 ------------------------------------------- */
        switch (g_mode) {
            case MODE_AUTO:    autonomous_step(&d); break;
            case MODE_RETURN:  return_step();        break;
            case MODE_MANUAL:
            default:           manual_step(&p);      break;
        }

        /* (4) 경고 부저 ---------------------------------------------- */
        warning_step(&d);

        /* (5) OLED 갱신 ---------------------------------------------- */
        oled_render(&d);

        HAL_Delay(20);    /* 약 50Hz 주기로 통합 제어 루프 동작 */
    }
}
