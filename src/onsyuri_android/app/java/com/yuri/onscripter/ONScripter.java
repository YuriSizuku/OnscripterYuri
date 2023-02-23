package com.yuri.onscripter;

import static com.yuri.onscripter.MainActivity.SHAREDPREF_GAMEARGS;
import static com.yuri.onscripter.MainActivity.SHAREDPREF_GAMEURI;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.documentfile.provider.DocumentFile;

import org.libsdl.app.SDLActivity;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Map;
import java.util.Objects;

public class ONScripter extends SDLActivity {
    private ArrayList<String> m_onsargs;
    private DocumentFile m_onsbase; // use this to judge whether to use saf

    // for onsyuri c code
    private native int nativeInitJavaCallbacks();

    public int getFD(byte[] pathbyte, int mode) {
        if (m_onsbase==null) return -1;
        String path = new String(pathbyte, StandardCharsets.UTF_8);
        String safmode = mode==0 ? "r" : "w";
        int fd = SafFile.getFdSaf(this, m_onsbase, path, safmode);
        Log.i("## onsyuri_android", String.format("getFD path=%s, mode=%d, fd=%d", new String(path), mode, fd));
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
//        try {
//            String replace = ("file://" + gCurrentDirectoryPath + "/" + new String(cArr)).replace('\\', '/');
//            StringBuilder sb = new StringBuilder();
//            sb.append("playVideo: ");
//            sb.append(replace);
//            Log.v("ONS", sb.toString());
//            Uri parse = Uri.parse(replace);
//            Intent intent = new Intent("android.intent.action.VIEW");
//            intent.setDataAndType(parse, "video/*");
//            startActivityForResult(intent, -1);
//        } catch (Exception e) {
//            Log.e("ONS", "playVideo error:  " + e.getClass().getName());
//        }
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

        m_onsargs = intent.getStringArrayListExtra(SHAREDPREF_GAMEARGS);
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
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        // hide title for SDL
        try {
            Objects.requireNonNull(this.getSupportActionBar()).hide();
        }
        catch (NullPointerException ignored){}
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
}
