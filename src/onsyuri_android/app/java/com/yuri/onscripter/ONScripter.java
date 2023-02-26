package com.yuri.onscripter;

import static com.yuri.onscripter.MainActivity.SHAREDPREF_GAMECONFIG;
import static com.yuri.onscripter.MainActivity.SHAREDPREF_GAMEURI;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.appcompat.app.AlertDialog;
import androidx.core.content.FileProvider;
import androidx.documentfile.provider.DocumentFile;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Objects;

public class ONScripter extends SDLActivity {
    private ArrayList<String> m_onsargs;
    // use this to judge whether to use saf
    private DocumentFile m_onsbase;

    // for onsyuri c code
    private native int nativeInitJavaCallbacks();

    @SuppressWarnings("UnnecessaryLocalVariable")
    public int getFD(byte[] pathbyte, int mode) {
        if (m_onsbase==null) return -1;
        String path = new String(pathbyte, StandardCharsets.UTF_8);
        String safmode = mode==0 ? "r" : "w";
        int fd = SafFile.getFdSaf(this, m_onsbase, path, safmode);
        // Log.i("## onsyuri_android", String.format("getFD path=%s, mode=%d, fd=%d", new String(path), mode, fd));
        return fd;
    }

    public int mkdir(byte[] pathbyte) {
        if (m_onsbase==null) return -1;
        String path = new String(pathbyte, StandardCharsets.UTF_8);
        DocumentFile doc = SafFile.mkdirsSaf(m_onsbase, path);
        if (doc==null) return  -1;
        else  return 0;
    }

    public void playVideo(byte[] pathbyte) {
        String path = new String(pathbyte, StandardCharsets.UTF_8);
        try {
            Uri uri;
            path = path.replace('\\', '/');
            if(m_onsbase==null)  {
                // scoped storage, content://com.yuri.onscripter.fileProvider/external_file_path/
                uri = FileProvider.getUriForFile(this,
                        Objects.requireNonNull(getClass().getPackage()).getName() +".fileProvider",
                        new File(path));
            }
            else uri = Uri.parse(m_onsbase.getUri() + "%2F" + Uri.encode(path));
            Log.i("## onsyuri_android", "playVideo " + uri.toString());
            invokeSystemPlayer(uri);

        } catch (Exception e) {
            Log.e("## onsyuri_android", "playVideo error:  " + e.getClass().getName() + " "+ e.getMessage());
        }
    }

    private void invokeSystemPlayer(Uri uri){
        AlertDialog.Builder builder = new AlertDialog.Builder(ONScripter.this);
        builder.setTitle("Playing game video");
        StringBuilder message = new StringBuilder();
        message.append("playVideo: ").append(uri.toString()).append("\n");
        message.append("Please select a video app to play ! ");
        AlertDialog.Builder builder1 = builder.setMessage(message);
        builder.setCancelable(false);
        builder.setPositiveButton("Yes", (dialog, which) -> {
            dialog.dismiss();
            Intent intent = new Intent("android.intent.action.VIEW");
            intent.setDataAndType(uri, "video/*");
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            startActivityForResult(intent, -1);
        });
        builder.setNegativeButton("No", (dialog, which) -> dialog.dismiss());
        runOnUiThread(() -> {
            AlertDialog dialog = builder.create();
            dialog.show();
        });
    }

    // override sdl functions
    static {
        System.loadLibrary("lua");
        System.loadLibrary("jpeg");
        System.loadLibrary("bz2");
        System.loadLibrary("onsyuri");
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_image",
                "SDL2_mixer",
                "SDL2_ttf",
                "onsyuri"
        };
    }

    @Override
    protected String[] getArguments() {
        return m_onsargs.toArray(new String[0]);
    }

    // override activity functions
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();

        m_onsargs = intent.getStringArrayListExtra(SHAREDPREF_GAMECONFIG);
        String uristr = intent.getStringExtra(SHAREDPREF_GAMEURI);
        if(uristr!=null) {
            Uri uri = Uri.parse(uristr);
            m_onsbase = DocumentFile.fromTreeUri(this, uri);
        }
        else {
            m_onsbase = null;
        }

        nativeInitJavaCallbacks();
        this.fullscreen();
    }

    @Override
    protected void onResume() {
        super.onResume();
        this.fullscreen();
    }


    public void onWindowFocusChanged (boolean hasFocus) {
        if(hasFocus) this.fullscreen();
    }

    private void fullscreen() {
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        // hide title for SDL
        try {
            Objects.requireNonNull(this.getSupportActionBar()).hide();
        }
        catch (NullPointerException ignored){}
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
}
