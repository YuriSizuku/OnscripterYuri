package com.yuri.onscripter;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import java.io.File;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {
    private  ArrayList<String> m_onsargs;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        m_onsargs = new ArrayList();

        File filedir = this.getFilesDir();
        File filedir2 = this.getExternalFilesDir(null);
        Log.i("## onsyuri_android", filedir.getAbsolutePath() + ";" + filedir2.getAbsolutePath());
        boolean ret = filedir2.canRead();
        ret = filedir2.canWrite();
        File file2 = new File("/storage/emulated/0/Android/data/com.yuri.onscripter/files/game/test/");
        Log.i("## onsyuri_android", "can_write " + file2.canWrite());

        runOnscripter();
    }

    public void runOnscripter() {
        m_onsargs.add("--root");
        m_onsargs.add("/storage/emulated/0/Android/data/com.yuri.onscripter/files/game/test");
        m_onsargs.add("--font");
        m_onsargs.add("/storage/emulated/0/Android/data/com.yuri.onscripter/files/game/test/default.ttf");
        m_onsargs.add("--fullscreen");
        m_onsargs.add("--debug:1");
//        m_onsargs.add("--sharpness");
//        m_onsargs.add("12.0");

        Intent intent = new Intent();
        intent.setClass(this, ONScripter.class);
        intent.putStringArrayListExtra("args", m_onsargs);
        startActivity(intent);
    }
}