# 📱 퍼즐

## 📖 개요
- **기간**: 2024.09 ~ 2024.12
- **기술 스택**: Android(Java), JNI, NDK(C), OpenCL, Linux Device Driver
- **역할**: 앱 기능 구현, 이미지 처리 로직 설계, JNI 연동, 7‑Segment 드라이버 연동

이 프로젝트는 **사용자가 직접 촬영한 사진을 퍼즐 게임으로 변환**하여 플레이할 수 있는 안드로이드 앱을 개발한 결과물입니다.  
카메라로 촬영한 이미지를 조각내어 **랜덤한 이미지 처리(Grayscale, 색반전, Sobel, Gaussian Blur, 추가 가능)**를 적용한 뒤,  
사용자가 퍼즐을 회전·배치하여 맞추는 게임을 구현했습니다.  
추가로 JNI를 통해 **7‑Segment 하드웨어 드라이버**와 연동하여 게임 시간 표시 기능을 구현했습니다.

---

## 💡 주요 구현 내용
### 🔹 MainActivity.java
- **카메라 촬영**  
  - `capture` 버튼 리스너 → `pictureCallback()` 호출 → 현재 카메라 프레임을 bitmap으로 저장
- **이미지 처리**  
  - 촬영한 이미지를 4등분 후, 각 조각에 랜덤으로 이미지 처리 적용
  - **Grayscale, 색반전, Sobel Filter**는 CPU 처리
  - **Gaussian Blur**는 OpenCL을 이용해 GPU 병렬 처리
- **데이터 전달**  
  - 조각 이미지 배열과 원본 이미지, 전체 편집 이미지를 `intent.putExtra()`로 PuzzleActivity에 전달

### 🔹 PuzzleActivity.java
- **퍼즐 조립 로직**
  - 하단에 랜덤 회전·순서로 나열된 4조각 → 슬롯 클릭으로 배치
  - 슬롯 리스너: 모든 슬롯이 채워지면 정답 여부 판정
  - 회전 버튼: 선택된 조각을 90° 회전
  - 리셋 버튼: 퍼즐 초기화 (타이머는 유지)
- **시간 측정**
  - `Runnable`로 별도 스레드에서 퍼즐 시작부터 소요 시간 측정
  - JNI로 7‑Segment 드라이버를 호출하여 시간 표시
- **퍼즐 맞춤 판정**
  - 모든 조각이 올바른 위치·회전으로 들어오면 정답 → ResultActivity로 이동

### 🔹 ResultActivity.java
- **게임 결과 표시**
  - 원본 사진과 맞춘 퍼즐 이미지를 화면에 표시
  - 퍼즐 클리어까지 걸린 시간 표시
  - `Try Again` 버튼으로 MainActivity 복귀

### 🔹 JNI & 드라이버
- JNI를 통해 `OpenCLDriver.c` 호출하여 GPU 이미지 처리
- 7‑Segment 하드웨어 드라이버 접근:
  - JNI 레이어에서 `/dev` 디바이스 파일 접근
  - 퍼즐 소요 시간을 실시간 표시

---

## ⚙️ 시스템 흐름
- Camera Capture  
- ↓ (MainActivity)  
- Image Processing (OpenCL, CPU)  
- ↓ (Intent Extra)  
- PuzzleActivity: 퍼즐 조립 + Timer  
- ↓ (JNI)  
- 7-Segment Hardware Timer Display  
- ↓  
- ResultActivity: 결과 이미지 + 시간 표시

## 🛠 실행 방법
### ▶️ 앱 빌드
- Android Studio에서 프로젝트 열기
- `minSdkVersion=19 (KitKat)` 기반으로 빌드
- APK 설치 후 실행

### ▶️ 퍼즐 진행
1. 앱 실행 → `Capture` 버튼으로 사진 촬영
2. 이미지 처리 후 4조각으로 퍼즐 화면 표시
3. 조각을 회전·배치하여 퍼즐을 완성
4. 7‑Segment에서 실시간으로 소요 시간 확인
5. 정답 시 결과 화면 출력

---

## 📷 결과물
![App Screenshot](docs/app_screenshot.png)  
*(퍼즐 조각이 랜덤하게 배치된 PuzzleActivity 화면)*

![Result Screen](docs/result_screen.png)  
*(퍼즐을 맞춘 후 원본 이미지와 소요 시간 표시)*

![7-Segment Display](docs/7seg_display.png)  
*(JNI 드라이버를 통해 7-Segment에 게임 시간 표시)*

---

## ✨ 어려웠던 점과 해결
- **GPU(OpenCL) 이미지 처리**  
  - Gaussian Blur는 GPU로 성공, 다른 필터는 GPU 처리 시 앱 크래시 발생 → CPU로 처리
- **대용량 Bitmap 전달 문제**  
  - Intent로 Bitmap 전달 시 OutOfMemory 발생 → 이미지 quality 50으로 압축 후 byte 배열로 전달
- **버튼 하드웨어 리스너 문제**  
  - Button interrupt 시 7‑Segment와 충돌 발생 → 버튼 기능 제외

---

## 📂 코드 및 자료
- `app/src/main/java/com/example/puzzle/`
  - `MainActivity.java`, `PuzzleActivity.java`, `ResultActivity.java`
- `app/src/main/jni/`
  - `OpenCLDriver.c`
- `docs/`
  - 앱 실행 화면 캡처, 퍼즐 플로우차트
  *(사진찍음)*
  <img width="551" height="364" alt="image" src="https://github.com/user-attachments/assets/129cce46-4cd1-4561-8c0f-a0c20edeb56f" />
  
  *(해당 사진에 대해 랜덤한 이미지 처리가 적용된 조각들 생성)*
<img width="670" height="382" alt="image" src="https://github.com/user-attachments/assets/b53c915d-b539-4f83-86fb-ced22e83ec9f" />

  *(선택된 퍼즐 조각 회전, 슬롯에 넣기)*
<img width="756" height="455" alt="image" src="https://github.com/user-attachments/assets/ea0633c9-e86a-45f3-94de-b41d16d2e789" />

 *(퍼즐을 잘못 맞춘 경우 리셋)*
<img width="409" height="572" alt="image" src="https://github.com/user-attachments/assets/93af10b9-9061-4aa4-805f-b4f85c44e87d" />

  *(알맞게 맞춘 경우 결과 처리)*
<img width="292" height="415" alt="image" src="https://github.com/user-attachments/assets/60184638-9139-4b7d-85ba-de3d587fa272" />


---

## ✨ 느낀 점
- **안드로이드 앱 개발 전 과정**(UI, Camera API, 이미지 처리, JNI)과 **임베디드 하드웨어 연동**을 처음부터 끝까지 경험할 수 있었습니다.
- 단순해 보이는 퍼즐 앱도 다양한 예외 처리와 성능 최적화가 필요하다는 것을 깨달았습니다.
- OpenCL, JNI, NDK 사용 경험을 통해 Android Native 개발의 가능성과 한계를 이해했습니다.

