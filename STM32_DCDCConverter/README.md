# ⚡ 30 kV급 파워서플라이 제작

## 📖 개요
- **기간**: 2025.03 ~ 2025.06
- **역할**: STM32 펌웨어 개발 & Altium PCB 설계
- **기술 스택**: STM32F103, C (HAL), UART, PWM, ADC, Altium Designer, EMI 분석

이 프로젝트는 30 V의 입력 전압을 약 30 kV로 승압하기 위한 고전압 파워서플라이를 설계하고 제작한 결과물입니다.
시스템은 STM32를 이용한 디지털 제어부, 푸시풀(Push‑Pull) 컨버터를 통한 1차 승압부,
그리고 Cockcroft–Walton(CW) 전압 멀티플라이어를 통한 2차 승압부로 구성됩니다.
기존 아날로그 PWM IC를 대체하여 STM32에서 PWM 생성, 소프트스타트, 피드백 제어를 직접 구현하여
보다 유연하고 확장 가능한 제어가 가능하도록 설계했으며,
안전한 실험 환경을 위해 HC‑05 블루투스를 통한 무선 제어 및 출력 전압 모니터링 기능을 구현하고,
고전압 부품과 회로는 에폭시 몰딩을 적용하여 절연과 방전 방지를 강화했습니다.

---

## ⚙️ 시스템 구성
- **입력**: 30 V DC
- **푸시풀 컨버터**: 변압기로 약 60배 승압
- **CW 회로(전압 멀티플라이어)**: 약 20배 승압
- **출력**: 약 30 kV DC

### 🔹 Push‑Pull Converter
- **역할**:  
  입력 DC 전압(30 V)을 트랜스포머를 통해 고주파 스위칭하여 약 60배 승압.
- **특징**:  
  - 두 개의 MOSFET을 교대로 구동해 트랜스포머의 코어 활용률을 높임  
  - 스위칭 손실과 전압/전류 피크를 줄여 높은 전력 효율 달성  
  - 작은 크기에서도 높은 전압 확보 가능
  - 
### 🔹 Cockcroft–Walton (CW) Voltage Multiplier
- **역할**:  
  Push‑Pull 컨버터의 출력 AC 전압을 단계적으로 정류·승압하여 약 20배 추가 승압.
- **특징**:  
  - n단(Stage)일수록 전압이 n배로 상승  
  - 소형 부품으로 고전압 설계 가능  
  - 전류보다 전압이 중요한 응용(EMP용 고전압 공급)에 최적화  
  - 소자 간 전계 분산 및 절연 거리 확보가 유리
 
  ✅ **최종 출력**:  
30 V → (Push‑Pull ×60) → (CW Multiplier ×20) → **약 30 kV DC**
  - 이후 피드백 제어를 통해 30kV 유지

---

## 💡 주요 구현 내용
### 🔸 STM32 펌웨어
- **PWM 생성**
  - TIM1/TIM2를 사용하여 **데드타임(dead-time)** 을 갖는 상보 PWM 2개 출력
  - **게이트 드라이버 입력 요구사항**(전류 싱크 능력, 전파 지연)을 고려하여 타이밍 조정
  - MOSFET shoot-through 방지 및 변압기 포화 방지를 위한 데드타임 확보
- **Soft Start**
  - 구동 시작 시, 소자 보호를 위해 약 10초간 듀티를 점진적으로 증가
- **ADC 피드백**
  - 출력 전압을 **1:10000 분압 회로**로 스케일 다운
  - STM32 ADC로 전압 측정 후 단순 비례 피드백 적용하여 목표 전압 유지
- **UART & Bluetooth**
  - HC-05 모듈을 통해 스마트폰에서 **원격으로 PWM 시작/종료**
  - 스마트폰에서 실시간으로 **출력 전압 모니터링**
 
  - ### 🔸 게이트 드라이버 설계 및 고려사항
- **게이트 드라이버 채택 이유**
  - STM32에서 직접 MOSFET을 구동하기엔 전류 구동능력과 전파지연 보장이 어려움
  - 고속 스위칭 시 MOSFET 게이트의 전하(Qg)를 빠르게 충·방전하기 위해 전용 게이트 드라이버 사용
- **데드타임 확보**
  - 두 개의 PWM 채널(CH1/CH1N) 사이에 데드타임을 넣어 **Shoot‑Through 방지**
  - 게이트 드라이버의 싱크 전류 능력과 내부 전파 지연을 고려해 t_d를 산출
- **게이트 드라이버 후 파형**
  - STM32 PWM 출력 → 게이트 드라이버 → MOSFET 게이트
  - 드라이버를 통과하면서 소자 보호를 위한 적절한 상승·하강 시간을 확보
- **적용 예시**
  - IR2110 계열 드라이버를 통해 High‑Side / Low‑Side 스위칭 동기화
  - Shoot‑Through 방지 및 트랜스포머 포화 방지를 위한 정밀 제어


### 🔸 PCB 설계 (Altium Designer)
- **Push‑Pull 컨버터 + CW 회로 PCB 설계**
  - EMI 및 크리피지 거리 확보를 고려한 부품 배치
  - Ball soldering 기법으로 Sharp Edge 효과 최소화
  - Epoxy 몰딩으로 고전압 절연 강화
- **피드백 및 신호 안정성**
  - 출력 전압 피드백용 저항망 구성 (약 1:10000)
  - UART 핀 아웃 및 블루투스 모듈 핀 매핑
- **2D/3D 시뮬레이션 및 EMI 검토**
  - Ansys SIwave로 PCB 레이아웃의 EMI 분석
  - GND/VCC 루프 최소화, 디커플링 커패시터 최적 배치

---

## 🛠 실행 / 사용 방법
1. STM32CubeIDE에서 `main.c` 프로젝트 빌드 및 다운로드
2. 30 V 입력 전원을 PCB에 공급
3. 스마트폰 → HC-05 블루투스 연결
4. UART 명령으로 PWM **Start/Stop** 제어 및 전압 모니터링

---

## 📷 결과물
> (오실로스코프 PWM 파형, PCB 3D 뷰, 제작된 기판 사진 등을 첨부하면 좋습니다)

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
![Uploading image.png…]()

*(HC‑05 블루투스를 통한 UART 통신 결과 – 스마트폰 앱에서 PWM 시작/정지 및 전압 모니터링)*
---

## ✨ 담당 및 성과
✅ **STM32 펌웨어 개발**
- PWM 신호 생성 및 softstart, 피드백 로직 구현
- UART 블루투스 제어 기능 개발

✅ **Altium PCB 설계**
- 고전압용 PCB 레이아웃 설계 및 EMI 개선
- 2D/3D 시뮬레이션 및 실측 검증

✅ **성과**
- 기존 상용 PWM IC 대비 유연한 제어 가능
- 약 **30 kV 출력** 안정적으로 달성
- 스마트폰 기반 원격 제어 및 실시간 모니터링 구현

---

## 📂 코드 및 자료
- `STM32_Firmware/` : `main.c`, `pwm_control.c` 등 소스코드
- `PCB_Design/` : Altium 프로젝트(`.PrjPcb`, `.SchDoc`, `.PcbDoc`)
- `docs/` : 2D/3D 시뮬레이션 이미지, 파형 캡처, 제작 기판 사진

