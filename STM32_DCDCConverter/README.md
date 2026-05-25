# ⚡ MCU 기반 DC-DC 컨버터 제작

## 📖 개요
- **기간**: 2025.03 ~ 2025.06
- **역할**: STM32 펌웨어 개발
- **기술 스택**: STM32F103, C (HAL), UART, PWM, ADC

이 프로젝트는 MCU를 활용하여 DC-DC 컨버터를 설계하고 제작한 결과물입니다.
시스템은 STM32 기반 디지털 제어부와 push-pull 컨버터 기반 승압부로 구성됩니다.
아날로그 PWM IC를 대신하여 STM32에서 PWM 생성, soft-start, 피드백 제어를 직접 구현하여
보다 유연하고 확장 가능한 제어가 가능하도록 설계했으며,
원격 제어 및 출력 전압 모니터링을 위해 HC-05 블루투스 기반 UART 인터페이스를 함께 구현했습니다.

---

## ⚙️ 시스템 구성

### 🔹 Push-Pull Converter
- **역할**:  
  입력 DC 전압을 트랜스포머를 통해 고주파 스위칭하여 승압.
- **특징**:  
  - 두 개의 MOSFET을 교대로 구동해 트랜스포머의 코어 활용률을 높임  
  - 스위칭 손실과 전압/전류 피크를 줄여 높은 전력 효율 달성  
  - 작은 크기에서도 효율적인 전력 변환 가능
  - 출력 전압은 피드백 제어를 통해 목표값 유지

---

## 💡 주요 구현 내용
### 🔸 STM32 펌웨어
- **PWM 생성**
  - TIM1을 사용하여 **데드타임(dead-time)** 을 갖는 상보 PWM 출력
  - **게이트 드라이버 입력 요구사항**(전류 싱크 능력, 전파 지연)을 고려하여 타이밍 조정
  - MOSFET shoot-through 방지 및 변압기 포화 방지를 위한 데드타임 확보
- **Soft Start**
  - 구동 시작 시, 소자 보호를 위해 듀티를 점진적으로 증가시키는 시퀀스 적용
- **ADC 피드백**
  - 출력 전압을 분압 회로로 스케일 다운
  - STM32 ADC로 전압 측정 후 비례 피드백을 적용하여 목표 전압 유지
  - ADC 소스 임피던스를 고려해 샘플링 시간을 71.5 cycles로 조정해 측정 안정성 확보
- **UART & Bluetooth**
  - HC-05 모듈을 통해 스마트폰에서 **원격으로 PWM 시작/종료**
  - 스마트폰에서 실시간으로 **출력 전압 모니터링**

### 🔸 게이트 드라이버 설계 및 고려사항
- **게이트 드라이버 채택 이유**
  - STM32에서 직접 MOSFET을 구동하기엔 전류 구동능력과 전파지연 보장이 어려움
  - 고속 스위칭 시 MOSFET 게이트의 전하(Qg)를 빠르게 충·방전하기 위해 전용 게이트 드라이버 사용
- **데드타임 확보**
  - 두 개의 PWM 채널(CH1/CH1N) 사이에 데드타임을 넣어 **Shoot-Through 방지**
  - 게이트 드라이버의 싱크 전류 능력과 내부 전파 지연을 고려해 t_d를 산출
- **게이트 드라이버 후 파형**
  - STM32 PWM 출력 → 게이트 드라이버 → MOSFET 게이트
  - 드라이버를 통과하면서 소자 보호를 위한 적절한 상승·하강 시간을 확보

### 🔸 PCB 설계 (Altium Designer)
- **STM32 제어부 PCB 설계**
  - STM32 및 주변 회로(UART/블루투스 모듈 인터페이스, ADC 피드백 입력단, PWM 출력단)의 PCB 레이아웃 설계
  - UART 핀 아웃 및 HC-05 블루투스 모듈 핀 매핑
  - 피드백 입력용 분압저항 배치
    
---


## 📷 결과물
![PCB 3D Simulation]<img width="2110" height="404" alt="image" src="https://github.com/user-attachments/assets/f707e8d5-0584-4379-aecd-ba87ce1d4d34" />
<img width="2091" height="389" alt="image" src="https://github.com/user-attachments/assets/2dc717f4-309b-4ad0-b0ca-8c4991d0136e" />
*(Altium Designer 3D 뷰)*
<img width="2294" height="422" alt="image" src="https://github.com/user-attachments/assets/6f4ad600-6058-4837-ba74-636748368ea1" />

![PWM waveform]<img width="571" height="425" alt="image" src="https://github.com/user-attachments/assets/4064d706-c50d-44b4-876c-f9ef46c8ec4d" />
*(STM32 TIM1/TIM2 Dead-Time PWM 파형)*

![Gate Driver PWM]<img width="533" height="397" alt="image" src="https://github.com/user-attachments/assets/75407f39-e1ad-4f07-b18e-8349b37b0593" />

*(게이트 드라이버를 통과한 CH1/CH1N PWM 파형 – 데드타임이 적용된 모습)*

![LTspice Simulation]<img width="2639" height="827" alt="image" src="https://github.com/user-attachments/assets/46f46eef-5743-4878-82eb-a77567628b7f" />

*(LTspice에서 푸시풀 컨버터 및 CW 회로 시뮬레이션으로 출력 전압 특성 검증)*

![PWM control interface]<img width="716" height="339" alt="image" src="https://github.com/user-attachments/assets/f0dd3fd6-3428-4dae-8c33-b2b2351aefec" />
<img width="538" height="130" alt="image" src="https://github.com/user-attachments/assets/33437666-6dbc-4709-aeed-531fe7322219" />

*(HC‑05 블루투스를 통한 UART 통신 결과 – 스마트폰 앱에서 PWM 시작/정지 및 전압 모니터링)*

몰딩전 피드백 핀의 전압 측정
<img width="453" height="111" alt="image" src="https://github.com/user-attachments/assets/28ae6e7f-a492-402c-baf7-9a77490c86ee" />
(750Meg+0.1Meg+0.05Meg)/0.05Meg -> 15003배
15003*1.07 -> 약 16kV

몰딩 후 전압
15003*1.96 -> 약 29kV
![Uploading image.png…]()

---


## ✨ 담당 및 성과
✅ **STM32 펌웨어 개발**
- PWM 신호 생성, soft-start, 피드백 로직 구현
- UART 블루투스 제어 기능 개발

✅ **Altium PCB 설계**
- STM32 제어부 PCB 레이아웃 설계

✅ **성과**
- 기존 상용 PWM IC 대비 유연한 제어 가능
- soft-start 및 ADC 피드백 기반 안정적인 출력 전압 유지
- 스마트폰 기반 원격 제어 및 실시간 모니터링 구현

---

