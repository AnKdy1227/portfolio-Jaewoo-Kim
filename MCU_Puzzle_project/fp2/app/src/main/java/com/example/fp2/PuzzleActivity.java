package com.example.fp2;



import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;

import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;


import android.graphics.drawable.BitmapDrawable;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import android.os.Handler;

public class PuzzleActivity extends AppCompatActivity implements Runnable, JNIListener{

    private long startTime;
    private long endTime;
    private long duration;
    private TextView timeElapsedText; // 실시간 시간 표시
    private Handler handler = new Handler(); // Handler for updating UI

    private JNIDriver mDriver; // button interrupt 객체

    //private JNIDriver mInterruptDriver; // 인터럽트 드라이버 인스턴스
    private JNIDriver mSegmentDriver; // 세그먼트 드라이버 인스턴스
    private JNIDriver mInterruptDriver;

    TextView tv;
    private ArrayList<ImageView> navigableElements; // 탐색 가능한 요소
    private int currentIndex = 0; // 현재 가리키는 요소의 인덱스


    private boolean isTimerRunning = false;
    private long pauseTime = 0;




    private ImageView slot1, slot2, slot3, slot4;
    private ImageView piece1, piece2, piece3, piece4;
    private ArrayList<ImageView> pieceViews;
    private ImageView selectedPiece = null; // 현재 선택된 조각
    private int currentRotation = 0; // 회전 각도
    private String originalImagePath; // 원본 이미지 경로
    private String combinedImagePath;// 편집된 이미지 경로
    private byte[] rotatedBitmapBytes;
    private byte[] combinedImageBytes;
    private ArrayList<byte[]> imageBytesList;

    // 조각 태그를 저장하는 클래스
    private static class PieceTag {
        int index;             // 조각의 올바른 위치
        int correctRotation;   // 조각의 올바른 회전 각도

        PieceTag(int index, int correctRotation) {
            this.index = index;
            this.correctRotation = correctRotation;
        }
    }

    static {
        System.loadLibrary("JNIDriver");
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sec); // 레이아웃 설정

        initializeViews(); // 레이아웃의 각 구성 요소를 초기화하고 해당 객체들을 설정
        startTime = System.currentTimeMillis();// 현재 시간을 밀리초 단위로 가져와 게임의 시작 시간을 설정
        startTimer();// 타이머를 시작, 이는 게임 진행 시간을 계산하는 데 사용

        //이전 액티비티로부터 사진을 받음
        rotatedBitmapBytes = getIntent().getByteArrayExtra("rotated_bitmap_bytes");
        combinedImageBytes = getIntent().getByteArrayExtra("edit");

        loadPuzzlePieces();// 퍼즐 조각을 로드하고 화면에 표시, 섞음

        setupPieceListeners();// 각 퍼즐 조각에 이벤트 리스너를 설정

        setupSlotListeners();// 퍼즐의 슬롯에 이벤트 리스너를 설정

        setupRotationListener();// 회전 버튼에 이벤트 리스너를 추가
        setupResetButton(); // 리셋 버튼에 이벤트 리스너를 설정

        mSegmentDriver = new JNIDriver();
        mInterruptDriver = new JNIDriver();

       if (mSegmentDriver.open("/dev/sm9s5422_segment") < 0) {
            Toast.makeText(this, "Segment Driver Open Failed", Toast.LENGTH_SHORT).show();
        }

        if (mSegmentDriver.open("/dev/sm9s5422_interrupt") < 0) {
            Toast.makeText(this, "Segment Driver Open Failed", Toast.LENGTH_SHORT).show();
        }


    }

    private void initializeViews() {
        slot1 = findViewById(R.id.puzzle_slot_1);
        slot2 = findViewById(R.id.puzzle_slot_2);
        slot3 = findViewById(R.id.puzzle_slot_3);
        slot4 = findViewById(R.id.puzzle_slot_4);

        piece1 = findViewById(R.id.image_piece_1);
        piece2 = findViewById(R.id.image_piece_2);
        piece3 = findViewById(R.id.image_piece_3);
        piece4 = findViewById(R.id.image_piece_4);

        timeElapsedText = findViewById(R.id.time_elapsed_text);


        pieceViews = new ArrayList<>(Arrays.asList(piece1, piece2, piece3, piece4));
        // 탐색 가능한 요소 리스트 초기화
        navigableElements = new ArrayList<>(Arrays.asList(
                piece1, piece2, piece3, piece4,
                slot1, slot2, slot3, slot4
        ));

        // 초기 강조 표시
        highlightElement(currentIndex);
    }



    private void highlightElement(int index) {
        // 모든 요소의 강조 표시 제거
        for (ImageView element : navigableElements) {
            element.clearColorFilter();
        }

        // 현재 가리키는 요소에 강조 표시
        ImageView currentElement = navigableElements.get(index);
        currentElement.setColorFilter(Color.argb(20, 255, 255, 0)); // 노란색 강조
    }

    @Override
    public void run() {
        long currentTime = System.currentTimeMillis();
        duration = currentTime - startTime; // 현재 경과 시간 계산
        updateTimerText(duration); // 화면 업데이트

        handler.postDelayed(this, 10); // 1초마다 실행
    }

    private void updateTimerText(long elapsedMillis) {
        int seconds = (int) (elapsedMillis / 1000) % 60;
        int minutes = (int) (elapsedMillis / 1000) / 60;
        String time = String.format("Time: %02d:%02d", minutes, seconds);
        timeElapsedText.setText(time); // 시간 업데이트

        byte[] n = new byte[6];
        // 상위 자리수를 배열의 뒷부분에 배치
        n[0] = 0;
        n[1] = 0;
        n[2] = (byte) (minutes / 10);
        n[3] = (byte) (minutes % 10);
        n[4] = (byte) (seconds / 10);
        n[5] = (byte) (seconds % 10);

        Log.d("segment", "1");
        mSegmentDriver.write(n, n.length);
        Log.d("segment", "2");

    }

//    private void startTimer() {
//        handler.post(this); // 현재 클래스의 run() 메서드를 실행
//    }
//
//    private void stopTimer() {
//        handler.removeCallbacks(this); // 현재 Runnable 제거
//    }
private void startTimer() {
    if (!isTimerRunning) {
        handler.post(this);
        startTime = System.currentTimeMillis() - pauseTime;
        isTimerRunning = true;
    }
}

    private void stopTimer() {
        if (isTimerRunning) {
            handler.removeCallbacks(this);
            pauseTime = System.currentTimeMillis() - startTime;
            isTimerRunning = false;
        }
    }


    @Override
    protected void onDestroy() {
        mSegmentDriver.close();
        super.onDestroy();

        stopTimer(); // 액티비티 종료 시 타이머 중지
    }


    private void loadPuzzlePieces() {
        Intent intent = getIntent();
        imageBytesList = (ArrayList<byte[]>) intent.getSerializableExtra("image_bytes_list");
        Collections.shuffle(pieceViews);  // 이미지 경로를 섞음
        if (imageBytesList != null && imageBytesList.size() == 4) {
            for (int i = 0; i < imageBytesList.size(); i++) {
                // byte[] -> Bitmap 변환
                Bitmap bitmap = BitmapFactory.decodeByteArray(imageBytesList.get(i), 0, imageBytesList.get(i).length);
                if (bitmap != null) {
                    int randomAngle = getRandomAngle(); // 랜덤 각도 생성
                    bitmap = applyRotateEffect(bitmap, randomAngle);

                    ImageView piece = pieceViews.get(i);
                    piece.setImageBitmap(bitmap);

                    // 태그에 인덱스와 올바른 회전 각도 저장
                    piece.setTag(new PieceTag(i, randomAngle));
                } else {
                    Toast.makeText(this, "Failed to load image part " + i, Toast.LENGTH_SHORT).show();
                }
            }
        } else {
            Toast.makeText(this, "Image load error", Toast.LENGTH_SHORT).show();
        }
    }


    private void setupPieceListeners() {
        for (ImageView piece : pieceViews) {
            piece.setOnClickListener(v -> {
                if (selectedPiece != v) {
                    if (selectedPiece != null) {
                        selectedPiece.clearColorFilter();
                    }
                    selectedPiece = (ImageView) v;
                    selectedPiece.setColorFilter(Color.argb(150, 255, 255, 0)); // 강조 표시
                    currentRotation = (int) selectedPiece.getRotation(); // 현재 회전 각도 업데이트
                }
            });
        }
    }

    private void setupRotationListener() {
        Button rotateButton = findViewById(R.id.rotate_button);

            rotateButton.setOnClickListener(v -> {
                if (selectedPiece != null) {
                    currentRotation += 90;
                    selectedPiece.setRotation(currentRotation % 360); // 조각 회전

                    // 업데이트된 회전 각도를 태그에 반영
                    PieceTag tag = (PieceTag) selectedPiece.getTag();
//                    if (tag != null) {
//                        tag.correctRotation = (tag.correctRotation + 90) % 360; // 회전 각도를 360도 이내로 유지
//                    }
                } else {
                    Toast.makeText(this, "회전할 조각을 선택하세요.", Toast.LENGTH_SHORT).show();
                }
            });
        }


    private void setupResetButton() {
        Button resetButton = findViewById(R.id.reset_button); // Reset 버튼 연결
        resetButton.setOnClickListener(v -> resetPuzzle());
    }



    private void setupSlotListeners() {
        ArrayList<ImageView> slots = new ArrayList<>(Arrays.asList(slot1, slot2, slot3, slot4));
        for (ImageView slot : slots) {
            slot.setOnClickListener(v -> {
                if (selectedPiece != null && slot.getDrawable() == null) {
                    slot.setImageBitmap(((BitmapDrawable) selectedPiece.getDrawable()).getBitmap());
                    slot.setRotation(selectedPiece.getRotation());
                    slot.setTag(selectedPiece.getTag()); // 조각의 식별자를 슬롯에 저장

                    selectedPiece.setVisibility(View.INVISIBLE); // 선택된 조각 숨김

                    // 모든 슬롯이 채워졌는지 확인하고 완성 검사
                    if (areAllSlotsFilled(slots)) {
                        if (isPuzzleComplete(slots)) {
                             endTime = System.currentTimeMillis();
                            duration = endTime - startTime;  // 소요 시간 계산
                            Toast.makeText(this, "Puzzle completed!", Toast.LENGTH_SHORT).show();
                            moveToResultScreen();
                        } else {
                            Toast.makeText(this, "Puzzle incorrect, resetting...", Toast.LENGTH_SHORT).show();
                            resetPuzzle(); // 퍼즐 리셋
                        }
                    }
                }
            });
        }
    }

    private boolean isPuzzleComplete(ArrayList<ImageView> slots) {
        boolean allCorrect = true;

        for (int i = 0; i < slots.size(); i++) {
            ImageView slot = slots.get(i);

            // 슬롯이 비어있거나 태그가 없으면 퍼즐이 완성되지 않음
            if (slot.getTag() == null) {
                allCorrect = false;
                break;
            }

            // 태그에서 인덱스와 회전 각도를 가져옴
            PieceTag tag = (PieceTag) slot.getTag();
            int correctIndex = tag.index;
            int correctRotation = tag.correctRotation;

            // 위치와 회전 각도를 검증
            if (correctIndex != i || correctRotation % 360 != 0) {
                allCorrect = false;
                break;
            }
        }

        return allCorrect;
    }


    private void resetPuzzle() {
        for (int i = 0; i < pieceViews.size(); i++) {
            ImageView piece = pieceViews.get(i);
            piece.setVisibility(View.VISIBLE); // 조각을 다시 보이게 함
            piece.setRotation(0); // 회전 초기화
            piece.clearColorFilter(); // 색상 필터 제거

            piece.setTag(new PieceTag(i, 0)); // 초기 상태: 올바른 위치와 회전 0도
        }
        for (ImageView slot : Arrays.asList(slot1, slot2, slot3, slot4)) {
            slot.setImageDrawable(null); // 슬롯 이미지 제거
        }
    }


    private boolean areAllSlotsFilled(ArrayList<ImageView> slots) {
        for (ImageView slot : slots) {
            if (slot.getDrawable() == null) {
                return false; // 하나라도 비어있는 슬롯이 있다면 false 반환
            }
        }
        return true; // 모든 슬롯이 채워져 있으면 true 반환
    }





    private void moveToResultScreen() {

        Intent intent = new Intent(this, ResultActivity.class);
        intent.putExtra("original_image_bytes", rotatedBitmapBytes);
        intent.putExtra("combined_image_bytes", combinedImageBytes);// 원본 이미지 경로 전달
        intent.putExtra("time_taken", duration);
        startActivity(intent);
        finish();
    }

    private int getRandomAngle() {
        int[] angles = {0, 90, 180, 270}; // 가능한 각도
        return angles[new java.util.Random().nextInt(angles.length)];
    }

    private Bitmap applyRotateEffect(Bitmap bitmap, int angle) {
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }

    @Override
    protected void onPause(){
        // TODO Auto-generated method stub
        //mInterruptDriver.close();
        mSegmentDriver.close();
        super.onPause();
    }


    @Override
    protected void onResume(){
        // TODO Auto-generated method stub

        super.onResume();
    }


    @Override
    public void onReceive(int val) {

    }





}






