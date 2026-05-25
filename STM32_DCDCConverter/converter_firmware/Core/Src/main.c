/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Push-Pull DC-DC Converter Control Firmware
  *                   - 6-state machine
  *                   - Soft-start PWM duty ramp
  *                   - ADC + IIR filter feedback
  *                   - OVP / UVP protection (immediate MOE off)
  *                   - Proportional voltage regulation with slew-rate limit
  *                   - HC-05 (UART1) remote command & monitoring
  *
  *  Switching topology
  *    - TIM1 (advanced timer) CH1 / CH1N complementary PWM
  *    - Two MOSFETs (Q1, Q2) of push-pull primary are driven by OC1 / OC1N
  *    - With duty < 50% and BDTR.DTG dead-time, Q1 and Q2 conduct alternately
  *      with a guaranteed dead band → primary winding sees ±Vin per half cycle
  *
  *  Architecture
  *    - TIM1   : complementary PWM with BDTR dead-time
  *    - TIM3   : 1ms periodic interrupt → control loop (compute only, no I/O)
  *    - main() : low-priority work (UART log output via flag queue)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* ===== System States ===== */
typedef enum {
    STATE_IDLE       = 0,   // 정지/대기
    STATE_SOFT_START = 1,   // 기동 (duty 점진 상승)
    STATE_RUN        = 2,   // 정상 운전 (피드백 제어)
    STATE_STOPPING   = 3,   // 정상 정지
    STATE_FAULT      = 4,   // 이상 상태 (PWM 차단)
    STATE_RESET      = 5    // FAULT 해제 처리
} SystemState_t;

/* ===== UART buffer ===== */
#define UART_BUFFER_SIZE 32
char    UART_Buffer[UART_BUFFER_SIZE];
uint8_t uart_index = 0;
uint8_t Rx_data[1];

/* ===== PWM parameters ===== */
/* push-pull에서는 두 MOSFET 동시 ON을 절대 피해야 하므로 duty는 50% 미만으로 제한.
 * 데드타임 마진까지 고려하여 안전한 상한을 둔다. */
#define DUTY_TARGET_PERCENT     40U     // 정상 운전 목표 듀티
#define DUTY_MAX_PERCENT        45U     // 피드백 보정 시 최대 듀티 (50% 미만 강제)
#define DUTY_MIN_PERCENT        5U      // 피드백 보정 시 최소 듀티
#define DUTY_SLEW_PERCENT       1U      // 한 step당 최대 변화량 (slew limit)

volatile uint32_t duty_cycle_percent = 0;   // 현재 적용 중인 duty

/* ===== Soft-start ===== */
#define SOFTSTART_STEP_MS       10U     // 10ms마다 1%씩 상승
#define SOFTSTART_STEP_PERCENT  1U

/* ===== ADC / Feedback ===== */
#define ADC_VREF_MV             3300U
#define ADC_RESOLUTION          4095U

/* 피드백 분압회로 후단(ADC 입력) 기준 임계값.
 * ADC saturation(3300mV) 대비 충분한 마진을 두어 OVP 시점에도
 * 측정값이 saturate되지 않도록 OVP를 2200mV로 설정. */
#define V_TARGET_MV             2000U   // 목표 피드백 전압
#define V_OVP_MV                2200U   // 과전압 임계 (목표 대비 +10%)
#define V_UVP_MV                500U    // 저전압 임계 (RUN 중에만 판정)

/* 1차 IIR 필터 계수 (alpha = 0.2) — 1ms 샘플링에서 시정수 약 5ms */
#define IIR_ALPHA_NUM           1
#define IIR_ALPHA_DEN           5
volatile uint32_t v_filtered_mv = 0;
static   uint32_t v_raw_last_mv = 0;    // ADC 실패 시 폴백용

/* 비례 게인: 오차 200mV당 duty 1%.
 * 한 step의 변화량은 DUTY_SLEW_PERCENT(=1%)로 추가 제한된다. */
#define KP_NUM                  1
#define KP_DEN                  200

/* ===== Timing counters (1ms tick 기준) ===== */
#define MONITOR_PERIOD_MS       500U
#define FEEDBACK_PERIOD_MS      10U

volatile uint32_t tick_softstart = 0;
volatile uint32_t tick_feedback  = 0;
volatile uint32_t tick_monitor   = 0;

/* ===== State ===== */
volatile SystemState_t sys_state  = STATE_IDLE;
volatile uint8_t       fault_flag = 0;  // 1: OVP, 2: UVP

/* ===== ISR → main 로그 전달 (블로킹 UART를 ISR에서 제거) ===== */
typedef enum {
    LOG_NONE = 0,
    LOG_STATE_SOFT_START,
    LOG_STATE_RUN,
    LOG_STATE_STOPPING,
    LOG_STATE_IDLE,
    LOG_STATE_RESET_CMD,
    LOG_STATE_RESET_DONE,
    LOG_FAULT_OVP,
    LOG_FAULT_UVP,
    LOG_WARN_NOT_IDLE,
    LOG_WARN_FREQ_NOT_IDLE,
    LOG_MONITOR
} LogEvent_t;

#define LOG_QUEUE_SIZE 16
volatile LogEvent_t log_queue[LOG_QUEUE_SIZE];
volatile uint8_t    log_q_head = 0;
volatile uint8_t    log_q_tail = 0;

static inline void Log_Push(LogEvent_t e) {
    uint8_t next = (log_q_head + 1) % LOG_QUEUE_SIZE;
    if (next != log_q_tail) {           // overflow면 drop
        log_queue[log_q_head] = e;
        log_q_head = next;
    }
}

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
static void     PWM_SetDuty(uint32_t duty_percent);
static void     PWM_OutputEnable(void);
static void     PWM_OutputDisable(void);
static uint32_t ADC_ReadMillivolts(void);
static void     Control_Loop_1ms(void);
static void     TransitToFault(uint8_t reason);
static void     UART_SendString(const char *s);
static void     Log_Drain(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* ----------------------------------------------------------------------------
 * UART 수신 콜백: 명령 파싱
 * --------------------------------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) return;

    char c = Rx_data[0];

    if (c == '\r' || c == '\n') {
        UART_Buffer[uart_index] = '\0';

        if (strcmp(UART_Buffer, "start") == 0) {
            if (sys_state == STATE_IDLE) {
                /* 필터 초기값을 현재 측정값으로 prime → 부팅 직후 보호 오판 방지 */
                v_filtered_mv = ADC_ReadMillivolts();

                duty_cycle_percent = 0;
                tick_softstart = 0;
                PWM_SetDuty(0);
                PWM_OutputEnable();
                sys_state = STATE_SOFT_START;
                Log_Push(LOG_STATE_SOFT_START);
            } else {
                Log_Push(LOG_WARN_NOT_IDLE);
            }
        }
        else if (strcmp(UART_Buffer, "stop") == 0) {
            if (sys_state == STATE_SOFT_START || sys_state == STATE_RUN) {
                sys_state = STATE_STOPPING;
                Log_Push(LOG_STATE_STOPPING);
            }
        }
        else if (strcmp(UART_Buffer, "reset") == 0) {
            if (sys_state == STATE_FAULT) {
                sys_state = STATE_RESET;
                Log_Push(LOG_STATE_RESET_CMD);
            }
        }
        else if (UART_Buffer[0] == '*') {
            /* 주파수 변경은 IDLE에서만 허용. 데드타임은 BDTR.DTG로 고정되어
             * 있으므로 여기서는 ARR만 갱신한다. */
            if (sys_state == STATE_IDLE) {
                uint32_t freq = atoi(&UART_Buffer[1]);
                if (freq > 0) {
                    uint32_t timer_clk = 72000000U;
                    uint32_t period    = timer_clk / freq;
                    if (period >= 100U && period <= 65535U) {
                        TIM1->ARR  = period - 1;
                        TIM1->CCR1 = 0;
                    }
                }
            } else {
                Log_Push(LOG_WARN_FREQ_NOT_IDLE);
            }
        }

        uart_index = 0;
    }
    else {
        if (uart_index < UART_BUFFER_SIZE - 1) {
            UART_Buffer[uart_index++] = c;
        }
    }

    HAL_UART_Receive_IT(&huart1, Rx_data, 1);
}

/* ----------------------------------------------------------------------------
 * TIM3 주기 인터럽트 콜백 (1ms tick) — 제어 루프 호출
 * --------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        Control_Loop_1ms();
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();
    MX_TIM1_Init();          /* PWM + BDTR dead-time는 여기서 모두 설정됨 */
    MX_TIM3_Init();
    MX_USART1_UART_Init();

    PWM_SetDuty(0);

    HAL_UART_Receive_IT(&huart1, Rx_data, 1);
    HAL_TIM_Base_Start_IT(&htim3);

    UART_SendString(
        "=== Push-Pull Converter Control ===\r\n"
        "Commands: start / stop / reset / *<freqHz>\r\n"
        "===================================\r\n"
    );

    while (1)
    {
        /* 인터럽트가 채운 로그 큐를 비우는 것이 main의 주된 일.
         * UART 송신은 절대 ISR에서 하지 않는다. */
        Log_Drain();
        __WFI();
    }
}

/* USER CODE BEGIN 4 */

/* ----------------------------------------------------------------------------
 * 제어 루프 (1ms 주기, TIM3 ISR 컨텍스트)
 *   - I/O 블로킹 금지. 계산과 플래그/큐 갱신만 수행.
 * --------------------------------------------------------------------------*/
static void Control_Loop_1ms(void)
{
    /* --- 1) ADC 측정 + IIR 필터 --- */
    uint32_t v_raw = ADC_ReadMillivolts();
    v_filtered_mv = (IIR_ALPHA_NUM * v_raw +
                     (IIR_ALPHA_DEN - IIR_ALPHA_NUM) * v_filtered_mv)
                    / IIR_ALPHA_DEN;

    /* --- 2) 보호 판정 (PWM이 살아있는 상태에서만) --- */
    if (sys_state == STATE_SOFT_START || sys_state == STATE_RUN) {
        if (v_filtered_mv > V_OVP_MV) {
            TransitToFault(1);
            return;
        }
        if (sys_state == STATE_RUN && v_filtered_mv < V_UVP_MV) {
            TransitToFault(2);
            return;
        }
    }

    /* --- 3) 상태 머신 --- */
    switch (sys_state)
    {
    case STATE_IDLE:
        break;

    case STATE_SOFT_START:
        if (++tick_softstart >= SOFTSTART_STEP_MS) {
            tick_softstart = 0;
            if (duty_cycle_percent < DUTY_TARGET_PERCENT) {
                duty_cycle_percent += SOFTSTART_STEP_PERCENT;
                PWM_SetDuty(duty_cycle_percent);
            } else {
                sys_state = STATE_RUN;
                Log_Push(LOG_STATE_RUN);
            }
        }
        break;

    case STATE_RUN:
        if (++tick_feedback >= FEEDBACK_PERIOD_MS) {
            tick_feedback = 0;

            int32_t error_mv = (int32_t)V_TARGET_MV - (int32_t)v_filtered_mv;
            int32_t delta    = (error_mv * KP_NUM) / KP_DEN;

            /* slew-rate limit: 한 step당 ±DUTY_SLEW_PERCENT 이내 */
            if (delta >  (int32_t)DUTY_SLEW_PERCENT) delta =  (int32_t)DUTY_SLEW_PERCENT;
            if (delta < -(int32_t)DUTY_SLEW_PERCENT) delta = -(int32_t)DUTY_SLEW_PERCENT;

            int32_t new_duty = (int32_t)duty_cycle_percent + delta;
            if (new_duty > (int32_t)DUTY_MAX_PERCENT) new_duty = DUTY_MAX_PERCENT;
            if (new_duty < (int32_t)DUTY_MIN_PERCENT) new_duty = DUTY_MIN_PERCENT;

            duty_cycle_percent = (uint32_t)new_duty;
            PWM_SetDuty(duty_cycle_percent);
        }
        break;

    case STATE_STOPPING:
        PWM_OutputDisable();
        PWM_SetDuty(0);
        duty_cycle_percent = 0;
        sys_state = STATE_IDLE;
        Log_Push(LOG_STATE_IDLE);
        break;

    case STATE_FAULT:
        /* PWM은 이미 차단됨. 'reset' 명령 대기 */
        break;

    case STATE_RESET:
        fault_flag = 0;
        duty_cycle_percent = 0;
        sys_state = STATE_IDLE;
        Log_Push(LOG_STATE_RESET_DONE);
        break;

    default:
        break;
    }

    /* --- 4) 주기 모니터링: 플래그만 세팅, 실제 송신은 main에서 --- */
    if (++tick_monitor >= MONITOR_PERIOD_MS) {
        tick_monitor = 0;
        Log_Push(LOG_MONITOR);
    }
}

/* ----------------------------------------------------------------------------
 * FAULT 전이:
 *   - BDTR.MOE를 즉시 0으로 → 게이트 출력 강제 inactive (가장 빠른 차단)
 *   - 이후 정상 정지 시퀀스로 PWM 멈춤
 * --------------------------------------------------------------------------*/
static void TransitToFault(uint8_t reason)
{
    TIM1->BDTR &= ~TIM_BDTR_MOE;   /* Main Output Enable off (즉시) */
    PWM_OutputDisable();
    PWM_SetDuty(0);
    duty_cycle_percent = 0;
    fault_flag = reason;
    sys_state = STATE_FAULT;

    if      (reason == 1) Log_Push(LOG_FAULT_OVP);
    else if (reason == 2) Log_Push(LOG_FAULT_UVP);
}

/* ----------------------------------------------------------------------------
 * PWM duty 적용
 *   - 데드타임은 BDTR.DTG로 하드웨어가 처리하므로 CCR1에 더하지 않는다.
 *   - duty < 50%에서 OC1/OC1N은 데드타임을 두고 거의 교대로 ON되어
 *     push-pull 1차측 두 MOSFET을 안전하게 구동한다.
 * --------------------------------------------------------------------------*/
static void PWM_SetDuty(uint32_t duty_percent)
{
    if (duty_percent > DUTY_MAX_PERCENT) duty_percent = DUTY_MAX_PERCENT;
    uint32_t period = TIM1->ARR + 1U;
    TIM1->CCR1 = (period * duty_percent) / 100U;
}

/* ----------------------------------------------------------------------------
 * PWM 출력 enable / disable
 * --------------------------------------------------------------------------*/
static void PWM_OutputEnable(void)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    TIM1->BDTR |= TIM_BDTR_MOE;            /* FAULT에서 MOE를 껐을 수 있으므로 복구 */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_Base_Start(&htim1);
}

static void PWM_OutputDisable(void)
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim1);
}

/* ----------------------------------------------------------------------------
 * ADC 단일 측정 (mV). 실패 시 마지막 raw 값 반환 → IIR 한 step만 정체.
 *
 * 변환 시간 추정: ADC clock = 72MHz/6 = 12MHz, 71.5+12.5 = 84 cycles
 *   → 약 7µs. 타임아웃 5ms은 HAL_GetTick 경계 이슈를 피하기 위한 안전 마진.
 * --------------------------------------------------------------------------*/
static uint32_t ADC_ReadMillivolts(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 5) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return v_raw_last_mv;
    }
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    v_raw_last_mv = (ADC_VREF_MV * raw) / ADC_RESOLUTION;
    return v_raw_last_mv;
}

/* ----------------------------------------------------------------------------
 * 로그 출력 (main 루프에서만 호출됨 → UART 블로킹 허용)
 * --------------------------------------------------------------------------*/
static void Log_Drain(void)
{
    while (log_q_tail != log_q_head)
    {
        LogEvent_t e = log_queue[log_q_tail];
        log_q_tail = (log_q_tail + 1) % LOG_QUEUE_SIZE;

        switch (e) {
        case LOG_STATE_SOFT_START:    UART_SendString("[STATE] SOFT_START\r\n"); break;
        case LOG_STATE_RUN:           UART_SendString("[STATE] RUN\r\n"); break;
        case LOG_STATE_STOPPING:      UART_SendString("[STATE] STOPPING\r\n"); break;
        case LOG_STATE_IDLE:          UART_SendString("[STATE] IDLE\r\n"); break;
        case LOG_STATE_RESET_CMD:     UART_SendString("[STATE] RESET\r\n"); break;
        case LOG_STATE_RESET_DONE:    UART_SendString("[STATE] IDLE (reset complete)\r\n"); break;
        case LOG_FAULT_OVP:           UART_SendString("[FAULT] OVP - PWM disabled\r\n"); break;
        case LOG_FAULT_UVP:           UART_SendString("[FAULT] UVP - PWM disabled\r\n"); break;
        case LOG_WARN_NOT_IDLE:       UART_SendString("[WARN] start ignored (not IDLE)\r\n"); break;
        case LOG_WARN_FREQ_NOT_IDLE:  UART_SendString("[WARN] freq change only in IDLE\r\n"); break;
        case LOG_MONITOR: {
            char msg[80];
            sprintf(msg, "[MON] state=%d duty=%lu%% Vfb=%lumV\r\n",
                    (int)sys_state,
                    (unsigned long)duty_cycle_percent,
                    (unsigned long)v_filtered_mv);
            UART_SendString(msg);
            break;
        }
        default: break;
        }
    }
}

static void UART_SendString(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
}

/* USER CODE END 4 */


/* === 이하 CubeMX 자동 생성 초기화 함수들 ===================================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    /* 분압회로 소스 임피던스를 고려한 가장 긴 샘플링 시간 */
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

/* ----------------------------------------------------------------------------
 * TIM1: complementary PWM with BDTR dead-time
 *
 *   PWM freq: 72MHz / (Period+1) ≈ 133 kHz (Period=540)
 *
 *   Dead-time는 BDTR.DTG의 비선형 인코딩을 사용한다.
 *     DTG[7:5] = 0xx → DT = DTG[7:0] × t_DTS
 *     본 설정에서 CR1.CKD = 00 → t_DTS = 1/CK_INT = 1/72MHz ≒ 13.9 ns
 *     DTG = 16 → 약 222 ns 데드타임 (스위칭 주기 7.5µs의 약 3%)
 *
 *   이 값은 BDTR.DTG의 선형 영역(DTG < 128)에서 직관적으로 계산되며,
 *   런타임 코드에서 DTG를 임의로 변경하지 않는다.
 * --------------------------------------------------------------------------*/
static void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 540;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigOC.OCMode      = TIM_OCMODE_PWM1;
    sConfigOC.Pulse       = 0;
    sConfigOC.OCPolarity  = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode  = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;   /* MOE off 시 게이트 LOW */
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 16;        /* DTG=16 → 약 222 ns */
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) Error_Handler();

    HAL_TIM_MspPostInit(&htim1);
}

/* TIM3: 1ms tick (제어 루프 인터럽트)
 *   APB1 timer clock = 72MHz, Prescaler 71 → 1MHz, Period 999 → 1ms */
static void MX_TIM3_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
