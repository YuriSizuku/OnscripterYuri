package com.yuri.onscripter;

import static android.provider.DocumentsContract.Document.MIME_TYPE_DIR;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Objects;

public class MainActivity extends AppCompatActivity {

    private static final int ACTIVITY_SAF = 10;
    private static final String COLOR_NOTDIR = "#FF0000";
    private static final String COLOR_INRDIR = "#4CAF50";
    private static final String COLOR_SAFDIR = "#FF26C6DA";
    private static final String COLOR_EXTDIR = "#FFD4E157";
    public static final String SHAREDPREF_NAME = "onsyuri";
    public static final String SHAREDPREF_GAMEDIR = "gamedir";
    public static final String SHAREDPREF_GAMEURI = "gameuri";
    public static final String SHAREDPREF_GAMEARGS = "gameargs";

    private

    enum DIR_TYPE{
        NOT_DIR,
        NORMAL_DIR,
        SAF_DIR
    }

    class GameInfo {
        public boolean m_isUri;
        public String m_path;
        public String m_name = null;
        public GameInfo(String path, boolean isUri) {
            m_isUri = isUri;
            m_path = path;
            if(m_isUri){
                Uri uri =  Uri.parse(path);
                String mimeType = getContentResolver().getType(uri);
                if(mimeType.equals(MIME_TYPE_DIR)) {
                   Cursor cursor = getContentResolver().query(uri, null, null, null, null);
                    if(cursor!=null) {
                        int nameidx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                        cursor.moveToFirst();
                        m_name = cursor.getString(nameidx);
                        cursor.close();
                    }
                }
            }
            else { // check if valid dir
                File file = new File(m_path);
                if(file.isDirectory())  {
                    m_name = file.getName();
                }
            }
        }

        public String getPathType(){
            if(m_isUri) return "saf";
            if(m_path.contains("emulated")) return "inr";
            else return "ext";
        }

        public String getShortPath(){
            String path = m_path;
            if(m_isUri){
                path = Uri.decode(path);
                int idx = path.lastIndexOf(0x3A);
                path = path.substring(idx+1);
            }
            return path;
        }

        public Bitmap getIcon(){
            Bitmap bitmap = null;
            if(m_isUri){ // append uri with %2f (`/`)
                Uri uri = Uri.parse(m_path +"%2Ficon.png");
                if(uri!=null) {
                    Context context = getApplicationContext();
                    DocumentFile doc = DocumentFile.fromSingleUri(context, uri);
                    if(doc!=null){
                        if(doc.canRead()){
                            try {
                                InputStream in = context.getContentResolver().openInputStream(doc.getUri());
                                bitmap = BitmapFactory.decodeStream(in);
                            }
                            catch (FileNotFoundException e){
                                return  null;
                            }
                        }
                    }
                }
            }
            else {
                String iconpath;
                if(m_path.charAt(m_path.length()-1) == '/') {
                    iconpath = m_path + "icon.png";
                }
                else{
                    iconpath = m_path + "/icon.png";
                }
                bitmap = BitmapFactory.decodeFile(iconpath);
            }
            return bitmap;
        }
    }

    class GamelistAdapter extends BaseAdapter {
        private LayoutInflater m_inflater;
        protected class ItemViewHolder { // cache view
            ImageButton button_gameicon;
            TextView text_gametitle;
            TextView text_pathtype;
            TextView text_gameno;
        }

        @Override
        public int getCount() {
            if(m_gamelist!=null) return m_gamelist.size();
            else return 0;
        }

        @Override
        public Object getItem(int position) {
            if(m_gamelist!=null) return m_gamelist.get(position);
            else return null;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @SuppressLint({"InflateParams", "SetTextI18n", "DefaultLocale"})
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LinearLayout rootview;
            ItemViewHolder holder;
            if (convertView==null) {
                holder=new ItemViewHolder();
                if(m_inflater==null) m_inflater = LayoutInflater.from(parent.getContext());
                rootview= (LinearLayout) m_inflater.inflate(R.layout.layout_gameinfo, null);
                holder.button_gameicon = rootview.findViewById(R.id.button_gameicon);
                holder.text_gametitle = rootview.findViewById(R.id.text_gametitle);
                holder.text_pathtype = rootview.findViewById(R.id.text_pathtype);
                holder.text_gameno = rootview.findViewById(R.id.text_gameno);
                rootview.setTag(holder);
            }
            else {
                rootview=(LinearLayout)convertView;
                holder=(ItemViewHolder)convertView.getTag();
            }

            if(m_gamelist!=null) {
                Context context = getApplicationContext();
                GameInfo gameinfo = m_gamelist.get(position);

                // game title
                String shortpath = "";
                Configuration config = context.getResources().getConfiguration();
                if(config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    shortpath = " | " + gameinfo.getShortPath();
                }
                holder.text_gametitle.setText(" " + gameinfo.m_name + shortpath);

                // game icon
                Bitmap bitmap = gameinfo.getIcon();
                if(bitmap==null) {
                    bitmap = BitmapFactory.decodeResource(context.getResources(), R.mipmap.ic_launcher);
                }
                holder.button_gameicon.setImageBitmap(bitmap);

                // game path type
                String pathtype = gameinfo.getPathType();
                switch (pathtype){
                    case "inr":
                        holder.text_pathtype.setTextColor(Color.parseColor(COLOR_INRDIR));
                        break;
                    case "ext":
                        holder.text_pathtype.setTextColor(Color.parseColor(COLOR_EXTDIR));
                        break;
                    case "saf":
                        holder.text_pathtype.setTextColor(Color.parseColor(COLOR_SAFDIR));
                        break;
                    default:
                        holder.text_pathtype.setTextColor(Color.parseColor(COLOR_NOTDIR));
                        break;
                }
                holder.text_pathtype.setText(pathtype);

                // game number
                holder.text_gameno.setText(String.format("%d/%d", position + 1,  this.getCount()));

                // game click
                rootview.setOnClickListener(event->{
                    Log.i("## convertView", "on click " + position);
                    boolean usesaf = gameinfo.m_isUri;
                    String gamedir = usesaf ? gameinfo.m_name : gameinfo.m_path;
                    startOnsyuri(gamedir, null, usesaf);
                });
            }
            return rootview;
        }
    }

    // onsyuri value
    private JSONObject m_onsargsjson;
    private String[] m_appdirs;

    private String m_gamedir = null; // direct dir without saf
    private ArrayList<GameInfo> m_gamelist;
    private GamelistAdapter m_listgameadaptor;
    private SharedPreferences m_sharedpref;

    // onsyuri ui
    private EditText m_textgamedir;
    private ImageButton m_buttonconfig;
    private ImageButton m_buttonexplore;
    private ListView m_listgame;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        File file = new File("/storage/emulated/0/Local/Game/ons_game/Noesis 羽化/0.txt");
        try {
            FileInputStream fin = new FileInputStream(file);
            int c = fin.read();
        }
        catch (FileNotFoundException e){

        } catch (IOException e) {
            throw new RuntimeException(e);
        }


        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initValue();
        initUi();
        updateGamelist();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent resultData){
        super.onActivityResult(requestCode, resultCode, resultData);
        if (requestCode == ACTIVITY_SAF) {
            Uri uri = resultData.getData();
            m_textgamedir.setText(uri.toString());
            SafFile.saveDocUri(resultData, requestCode);
            m_gamedir = null;
            m_sharedpref.edit().remove(SHAREDPREF_GAMEDIR).apply();
            updateGamelist();
        }
    }

    @Override
    public void onConfigurationChanged (@NonNull Configuration newConfig){
        super.onConfigurationChanged(newConfig);
        m_listgameadaptor.notifyDataSetChanged();
    }
    // onsyuri android info function
    private DIR_TYPE checkDir(String path){
        try {
            File file = new File(path);
            if(file.isDirectory()) { // if input valid path
                return DIR_TYPE.NORMAL_DIR;
            }
            else {
                Uri uri = Uri.parse(m_gamedir);
                DocumentFile doc = DocumentFile.fromTreeUri(this, uri);
                if(doc!=null && doc.isDirectory()) return DIR_TYPE.SAF_DIR;
                else return DIR_TYPE.NOT_DIR;
            }
        }
        catch (IllegalArgumentException e){
            return  DIR_TYPE.NOT_DIR;
        }

    }

    private void initValue(){
        // init valus
        m_onsargsjson = new JSONObject();
        m_gamelist = new ArrayList<>();

        // load from sharedpref
        m_sharedpref = getSharedPreferences(SHAREDPREF_NAME, MODE_PRIVATE);
        m_gamedir = m_sharedpref.getString(SHAREDPREF_GAMEDIR, null);

        // init dirs
        SafFile.init(this, m_sharedpref);
        SafFile.g_sharedprefname = SHAREDPREF_GAMEURI;
        m_appdirs = SafFile.getAppDirectories();
    }

    private void startOnsyuri(String gamedir, String savedir, boolean usesaf) {
        // storage/emulated/0/Android/data/com.yuri.onscripter/files/game/test
        Log.i("## onsyuri_anroid", String.format("gamedir %s, savedir %s, usesaf %d", gamedir, savedir, usesaf ? 1:0));
        ArrayList<String> onsargs = new ArrayList<>();
        onsargs.add("--root");
        onsargs.add(gamedir);
        onsargs.add("--font");
        onsargs.add(gamedir + "/default.ttf");
        if (savedir!=null) {
            onsargs.add("--save-dir");
            onsargs.add(savedir);
        }

        Intent intent = new Intent();
        intent.setClass(this, ONScripter.class);
        if(usesaf){
            Uri uri = SafFile.loadDocUri();
            if(uri!=null) intent.putExtra(SHAREDPREF_GAMEURI, uri.toString());
        }
        intent.putStringArrayListExtra(SHAREDPREF_GAMEARGS, onsargs);
        startActivity(intent);
    }

    // onsyuri android ui function
    private void initUi(){

        // gamedir button
        m_buttonexplore= findViewById(R.id.button_explore);
        m_buttonexplore.setOnClickListener(view -> SafFile.requestDocUri(this, ACTIVITY_SAF));

        // setting button
        m_buttonconfig = findViewById(R.id.button_config);
        m_buttonconfig.setOnClickListener(view->{

        });

        // gamdir text
        m_textgamedir = findViewById(R.id.text_gamedir);
        if (m_gamedir!=null) {
            updateGameDir();
        }
        else {
            Uri uri = SafFile.loadDocUri();
            if(uri!=null) m_gamedir = uri.toString();
            updateGameDir();
            m_gamedir = null;
        }
        m_textgamedir.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
            @Override
            public void afterTextChanged(Editable s) {
                m_gamedir = s.toString();
                DIR_TYPE dir_type = updateGameDir();
                if(dir_type!=DIR_TYPE.NOT_DIR) updateGamelist();
            }
        });

        // gamelist view
        m_listgame = findViewById(R.id.list_game);
        m_listgameadaptor = new GamelistAdapter();
        m_listgame.setAdapter(m_listgameadaptor);
    }

    private DIR_TYPE updateGameDir(){
        String _gamedir = m_textgamedir.getText().toString();
        DIR_TYPE dir_type = checkDir(m_gamedir);
        switch (dir_type) {
            case NOT_DIR:
                m_sharedpref.edit().remove(SHAREDPREF_GAMEDIR).apply();
                m_textgamedir.setTextColor(Color.parseColor(COLOR_NOTDIR));
                break;
            case NORMAL_DIR:
                SafFile.removeDocUri();
                m_sharedpref.edit().putString(SHAREDPREF_GAMEDIR, m_gamedir).apply();
                m_textgamedir.setTextColor(Color.parseColor(COLOR_INRDIR));
                break;
            case SAF_DIR:
                m_sharedpref.edit().remove(SHAREDPREF_GAMEDIR).apply();
                m_textgamedir.setTextColor(Color.parseColor(COLOR_SAFDIR));
        }
        if(!_gamedir.equals(m_gamedir)) m_textgamedir.setText(m_gamedir);
        return dir_type;
    }

    private void updateGamelist() {
        m_gamelist.clear();

        // check app dirs
        for(String appdir: m_appdirs){
            File dir = new File(appdir);
            for(File file: Objects.requireNonNull(dir.listFiles())){
                if(file==null) continue;
                GameInfo gameinfo =  new GameInfo(file.getAbsolutePath(), false);
                if(gameinfo.m_name!=null)  m_gamelist.add(gameinfo);
            }
        }

        // check game dir without saf
        if(m_gamedir!=null){
            File dir = new File(m_gamedir);
            File[] files = dir.listFiles();
            if(files!=null){
                for(File file: files ){
                    if(file==null) continue;
                    GameInfo gameinfo =  new GameInfo(file.getAbsolutePath(), false);
                    if(gameinfo.m_name!=null)  m_gamelist.add(gameinfo);
                }
            }
        }

        // check selected dir by saf
        DocumentFile doc = SafFile.getBaseDocumentFile();
        if(doc!=null){
            for(DocumentFile docfile : doc.listFiles()) {
                if(docfile==null) continue;
                GameInfo gameinfo =  new GameInfo(docfile.getUri().toString(), true);
                if(gameinfo.m_name!=null)  m_gamelist.add(gameinfo);
            }
        }
        m_listgameadaptor.notifyDataSetChanged();
    }
}