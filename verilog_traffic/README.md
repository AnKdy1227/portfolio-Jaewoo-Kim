# 🚦 Verilog Traffic Light Controller with LCD


## 📖 개요
- **기간**: (예: 2023.08 ~ 2023.12)
- **기술 스택**: Verilog HDL, FPGA (Vivado 사용), FSM 설계, LCD 제어
- ✅ - **사용 보드**: COMBO 2 DLD (Xilinx Spartan‑7 XC7S75F FGG484-1 탑재)


이 프로젝트는 **FPGA 상에서 동작하는 신호등 제어기**를 Verilog로 구현한 것입니다.  
시간 카운터, 상태머신(FSM), LCD 디스플레이 모듈을 결합하여 **실제 신호등 동작과 유사하게 LED를 제어**하고, 동시에 LCD에 현재 상태와 시간을 표시합니다.


## 💡 주요 구현 내용
- **Top Module `traffic`**
  - `clock` 모듈: DIP 스위치와 버튼 입력으로 시간을 카운트하고 시·분·초를 출력
  - `statework` 모듈: 주간/야간 모드에 따른 신호등 FSM 제어 (빨강, 노랑, 초록, 좌회전, 보행자 신호)
  - `lcdstate` 모듈: 신호 상태를 LCD 표시용 코드로 변환
  - `LCD` 모듈: 위 정보를 받아 LCD에 시간과 상태를 출력
- `rst` 입력을 active-low로 변환(`rstn`)하여 내부 모듈에 전달
- **입력**: 
  - `clk`, `dip`, `bt`, `bt_manual`
- **출력**: 
  - `led_red`, `led_yellow`, `led_green`, `led_left`, `led_walk_red`, `led_walk_green`
  - `LCD_E`, `LCD_RS`, `LCD_RW`, `LCD_DATA`, `LED_out`

---

## 🛠 실행 / 사용 방법
1. Vivado에서 프로젝트를 생성 후, `traffic.v` 및 관련 모듈 파일(`clock.v`, `statework.v`, `lcdstate.v`, `LCD.v`)을 추가합니다.
2. 시뮬레이션(Testbench)으로 FSM 동작을 검증합니다.
3. FPGA 보드(COMBO 2 DLD)에 업로드 후 DIP 스위치 및 버튼으로 제어합니다.

---


## 📷 결과물
RTL analysis schematic
<img width="1956" height="781" alt="image" src="https://github.com/user-attachments/assets/c2a719fb-462b-4278-ab31-060529197834" />

Slice, DSP, MUX 개수
<img width="1330" height="793" alt="image" src="https://github.com/user-attachments/assets/5dfd4fd3-f5bb-44a8-a090-f856a562adac" />

신호등 결과
<img width="1248" height="480" alt="image" src="https://github.com/user-attachments/assets/e6d4da5b-0132-48c3-811f-c038a7e1167c" />
<img width="1373" height="490" alt="image" src="https://github.com/user-attachments/assets/90308071-ceec-4f7c-a480-07449a0bfd8e" />

---

## ✨ 느낀 점
- 여러 모듈을 top module에서 체계적으로 연결하는 경험을 통해 **설계 분할과 인터페이스 설계의 중요성**을 체감했습니다.
- LCD와 신호등 LED를 동시에 제어하면서 **FSM 설계와 디버깅 능력**이 크게 향상되었습니다.

