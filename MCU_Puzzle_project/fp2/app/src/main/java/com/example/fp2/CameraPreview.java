package com.example.fp2;


import android.content.Context;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback{
    private SurfaceHolder mHolder;
    private Camera mCamera;

    public CameraPreview(Context context, Camera camera) {
        super(context);
        mCamera = camera;
        mHolder = getHolder();
        mHolder.addCallback(this);
    }

    public void surfaceCreated(SurfaceHolder holder){
        try{
            // create the surface and start camera preview
            if(mCamera == null) {
                mCamera.setPreviewDisplay(holder);
                mCamera.startPreview();
            }
        } catch (IOException e) {
            Log.d(VIEW_LOG_TAG, "Error setting camera preview: " + e.getMessage());
        }

    }

    public void refreshCamera(Camera camera){
        if(mHolder.getSurface() == null) {
            //preview surface does not exist
            return;
        }
        // stop preview before making changes
        try{
            mCamera.stopPreview();
        } catch(Exception e){
            //ignore: tried to stop a non-existent preview
        }
        setCamera(camera);
        try{
            mCamera.setPreviewDisplay(mHolder);
            mCamera.startPreview();
        } catch (Exception e){
            Log.d(VIEW_LOG_TAG,"Error starting camera preview: " + e.getMessage());
        }
    }
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h){
        refreshCamera(mCamera);
    }

    public void setCamera(Camera camera){
        mCamera= camera;
    }
    @Override
    public void surfaceDestroyed(SurfaceHolder holder){
        // TODO Auto-generated method stub
        // mCamera.release();
    }

}

