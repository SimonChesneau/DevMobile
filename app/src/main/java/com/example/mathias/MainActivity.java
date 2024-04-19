package com.example.mathias;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.Manifest;

import com.example.mathias.databinding.ActivityMainBinding;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'sharingan_camera' library on application startup.
    static {
        System.loadLibrary("mathias");
    }

    public static final String TAG = "NativeBarcodeTracker";
    private int CAMERA_PERMISSION_CODE = 1;

    SurfaceView mSurfaceView;
    Button mButtonFlipCamera;
    SurfaceHolder mSurfaceHolder;

    public native void initNativeCode(String filePath, AssetManager assetManager);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        String filesDir = getFilesDir().getAbsolutePath();
        AssetManager assetManager = getAssets();
        try {
            InputStream in = assetManager.open("haarcascade_frontalface_default.xml");
            File outFile = new File(getApplicationContext().getFilesDir(), "haarcascade_frontalface_default.xml");
            OutputStream out = new FileOutputStream(outFile);
            copyFile(in, out);
            out.flush();
        } catch (Exception e) {
            int t=0;
        }

        initNativeCode(filesDir, assetManager);

        mSurfaceView = (SurfaceView)findViewById(R.id.texturePreview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mButtonFlipCamera = (Button)findViewById(R.id.FlipCamera);

        mSurfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                if (ContextCompat.checkSelfPermission(
                        MainActivity.this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
                    // Sends surface buffer to NDK
                    setSurface(surfaceHolder.getSurface());
                } else {
                    requestCameraPermission();
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

            }
        });
        // Récupérer les dimensions de l'écran
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int screenWidth = metrics.widthPixels;
        int screenHeight = metrics.heightPixels;

        mButtonFlipCamera.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                flipCamera(mSurfaceHolder.getSurface());
            }
        });

    }

    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1];
        int read;
        while((read = in.read(buffer)) != -1){
            out.write(buffer, 0, read);
        }
    }

    private void requestCameraPermission() {
        ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA}, CAMERA_PERMISSION_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == CAMERA_PERMISSION_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // Sends surface buffer to NDK
                setSurface(mSurfaceHolder.getSurface());
            } else {
                Toast.makeText(getApplicationContext(), "Please allow permission to use the camera",
                        Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        releaseCVMain();
    }


    // Sends surface buffer to NDK
    public native void setSurface(Surface surface);

    public native void flipCamera(Surface surface);
    // Release native resources
    public native void releaseCVMain();
}