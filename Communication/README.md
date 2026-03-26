# Communication Implementation

다양한 통신 프로토콜을 직접 구현한 프로젝트입니다.

## 📌 Features
- I2C (Blocking 방식)
  - Master / Slave 구현
- CAN Communication
- SPI (Slave)
- UART Communication
- WiFi Communication

## 🧠 Description
임베디드 시스템에서 자주 사용되는 주요 통신 프로토콜들을  
직접 구현하고 동작을 확인한 프로젝트입니다.

각 통신 방식의 기본 구조와 데이터 송수신 과정을 이해하는 것을 목표로 합니다.

## 🛠️ Tech
- Language: C

## 📁 Structure
- i2c/      : I2C Master / Slave (Blocking)
- can/      : CAN 통신 구현
- spi/      : SPI Slave 구현
- uart/     : UART 통신 구현
- wifi/     : WiFi 통신 구현

## ▶️ Usage
각 폴더에서 개별적으로 빌드 및 실행

예시:
gcc main.c -o run
./run
