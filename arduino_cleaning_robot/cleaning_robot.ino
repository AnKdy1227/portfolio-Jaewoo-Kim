#include <AFMotor.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PS2X_lib.h"

// 모터 설정
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// OLED 디스플레이 설정
#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

// 초음파 센서 핀 설정
#define trig_1 22
#define echo_1 23
#define trig_2 24
#define echo_2 25
#define trig_3 26
#define echo_3 27
#define trig_4 28
#define echo_4 29

// 스피커 핀
#define SPEAKER_PIN 44  // 스피커 핀을 46으로 변경

// PS2 컨트롤러 핀 설정
#define PS2_DAT 38
#define PS2_CMD 39
#define PS2_SEL 40
#define PS2_CLK 41

PS2X ps2x;  // PS2 컨트롤러 객체 생성
int error = 0;
byte vibrate = 0;
char currentCmd = 's';  // 현재 명령 저장

void setup() {
  Serial.begin(9600);  // Serial 모니터 시작

  // 모터 초기화
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);

  // 디스플레이 초기화
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 디스플레이 초기화
    Serial.println("SSD1306 allocation failed");
    for(;;);  // 초기화 실패 시 무한 루프
  }
  delay(1000);  // 디스플레이 안정화 시간
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // 초음파 센서 핀 모드 설정
  pinMode(trig_1, OUTPUT);
  pinMode(echo_1, INPUT);
  pinMode(trig_2, OUTPUT);
  pinMode(echo_2, INPUT);
  pinMode(trig_3, OUTPUT);
  pinMode(echo_3, INPUT);
  pinMode(trig_4, OUTPUT);
  pinMode(echo_4, INPUT);

  pinMode(SPEAKER_PIN, OUTPUT);  // 스피커 핀 설정

  // PS2 컨트롤러 초기화
  do {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, true, false);  // 압력 감지 비활성화
    if (error == 0) {
      Serial.println("PS2 Controller configured successfully!");
      break;
    } else {
      delay(100);  // 재시도 간격
    }
  } while (1);
}

// 초음파 센서로 거리 측정 함수
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  unsigned long duration = pulseIn(echoPin, HIGH, 60000);  // 타임아웃 60ms 설정
  float distance = (duration == 0) ? 9999 : ((float)(340 * duration) / 10000) / 2;  // 초음파 거리 계산
  return distance;
}

// 명령 실행 함수
void executeCommand(char cmd) {
  recordPath(cmd); // 경로기록 추가
  switch (cmd) {
    case 'u':  // 앞으로
      display.println("Forward");
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor1.setSpeed(60);
      motor2.setSpeed(80);
      break;
    case 'l':  // 왼쪽
      display.println("Left");
      motor1.run(BACKWARD);
      motor2.run(FORWARD);
      motor1.setSpeed(100);
      motor2.setSpeed(100);
      break;
    case 'r':  // 오른쪽
      display.println("Right");
      motor1.run(FORWARD);
      motor2.run(BACKWARD);
      motor1.setSpeed(100);
      motor2.setSpeed(100);
      break;
    case 'd':  // 뒤로
      display.println("Backward");
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor1.setSpeed(50);
      motor2.setSpeed(80);
      break;
    case 's':  // 정지
      display.println("Stop");
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      break;
    case 't':  // 브러시 회전
      display.println("Turn On");
      motor3.run(FORWARD);
      motor3.setSpeed(255);
      break;
    case 'f':  // 브러시 정지
      display.println("Turn Off");
      motor3.run(RELEASE);
      break;
    case 'a':  // 브러시 켜기
      display.println("Brush On");
      motor4.run(FORWARD);
      motor4.setSpeed(100);
      break;
    case 'b':  // 브러시 멈춤
      display.println("Brush Stop");
      motor4.run(RELEASE);
      break;
    default:  // 기본 정지
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      break;
  }
  display.display();  // 디스플레이 갱신
}

int automove = 0;  // 자율주행 모드 플래그
int speakmode = 0;

void loop() {
  // 초음파 센서 거리 측정 및 디스플레이 표시
  float distance1 = getDistance(trig_1, echo_1);
  float distance2 = getDistance(trig_2, echo_2);
  float distance3 = getDistance(trig_3, echo_3);
  float distance4 = getDistance(trig_4, echo_4);

  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("LEFT: ");
  display.print(distance1 == 9999 ? "No Obj" : String(distance1) + " cm");
  display.println();

  display.print("RIGHT: ");
  display.print(distance2 == 9999 ? "No Obj" : String(distance2) + " cm");
  display.println();

  display.print("FRONT: ");
  display.print(distance3 == 9999 ? "No Obj" : String(distance3) + " cm");
  display.println();

  display.print("BACK: ");
  display.print(distance4 == 9999 ? "No Obj" : String(distance4) + " cm");
  display.println();

  // PS2 컨트롤러 입력 처리
  ps2x.read_gamepad(false, vibrate);  // 컨트롤러 상태 읽기

  // SELECT 버튼을 눌러 자율주행 모드 전환
  // 자율주행 모드에서 이전 명령을 저장할 변수
char prevCmd = 's'; // 초기값은 정지 상태

if (ps2x.Button(PSB_SELECT)) {
  automove = !automove;  // 자율주행 모드 전환
  delay(500);  // 중복 입력 방지를 위한 딜레이
}

if(ps2x.Button(PSB_L1)) { // L1버튼을 눌러 복귀 모드 시작
  returnMode = true;
  delay(500);
}
// 복귀 모드가 활성화된 경우 경로를 역으로 따라감
if (returnMode && pathIndex > 0) {
  pathIndex--;
  char cmd = path[pathIndex];

  // 명령을 반대로 실행
  switch (cmd) {
    case 'u': executeCommand('d'); break;  // 전진 -> 후진
    case 'd': executeCommand('u'); break;  // 후진 -> 전진
    case 'l': executeCommand('r'); break;  // 좌회전 -> 우회전
    case 'r': executeCommand('l'); break;  // 우회전 -> 좌회전
    default: executeCommand('s'); break;   // 정지 명령은 그대로 유지
  }

  if (pathIndex == 0) {  // 복귀 완료 시 모드 종료
    returnMode = false;
    executeCommand('s');  // 멈춤
  }
}



if (ps2x.Button(PSB_START)) {
  if(speakmode == 0){
    speakmode = 1;
  }
  else{
    speakmode = 0;
  }
}

if(speakmode){
  display.print("Speaker Mode: ON");
  display.println();

  if((distance1 < 15) || (distance2 < 15) || (distance3 < 15) || (distance4 < 15)){
    tone(SPEAKER_PIN, 1000);
  }
  else{
    noTone(SPEAKER_PIN);
  }
} else{
  display.print("Speaker Mode: OFF");
  display.println();
  noTone(SPEAKER_PIN);
}


if (automove) {
  display.print("Auto Move: ON");
  display.println();

  bool obstacleInFront = (distance3 < 30);  // 전방 장애물 감지
  bool obstacleOnLeft = (distance1 < 30);   // 좌측 장애물 감지
  bool obstacleOnRight = (distance2 < 30);  // 우측 장애물 감지

  if (obstacleInFront || obstacleOnLeft || obstacleOnRight) {  // 어떤 방향이든 장애물이 있는 경우
    if (prevCmd != 's') {  // 이전 상태가 정지가 아니라면 정지
      executeCommand('s');  // 정지
      prevCmd = 's';
      //delay(200);  // 잠시 정지
    }

    // 장애물 회피 동작
    if (obstacleInFront) {
      if (distance1 > distance2 && !obstacleOnLeft) {  // 왼쪽으로 회피 가능
        if (prevCmd != 'l') {  // 이전 명령이 'l'이 아닐 때만 실행
          executeCommand('l');
          prevCmd = 'l';
          //delay(200);  // 왼쪽 회피 후 대기
        }
      } else if (distance2 > distance1 && !obstacleOnRight) {  // 오른쪽으로 회피 가능
        if (prevCmd != 'r') {  // 이전 명령이 'r'이 아닐 때만 실행
          executeCommand('r');
          prevCmd = 'r';
          //delay(200);  // 오른쪽 회피 후 대기
        }
      } else {  // 좌우 모두 회피가 어려운 경우 후진
        if (prevCmd != 'd') {  // 이전 명령이 'd'가 아닐 때만 실행
          executeCommand('d');
          prevCmd = 'd';
          //delay(200);  // 후진 후 대기
        }
      }
    } else if (obstacleOnLeft && !obstacleOnRight) {  // 좌측 장애물 감지, 우측 회피 가능
      if (prevCmd != 'r') {
        executeCommand('r');
        prevCmd = 'r';
        //delay(200);  // 오른쪽 회피 후 대기
      }
    } else if (obstacleOnRight && !obstacleOnLeft) {  // 우측 장애물 감지, 좌측 회피 가능
      if (prevCmd != 'l') {
        executeCommand('l');
        prevCmd = 'l';
        //delay(200);  // 왼쪽 회피 후 대기
      }
    }
  } else {  // 전방이 비어 있고 좌우 장애물이 없으면 계속 전진
    if (prevCmd != 'u') {  // 이전 명령이 'u'가 아닐 때만 실행
      executeCommand('u');
      prevCmd = 'u';
    }
  }
} else {
    display.print("Auto Move: OFF");
    display.println();
    // 수동 모드: PS2 컨트롤러 입력에 따라 로봇 제어
    if (ps2x.Button(PSB_PAD_UP)) {
      executeCommand('u');  // 앞으로 이동
    } else if (ps2x.Button(PSB_PAD_DOWN)) {
      executeCommand('d');  // 뒤로 이동
    } else if (ps2x.Button(PSB_PAD_LEFT)) {
      executeCommand('l');  // 왼쪽 이동
    } else if (ps2x.Button(PSB_PAD_RIGHT)) {
      executeCommand('r');  // 오른쪽 이동
    } else if (ps2x.Button(PSB_CIRCLE)) {
      executeCommand('t');  // 브러시 회전 시작
    } else if (ps2x.Button(PSB_CROSS)) {
      executeCommand('f');  // 브러시 정지
    } else if (ps2x.Button(PSB_TRIANGLE)) {
      executeCommand('a');  // 브러시 켜기
    } else if (ps2x.Button(PSB_SQUARE)) {
      executeCommand('b');  // 브러시 멈춤
    } else {
      executeCommand('s');  // 정지
    }
  }

  display.display();  // 디스플레이 갱신
}