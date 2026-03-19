/* main.c */
#include "main.h"
#include "keypad.h" // 수정된 keypad.h 포함

/* --- [1] 설정 및 상수 --- */
#define MAX_FLOOR 4

// FS90R PWM 값 (1MHz 타이머, Prescaler조정으로 1tick=1us 가정)
#define SERVO_STOP 1500
#define SERVO_UP   1420  // 속도 조절: 1500에 가까울수록 느림
#define SERVO_DOWN 1550  // 방향 반대면 UP/DOWN 값 교체

// 자동 리셋 시간 (ms)
#define IDLE_TIMEOUT 10000 // 10초 동안 동작 없으면 1층으로

/* --- [2] 전역 변수 (상태 관리) --- */
typedef enum {
    STATE_IDLE,
    STATE_UP,
    STATE_DOWN,
    STATE_STOP_OPEN
} ElevatorState;

ElevatorState current_state = STATE_IDLE;
int current_floor = 1;     // 현재 층 (센서로 업데이트)
int last_valid_floor = 1;  // 센서가 꺼져도 기억하는 마지막 층

// 요청 배열 [층][종류]
// 종류: 0=내부(Inner), 1=외부상행(Up), 2=외부하행(Down)
int requests[MAX_FLOOR + 1][3] = {0};

uint32_t last_idle_time = 0; // 리셋 타이머용

extern TIM_HandleTypeDef htim2; // 큐브MX에서 설정한 타이머 핸들

/* --- [3] 함수 프로토타입 --- */
void Motor_Control(int pwm);
int Read_Floor_Sensor(void);
void Process_Keypad(void);
void Algorithm_Loop(void);
int Check_Requests_Direction(int dir); // 해당 방향에 요청이 있는지 확인

/* --- [4] 메인 함수 --- */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init(); // 타이머 초기화

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  Motor_Control(SERVO_STOP);
  last_idle_time = HAL_GetTick();

  while (1)
  {
    // 1. 센서 읽기 & 위치 업데이트
    int sensor = Read_Floor_Sensor();
    if (sensor > 0) {
        current_floor = sensor;
        last_valid_floor = sensor;
    }

    // 2. 키패드 입력 처리 (요청 등록)
    Process_Keypad();

    // 3. 엘리베이터 알고리즘 (핵심 로직)
    Algorithm_Loop();

    HAL_Delay(20); // 루프 주기 20ms (디바운싱 역할 겸함)
  }
}

/* --- [5] 기능 구현 --- */

// 모터 구동 래퍼 함수
void Motor_Control(int pwm) {
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
}

// 센서 값 읽기 (1~4층, 없으면 0)
// 큐브MX에서 핀 이름을 SENSOR_1F 등으로 설정했다고 가정
int Read_Floor_Sensor(void) {
    if (HAL_GPIO_ReadPin(SENSOR_1F_GPIO_Port, SENSOR_1F_Pin) == GPIO_PIN_RESET) return 1;
    if (HAL_GPIO_ReadPin(SENSOR_2F_GPIO_Port, SENSOR_2F_Pin) == GPIO_PIN_RESET) return 2;
    if (HAL_GPIO_ReadPin(SENSOR_3F_GPIO_Port, SENSOR_3F_Pin) == GPIO_PIN_RESET) return 3;
    if (HAL_GPIO_ReadPin(SENSOR_4F_GPIO_Port, SENSOR_4F_Pin) == GPIO_PIN_RESET) return 4;
    return 0;
}

// 키패드 입력을 요청 배열에 매핑
void Process_Keypad(void) {
    char key = read_keypad();
    if (key == '\0') return;

    // 리셋 버튼 (*)
    if (key == '*') {
        // 모든 요청 초기화 후 1층 내부 버튼 누른 효과
        for(int f=1; f<=MAX_FLOOR; f++) { requests[f][0]=0; requests[f][1]=0; requests[f][2]=0; }
        requests[1][0] = 1; 
        last_idle_time = HAL_GetTick(); // 타이머 갱신
        return;
    }

    // 키 매핑 (배열 인덱스: 0=내부, 1=UP, 2=DOWN)
    switch(key) {
        // Row 1: 내부 버튼
        case '1': requests[1][0] = 1; break;
        case '2': requests[2][0] = 1; break;
        case '3': requests[3][0] = 1; break;
        case 'A': requests[4][0] = 1; break; // 4층 내부

        // Row 2: 외부 UP
        case '4': requests[1][1] = 1; break; // 1층 UP
        case '5': requests[2][1] = 1; break; // 2층 UP
        case '6': requests[3][1] = 1; break; // 3층 UP

        // Row 3: 외부 DOWN
        case '7': requests[2][2] = 1; break; // 2층 DOWN
        case '8': requests[3][2] = 1; break; // 3층 DOWN
        case '9': requests[4][2] = 1; break; // 4층 DOWN
    }
    
    // 키가 눌리면 리셋 타이머 초기화 (사용 중임)
    last_idle_time = HAL_GetTick();
}

// 방향 확인 헬퍼 함수: dir 방향(1:위, -1:아래)에 처리해야 할 요청이 있는가?
int Check_Requests_Direction(int dir) {
    if (dir == 1) { // 위쪽에 요청이 있는가?
        for (int f = current_floor + 1; f <= MAX_FLOOR; f++) {
            if (requests[f][0] || requests[f][1] || requests[f][2]) return 1;
        }
    } else { // 아래쪽에 요청이 있는가?
        for (int f = current_floor - 1; f >= 1; f--) {
            if (requests[f][0] || requests[f][1] || requests[f][2]) return 1;
        }
    }
    return 0;
}

// 핵심 알고리즘 (Collective Selective)
void Algorithm_Loop(void) {
    int should_stop = 0;

    switch (current_state) {
    case STATE_IDLE:
        Motor_Control(SERVO_STOP);
        
        // 1. 요청 스캔 (위/아래 중 어디로 갈까?)
        // 현재 층 요청 처리
        if (requests[current_floor][0] || requests[current_floor][1] || requests[current_floor][2]) {
            current_state = STATE_STOP_OPEN;
            break;
        }

        // 다른 층 요청 확인
        if (Check_Requests_Direction(1)) { // 위에 있으면
            current_state = STATE_UP;
        } else if (Check_Requests_Direction(-1)) { // 아래에 있으면
            current_state = STATE_DOWN;
        } 
        
        // 자동 리셋 (오래 쉬면 1층으로)
        if (current_floor != 1 && (HAL_GetTick() - last_idle_time > IDLE_TIMEOUT)) {
            requests[1][0] = 1; // 1층 가기 명령 생성
        }
        break;

    case STATE_UP:
        Motor_Control(SERVO_UP);
        
        // 센서 감지 시 판단
        if (Read_Floor_Sensor() == current_floor) {
            // 멈춰야 하는가?
            // 1. 이 층에 '내부' 요청이나 'UP' 요청이 있는 경우
            if (requests[current_floor][0] || requests[current_floor][1]) should_stop = 1;
            
            // 2. 더 이상 위로 갈 요청이 없고, 이 층에 'DOWN' 요청이 있는 경우 (꼭대기 회차)
            else if (!Check_Requests_Direction(1) && requests[current_floor][2]) should_stop = 1;
            
            // 3. 더 이상 위로 갈 요청이 아예 없고, 아무 요청도 없으면 (잘못된 진입 방지)
            else if (!Check_Requests_Direction(1) && !requests[current_floor][2]) should_stop = 1;

            if (should_stop) {
                current_state = STATE_STOP_OPEN;
            }
        }
        break;

    case STATE_DOWN:
        Motor_Control(SERVO_DOWN);

        if (Read_Floor_Sensor() == current_floor) {
            // 멈춰야 하는가?
            // 1. 이 층에 '내부' 요청이나 'DOWN' 요청이 있는 경우
            if (requests[current_floor][0] || requests[current_floor][2]) should_stop = 1;
            
            // 2. 더 이상 아래로 갈 요청이 없고, 이 층에 'UP' 요청이 있는 경우 (바닥 회차)
            else if (!Check_Requests_Direction(-1) && requests[current_floor][1]) should_stop = 1;

            if (should_stop) {
                current_state = STATE_STOP_OPEN;
            }
        }
        break;

    case STATE_STOP_OPEN:
        Motor_Control(SERVO_STOP);
        
        // 요청 처리 완료 (Clearing Requests)
        requests[current_floor][0] = 0; // 내부 요청 삭제
        
        // 방향에 따른 외부 요청 삭제 로직 (정교함 필요)
        // 위로 가던 중이었거나, 앞으로 위로 갈거면 UP 요청 삭제
        // 아래로 가던 중이었거나, 앞으로 아래로 갈거면 DOWN 요청 삭제
        // 단순화를 위해: 일단 도착하면 해당 층 UP/DOWN 모두 삭제 (실제로는 방향 따져야 함)
        // 여기서는 '방향 유지'를 위해 조건을 따짐:
        
        // 다음 갈 곳 결정
        int go_up = Check_Requests_Direction(1);
        int go_down = Check_Requests_Direction(-1);

        if (go_up) requests[current_floor][1] = 0; // 위로 갈거면 UP버튼 끔
        else if (go_down) requests[current_floor][2] = 0; // 아래로 갈거면 DOWN버튼 끔
        else { // 갈 곳 없으면 둘 다 끔 (종착지)
            requests[current_floor][1] = 0;
            requests[current_floor][2] = 0;
        }

        // 문 열림 시뮬레이션 (2초 대기)
        HAL_Delay(2000);
        last_idle_time = HAL_GetTick(); // 대기 시간 초기화

        // 다음 상태 결정
        if (go_up) current_state = STATE_UP;
        else if (go_down) current_state = STATE_DOWN;
        else current_state = STATE_IDLE;
        break;
    }
}