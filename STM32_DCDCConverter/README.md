# ⚡ 30 kV급 파워서플라이 제작

## 📖 개요
- **기간**: 2025.03 ~ 2025.06
- **역할**: STM32 펌웨어 개발 & Altium PCB 설계
- **기술 스택**: STM32F103, C (HAL), UART, PWM, ADC, Altium Designer, EMI 분석

이 프로젝트는 **입력 전압 30 V를 30 kV로 승압하는 고전압 파워서플라이**를 설계하고 제작한 결과물입니다.  
**STM32 → 푸시풀(Push‑Pull) 컨버터 → Cockcroft–Walton(CW) 볼티지 멀티플라이어** 로 구성된 시스템이며,  
기존 아날로그 PWM IC의 회로를 참고하여 **STM32 기반으로 소프트스타트와 피드백 기능을 구현**했습니다.

---

## ⚙️ 시스템 구성
- **입력**: 30 V DC
- **푸시풀 컨버터**: 변압기로 약 60배 승압
- **CW 회로(전압 멀티플라이어)**: 약 20배 승압
- **출력**: 약 30 kV DC

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

![PCB 3D Simulation](docs/pcb_3d.png)
*(Altium Designer 3D 뷰)*

![PWM waveform](docs/pwm_waveform.png)
*(STM32 TIM1/TIM2 Dead-Time PWM 파형)*

![Gate Driver PWM](docs/gate_driver_waveform.png)
*(게이트 드라이버를 통과한 CH1/CH1N PWM 파형 – 데드타임이 적용된 모습)*

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

