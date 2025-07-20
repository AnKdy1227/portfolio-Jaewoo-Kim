package com.example.fp2;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.content.Intent;

import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;

import android.graphics.Paint;
import android.graphics.Canvas;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;

import com.example.fp2.databinding.ActivityMainBinding;
import com.google.firebase.crashlytics.buildtools.reloc.org.apache.commons.io.output.ByteArrayOutputStream;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("OpenCLDriver");
    }

    //OpenCL 함수 정의
    public native Bitmap GaussianBlurGPU(Bitmap bitmap);
    public native Bitmap Sobel(Bitmap bitmap);

    //카메라 객체 정의
    private Camera mCamera;
    private CameraPreview mPreview;
    private ImageView capturedImageHolder;


    private ActivityMainBinding binding;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main); // 첫 화면

        //사진 촬영 버튼
        Button btn = (Button) findViewById(R.id.button_capture);

        // 카메라 객체 생성
        mCamera = getCameraInstance(); // 뒤에서 try-catch로 정의
        if (mCamera == null) {
            Toast.makeText(this, "Failed to open camera", Toast.LENGTH_SHORT).show();
            return; // 카메라 열기 실패 시 함수 종료
        }
        mCamera.setDisplayOrientation(180);// 각도설정

        // 내가 지금 찍고 있는거.
        mPreview = new CameraPreview(this, mCamera);
        FrameLayout preview = (FrameLayout) findViewById(R.id.camera_preview);
        preview.addView(mPreview);

        // 버튼 입력시 해당 사진을 찍음
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mCamera.takePicture(null, null, pictureCallback);
            }
        });
    }
//-------------------------------------------

    //카메라 관련 메서드
    public static Camera getCameraInstance(){
        Camera c = null;
        try{
            c = Camera.open(); // attempt to get a Camera instance
        }
        catch (Exception e){
            // Camera is not available (in use of does not exist)
        }
        return c;
    }

    // capture버튼 클릭 -> 이미지 처리 -> 이미지와 레이아웃을 다음 xml로 보냄
    Camera.PictureCallback pictureCallback = new Camera.PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera){
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = 4;
            Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);

            //에러처리
            if(bitmap==null){
                Toast.makeText(MainActivity.this, "Capture image is empty", Toast.LENGTH_LONG).show();
                return;
            }

            //회전
            int w = bitmap.getWidth();
            int h = bitmap.getHeight();

            Matrix mtx = new Matrix();
            mtx.postRotate(180);
            Bitmap rotatedBitmap = Bitmap.createBitmap(bitmap, 0, 0 ,w,h,mtx,true);

            Bitmap rotatedBitmap2 = scaleDownBitmapImage(rotatedBitmap, 400, 400);


            // 다음 xml로 보내기 위한 객체
            Intent intent = new Intent(MainActivity.this, PuzzleActivity.class);

            // 전체 이미지를 조각내고 ArrayList에 저장
            ArrayList<Bitmap> processedPieces = processImage(rotatedBitmap2);

            ArrayList<byte[]> imageBytesList = new ArrayList<>();


            byte[] combinedImageBytes = combineAndSaveImageAsBytes(processedPieces);

            //4조각으ㅢ processedPieces를 imageBytesList에 넣음
            for (Bitmap piece : processedPieces) {
                // Bitmap -> byte[]
                imageBytesList.add(convertBitmapToByteArray(piece));
            }

            // 편집되기 전의 원본 이미지를 바이트로 저장
            byte[] rotatedBitmapBytes = convertBitmapToByteArray(rotatedBitmap2);



            intent.putExtra("image_bytes_list", imageBytesList); // 4조각
            intent.putExtra("edit", combinedImageBytes);  // 편집된 합본
            intent.putExtra("rotated_bitmap_bytes", rotatedBitmapBytes); // 원본
            startActivity(intent);
            finish();


        }
    };

    private Bitmap scaleDownBitmapImage(Bitmap bitmap, int newWidth, int newHeight){
        Bitmap resizedBitmap = Bitmap.createScaledBitmap(bitmap, newWidth, newHeight, true);
        return resizedBitmap;
    }

    // Bitmap -> byte[] 변환
    private byte[] convertBitmapToByteArray(Bitmap bitmap) {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 50, stream);
        return stream.toByteArray();
    }


    @Override
    protected void onPause(){
        super.onPause();
        releaseMediaRecorder();
        releaseCamera();
    }

    private void releaseMediaRecorder() { mCamera.lock();}

    private void releaseCamera(){
        if(mCamera != null){
            mCamera.release();
            mCamera = null;
        }
    }
    // 이미지 처리 및 4등분
    private ArrayList<Bitmap> processImage(Bitmap bitmap) {
        ArrayList<Bitmap> pieces = new ArrayList<>();
        int width = bitmap.getWidth() / 2;
        int height = bitmap.getHeight() / 2;

        // 4등분
        Bitmap piece1 = Bitmap.createBitmap(bitmap, 0, 0, width, height);
        Bitmap piece2 = Bitmap.createBitmap(bitmap, width, 0, width, height);
        Bitmap piece3 = Bitmap.createBitmap(bitmap, 0, height, width, height);
        Bitmap piece4 = Bitmap.createBitmap(bitmap, width, height, width, height);

        // 각 조각에 대해 랜덤 이미지 처리 적용
        pieces.add(applyRandomEffect(piece1));
        pieces.add(applyRandomEffect(piece2));
        pieces.add(applyRandomEffect(piece3));
        pieces.add(applyRandomEffect(piece4));

        return pieces;
    }

    // 랜덤 이미지 처리 적용
    private Bitmap applyRandomEffect(Bitmap bitmap) {
        // 이미지 처리 함수 배열
        BitmapProcessor[] processors = new BitmapProcessor[]{
                this::applyGrayscaleEffect,// 흑백
                this::applyInvertEffect, // 색반전
                this::GaussianBlurGPU, // 블러
                this::Sobel // 소벨 필터
        };

        // 랜덤 함수 선택
        int randomIndex = new java.util.Random().nextInt(processors.length);
        return processors[randomIndex].process(bitmap);
    }

    // 인터페이스 정의 (람다로 이미지 처리 함수 사용)
    @FunctionalInterface
    interface BitmapProcessor {
        Bitmap process(Bitmap bitmap);
    }

// 기존 처리 함수들

    // 그레이스케일 효과
    private Bitmap applyGrayscaleEffect(Bitmap bitmap) {
        Bitmap grayscaleBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(grayscaleBitmap);
        Paint paint = new Paint();
        ColorMatrix colorMatrix = new ColorMatrix();
        colorMatrix.setSaturation(0);
        ColorMatrixColorFilter filter = new ColorMatrixColorFilter(colorMatrix);
        paint.setColorFilter(filter);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return grayscaleBitmap;
    }



    // 색 반전 효과
    private Bitmap applyInvertEffect(Bitmap bitmap) {
        Bitmap invertedBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(invertedBitmap);
        Paint paint = new Paint();
        ColorMatrix colorMatrix = new ColorMatrix(new float[]{
                -1, 0, 0, 0, 255,
                0, -1, 0, 0, 255,
                0, 0, -1, 0, 255,
                0, 0, 0, 1, 0
        });
        ColorMatrixColorFilter filter = new ColorMatrixColorFilter(colorMatrix);
        paint.setColorFilter(filter);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return invertedBitmap;
    }

    private byte[] combineAndSaveImageAsBytes(ArrayList<Bitmap> pieces) {
        if (pieces.size() != 4) {
            Log.e("MainActivity", "Invalid number of pieces to combine.");
            return null;
        }

        // 각 조각의 크기를 기준으로 전체 이미지 크기를 계산
        int pieceWidth = pieces.get(0).getWidth();
        int pieceHeight = pieces.get(0).getHeight();
        int combinedWidth = pieceWidth * 2;
        int combinedHeight = pieceHeight * 2;

        // 빈 Bitmap 생성
        Bitmap combinedImage = Bitmap.createBitmap(combinedWidth, combinedHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(combinedImage);

        // 각 조각을 해당 위치에 그리기
        canvas.drawBitmap(pieces.get(0), 0, 0, null); // 왼쪽 위
        canvas.drawBitmap(pieces.get(1), pieceWidth, 0, null); // 오른쪽 위
        canvas.drawBitmap(pieces.get(2), 0, pieceHeight, null); // 왼쪽 아래
        canvas.drawBitmap(pieces.get(3), pieceWidth, pieceHeight, null); // 오른쪽 아래

        // Bitmap -> byte[] 변환
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        combinedImage.compress(Bitmap.CompressFormat.PNG, 100, stream);
        return stream.toByteArray();
    }




    /**
     * A native method that is implemented by the 'fp2' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}