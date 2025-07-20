package com.example.fp2;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class ResultActivity extends AppCompatActivity {

    private ImageView originalImageView, completedPuzzleImageView;
    private TextView timeTakenText;
    private Button backButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_thr);



        // XML 요소 초기화
        originalImageView = findViewById(R.id.original_image);
        completedPuzzleImageView = findViewById(R.id.completed_puzzle_image);
        timeTakenText = findViewById(R.id.time_taken_text);
        backButton = findViewById(R.id.back_button);

        // Intent로 전달받은 데이터 가져오기
        Intent intent = getIntent();
        //String originalImagePath = intent.getStringExtra("original_image_path"); // 변경된 키 이름
        //String completedPuzzleImagePath = intent.getStringExtra("combined_image_path");

        long timeTaken = getIntent().getLongExtra("time_taken", 0);



        // Intent에서 바이트 배열 수신
        byte[] rotatedBitmapBytes = getIntent().getByteArrayExtra("original_image_bytes");
        if (rotatedBitmapBytes != null) {
            Bitmap rotatedBitmap = BitmapFactory.decodeByteArray(rotatedBitmapBytes, 0, rotatedBitmapBytes.length);
            // 받은 Bitmap으로 작업 수행
            originalImageView.setImageBitmap(rotatedBitmap);

        }


        byte[] combinedBitmapBytes = getIntent().getByteArrayExtra("combined_image_bytes");
        if (combinedBitmapBytes != null) {
            Bitmap combinedBitmap = BitmapFactory.decodeByteArray(combinedBitmapBytes, 0, combinedBitmapBytes.length);
            // 받은 Bitmap으로 작업 수행
            completedPuzzleImageView.setImageBitmap(combinedBitmap);

        }




//        if (completedPuzzleImagePath != null) {
//            Bitmap completedBitmap = BitmapFactory.decodeFile(completedPuzzleImagePath);
//            completedPuzzleImageView.setImageBitmap(completedBitmap);
//        }

        timeTakenText.setText("Total time: " + formatDuration(timeTaken));
        // 돌아가기 버튼 클릭 이벤트
        backButton.setOnClickListener(v -> {
            Intent mainIntent = new Intent(ResultActivity.this, MainActivity.class);
            mainIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(mainIntent);
            finish();
        });
    }

    private String formatDuration(long duration) {
        long seconds = (duration / 1000) % 60;
        long minutes = (duration / (1000 * 60)) % 60;
        long hours = duration / (1000 * 60 * 60);

        return String.format("%02d:%02d:%02d", hours, minutes, seconds);
    }

}
