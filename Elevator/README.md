# 🚀 STM32 Elevator Project

## 📌 Overview
이 프로젝트는 STM32 마이크로컨트롤러 기반으로 구현한 **엘리베이터 제어 시스템**입니다.

처음에는 **Bare-metal 방식(STM32H533RE)**으로 구현되었으며,  
이후 구조 개선과 확장성을 위해 **RTOS(FreeRTOS) + TouchGFX 기반(STM32F429)**으로 리팩토링했습니다.

---

## 🖼️ Demo / Images
<!-- 나중에 이미지 추가 -->
![demo](./images/demo.gif)

<!--
추가 예시:
![ui](./images/ui.png)
![hardware](./images/hardware.jpg)
-->

---

## 🔄 Development History

### 1️⃣ Bare-metal Version (STM32H533RE)
- HAL 기반 polling 구조
- 상태 머신(State Machine) 기반 엘리베이터 제어
- 키패드 입력 처리
- 서보 모터(PWM) 제어
- 층 센서 기반 위치 추적

👉 특징
- 단일 루프 (`while(1)`)
- Collective Selective 알고리즘 구현
- 요청 배열 기반 처리

---

### 2️⃣ RTOS Refactoring (STM32F429)
- FreeRTOS 기반 구조
- Task / Queue / Mutex 사용
- 이벤트 기반 처리 방식
- 상태 공유 구조체 도입

👉 개선점
- Blocking → Non-blocking 구조
- 확장성 증가
- 모듈화

---

### 3️⃣ TouchGFX Integration (In Progress)
- STM32F429 + TouchGFX 기반 UI 추가 예정
- 현재 UI는 아직 프로젝트에 완전히 통합되지 않음

👉 계획 기능
- 층 표시 UI
- 버튼 입력 UI
- 엘리베이터 상태 시각화 (이동, 문 상태 등)

---

## ⚙️ Key Features

### 🎯 Elevator Control Algorithm
- Collective Selective 방식
- 이동 방향 유지
- 요청 우선순위 처리

---

### 🚪 Door State Machine (RTOS)
- DOOR_OPENING
- DOOR_OPEN
- DOOR_CLOSING
- DOOR_CLOSED

👉 타이머 기반 상태 전이

---

### 🔢 Input System (Keypad)
- 4x4 Matrix Keypad
- GPIO 스캔 방식
- 디바운싱 포함

---

### ⚡ Motor Control
- PWM 기반 서보 모터 제어
- 방향:
  - UP
  - DOWN
  - STOP

---

## 🧩 Key Differences

| Feature | Bare-metal | RTOS |
|--------|-----------|------|
| 구조 | 단일 루프 | Task 기반 |
| 입력 처리 | Polling | Queue |
| 상태 관리 | 전역 변수 | Mutex |
| 확장성 | 낮음 | 높음 |
| UI 연동 | 어려움 | 쉬움 |

---

## 🛠️ Hardware

- MCU:
  - STM32H533RE (Bare-metal)
  - STM32F429 (RTOS + GUI)
- Input:
  - 4x4 Keypad
- Output:
  - Servo Motor
- Sensor:
  - 층 감지 GPIO

---

## 📂 Project Structure

```bash
/Core
 ├── main.c
 ├── keypad.c
 ├── keypad.h
 ├── elevator.c   # RTOS 기반 리팩토링 코드
```

---

## 📈 Future Work

- [ ] TouchGFX UI 완전 통합
- [ ] Emergency 기능 추가
- [ ] 스케줄링 알고리즘 개선
- [ ] 통신 기능 (UART / CAN)
- [ ] 실제 하드웨어 확장

---

## 💡 Notes

- Bare-metal 구현 → 제어 흐름과 알고리즘 이해 목적
- RTOS 구현 → 실제 시스템 구조 설계 목적
- 현재 프로젝트는 RTOS 기반으로 확장 진행 중

---

## 👨‍💻 Author
- Embedded Developer
