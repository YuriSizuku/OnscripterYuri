package com.yuri.onscripter;

import android.app.ActionBar;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.View;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.ArrayList;

public class ONScripter extends SDLActivity {
    static {
        System.loadLibrary("lua");
        System.loadLibrary("jpeg");
        System.loadLibrary("bz2");
        System.loadLibrary("onsyuri");
    }

    private ArrayList<String> m_onsargs;
    private native int nativeInitJavaCallbacks();

    public int getFD(char[] path, int mode){
        // Log.i("## onsyuri_android", new String(path));
        File file = new File(new String(path));
        return -1;
    }

    public int mkdir(char[] path){
        return 0;
    }

    public void playVideo(char[] cArr) {
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
        return  (String[]) m_onsargs.toArray(new String[0]);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        m_onsargs = intent.getStringArrayListExtra("args");
        nativeInitJavaCallbacks();
        this.fullscreen();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        this.fullscreen();
    }


    public void onWindowFocusChanged (boolean hasFocus){
        if(hasFocus) this.fullscreen();
    }

    private void fullscreen(){
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        // hide title for SDL
        try {
            this.getSupportActionBar().hide();
        }
        catch (NullPointerException e){}
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
}
