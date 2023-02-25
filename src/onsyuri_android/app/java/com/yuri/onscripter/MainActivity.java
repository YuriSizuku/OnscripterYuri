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
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashSet;
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
    public static final String SHAREDPREF_GAMECONFIG = "gameargs";

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

        public String getGameDir() {
            return  m_isUri ? m_name : m_path;
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
            if(m_isUri) { // append uri with %2f (`/`)
                Uri uri = Uri.parse(m_path +"%2Ficon.png");
                if(uri!=null) {
                    Context context = getApplicationContext();
                    DocumentFile doc = DocumentFile.fromSingleUri(context, uri);
                    if(doc!=null && doc.canRead()){
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
            else {
                String iconpath;
                if(m_path.charAt(m_path.length()-1) == '/') iconpath = m_path + "icon.png";
                else iconpath = m_path + "/icon.png";
                bitmap = BitmapFactory.decodeFile(iconpath);
            }
            return bitmap;
        }

        public int checkValid() {
            int res = 0;
            boolean has_script = false;
            boolean has_font = false;
            HashSet<String> ons_script = new HashSet<>();
            ons_script.add("0.txt");
            ons_script.add("00.txt");
            ons_script.add("nscr_sec.dat");
            ons_script.add("nscript.dat");
            ons_script.add("onscript.nt2");
            ons_script.add("onscript.nt3");

            if(m_isUri){
                Context context = getApplicationContext();
                Uri uri = Uri.parse(m_path);
                DocumentFile docfile = DocumentFile.fromTreeUri(context, uri);
                if(Objects.requireNonNull(docfile).canRead()) res += 4;
                if(docfile.canWrite()) res += 2;

                uri = Uri.parse(m_path +"%2Fdefault.ttf");
                docfile = DocumentFile.fromSingleUri(context, uri);
                if(docfile!=null && docfile.exists()){
                    has_font = true;
                }
                if(has_font){
                    for (String name: ons_script){
                        uri = Uri.parse(m_path +"%2F" + name);
                        docfile = DocumentFile.fromSingleUri(context, uri);
                        if(docfile!=null && docfile.exists()){
                            has_script = true;
                            break;
                        }
                    }
                }
            }
            else {
                File file = new File(m_path);
                if(file.canRead()) res += 4;
                if(file.canWrite()) res += 2;
                for(String path : Objects.requireNonNull(file.list())){
                    if(ons_script.contains(path)) has_script = true;
                    if(path.equals("default.ttf")) has_font = true;
                    if(has_script && has_font) break;
                }
            }
            if(has_script && has_font) res += 1;
            return res;
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
            if (convertView==null) { // cache the view
                holder=new ItemViewHolder();
                if(m_inflater==null) m_inflater = LayoutInflater.from(parent.getContext());
                rootview= (LinearLayout) m_inflater.inflate(R.layout.layout_gameinfo, null);
                holder.button_gameicon = rootview.findViewById(R.id.button_gameicon);
                holder.text_gametitle = rootview.findViewById(R.id.text_gametitle);
                holder.text_pathtype = rootview.findViewById(R.id.text_pathtype);
                holder.text_gameno = rootview.findViewById(R.id.text_gameno);
                rootview.setTag(holder);
            }
            else { // obtain from cache
                rootview=(LinearLayout)convertView;
                holder=(ItemViewHolder)convertView.getTag();
            }
            if (m_gamelist==null) return rootview;

            getGameInfoView(holder, position);
            getGameDetailView(rootview, holder, position);

            return rootview;
        }

        @SuppressLint({"SetTextI18n", "DefaultLocale"})
        private void getGameInfoView(ItemViewHolder holder, int position) {
            GameInfo gameinfo = m_gamelist.get(position);

            // gamelist title
            String shortpath = "";
            Configuration config = MainActivity.this.getResources().getConfiguration();
            if(config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                shortpath = " | " + gameinfo.getShortPath();
            }
            holder.text_gametitle.setText(" " + gameinfo.m_name + shortpath);

            // gamelist icon
            Bitmap bitmap = gameinfo.getIcon();
            if(bitmap==null) {
                bitmap = BitmapFactory.decodeResource(MainActivity.this.getResources(), R.mipmap.ic_launcher);
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
        }

        private void getGameDetailView(LinearLayout rootview, ItemViewHolder holder, int position) {
            GameInfo gameinfo = m_gamelist.get(position);

            // game run shortcut
            int gamemod = gameinfo.checkValid();
            holder.button_gameicon.setOnClickListener(view -> {
                ArrayList<String> onsargs = prepareOnsargs(gameinfo.getGameDir());
                startOnsyuri(gamemod, gameinfo.getShortPath(), onsargs, gameinfo.m_isUri);
            });

            // game details
            holder.text_gametitle.setOnClickListener(event->{
                // game name, path, icon
                @SuppressLint("InflateParams") View view_gamedetail = LayoutInflater.from(MainActivity.this).inflate(R.layout.layout_gamedetail, null);
                TextView text_gamename = view_gamedetail.findViewById(R.id.text_gamename);
                text_gamename.setText(gameinfo.m_name);
                TextView text_gamepath = view_gamedetail.findViewById(R.id.text_gamepath);
                text_gamepath.setText(gameinfo.m_path);
                ImageView image_gameicon = view_gamedetail.findViewById(R.id.image_gameicon);
                image_gameicon.setImageBitmap(((BitmapDrawable)holder.button_gameicon.getDrawable()).getBitmap());

                // game args, check read, write exec
                EditText text_gameargs = view_gamedetail.findViewById(R.id.text_gameargs);
                ArrayList<String> gameargs = prepareOnsargs(gameinfo.getGameDir());
                String gamecmd = argsToCmd(gameargs);
                text_gameargs.setText(gamecmd);
                try {
                    if(m_gameargs!=null) text_gameargs.setEnabled(m_gameargs.getBoolean("alloweditargs"));
                    else text_gameargs.setEnabled(false);
                }
                catch (JSONException e){
                    text_gameargs.setEnabled(false);
                }

                CheckBox button_checkexec = view_gamedetail.findViewById(R.id.button_checkexec);
                CheckBox button_checkread = view_gamedetail.findViewById(R.id.button_checkread);
                CheckBox button_checkwrite = view_gamedetail.findViewById(R.id.button_checkwrite);
                button_checkread.setChecked((gamemod&4)!=0);
                button_checkwrite.setChecked((gamemod&2)!=0);
                button_checkexec.setChecked((gamemod&1)!=0);

                // game run
                Button button_run = view_gamedetail.findViewById(R.id.button_run);
                button_run.setOnClickListener(view -> {
                    String cmd = text_gameargs.getText().toString();
                    ArrayList<String> args;
                    if(!cmd.equals(gamecmd)) args = cmdToArgs(cmd);
                    else  args = gameargs;
                    startOnsyuri(gamemod, gameinfo.getShortPath(), args, gameinfo.m_isUri);
                });

                // game detail popup
                PopupWindow window_config = new PopupWindow(MainActivity.this);
                window_config.setContentView(view_gamedetail);
                window_config.setWidth(LinearLayout.LayoutParams.MATCH_PARENT);
                window_config.setFocusable(true); // close window when outside click
                window_config.setAnimationStyle(android.R.style.Animation_Toast);
                ImageButton button_closedetail = view_gamedetail.findViewById(R.id.button_closedetail);
                button_closedetail.setOnClickListener(view->window_config.dismiss());
                window_config.showAtLocation(rootview,  Gravity.BOTTOM, 0, 0);

            });
        }

        public void startOnsyuri(int mode, String gamepath, ArrayList<String> args, boolean usesaf) {
            if(mode==7)  {
                MainActivity.this.startOnsyuri(args, usesaf);
                return;
            }
            AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
            builder.setTitle("Invalid Game Dir Warning");
            StringBuilder message = new StringBuilder();
            message.append("Gamepath: ").append(gamepath).append("\n");
            if((mode&4)==0) message.append("can not read !\n");
            if((mode&2)==0) message.append("can not write !\n");
            if((mode&1)==0) message.append("can not find 0.txt or default.ttf !\n");
            message.append("Do you still want to run ? ");
            builder.setMessage(message);
            builder.setCancelable(true);
            builder.setPositiveButton("Yes", (dialog, which) -> {
                dialog.dismiss();
                MainActivity.this.startOnsyuri(args, usesaf);
            });
            builder.setNegativeButton("No", (dialog, which) -> dialog.dismiss());
            AlertDialog dialog = builder.create();
            dialog.show();
        }
    }

    // onsyuri info
    private String[] m_appdirs;

    private String m_gamedir = null; // direct dir without saf
    private ArrayList<GameInfo> m_gamelist;
    private SharedPreferences m_sharedpref;
    private JSONObject m_gameargs;

    // onsyuri ui
    private EditText m_textgamedir;
    private GamelistAdapter m_listgameadaptor;

    public static String argsToCmd(ArrayList<String> args){
        StringBuilder cmd = new StringBuilder();
        for(int i=0;i<args.size();i++) {
            cmd.append(args.get(i)).append(" ");
        }
        return cmd.toString();
    }

    public static ArrayList<String> cmdToArgs(String cmd) {
        ArrayList<String> args = new ArrayList<>();
        for(String arg: cmd.split(" ")){
            if(arg==null || arg.length()==0) continue;
            args.add(arg);
        }
        return args;
    }

    public void test(){
        // storage/emulated/0/Android/data/com.yuri.onscripter/files/game/test
        File file = new File("/storage/emulated/0/Local/Game/ons_game/Noesis 羽化/0.txt");
        try {
            FileInputStream fin = new FileInputStream(file);
            int c = fin.read();
            fin.close();
        }
        catch (FileNotFoundException ignored){

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initValue();
        initUiGameConfig();
        initUiGameDir();
        updateGamelist();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent resultData){
        super.onActivityResult(requestCode, resultCode, resultData);
        if (requestCode == ACTIVITY_SAF ) {
            if(resultData==null) return;
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
    private void initValue() {
        // init valus
        m_gamelist = new ArrayList<>();

        // load from sharedpref
        m_sharedpref = getSharedPreferences(SHAREDPREF_NAME, MODE_PRIVATE);
        m_gamedir = m_sharedpref.getString(SHAREDPREF_GAMEDIR, null);

        // init dirs
        SafFile.init(this, m_sharedpref);
        SafFile.g_sharedprefname = SHAREDPREF_GAMEURI;
        m_appdirs = SafFile.getAppDirectories();
    }

    private DIR_TYPE checkDir(String dirpath){
        if(dirpath==null) return DIR_TYPE.NOT_DIR;
        try {
            File file = new File(dirpath);
            if(file.isDirectory()) { // if input valid dirpath
                return DIR_TYPE.NORMAL_DIR;
            }
            else {
                Uri uri = Uri.parse(dirpath);
                DocumentFile doc = DocumentFile.fromTreeUri(this, uri);
                if(doc!=null && doc.isDirectory()) return DIR_TYPE.SAF_DIR;
                else return DIR_TYPE.NOT_DIR;
            }
        }
        catch (IllegalArgumentException e){
            return  DIR_TYPE.NOT_DIR;
        }
    }

    private ArrayList<String> prepareOnsargs(String gamedir){
        ArrayList<String> onsargs = new ArrayList<>();
        onsargs.add("--root");
        onsargs.add(gamedir);
        onsargs.add("--font");
        onsargs.add(gamedir + "/default.ttf");
        try {
            if(m_gameargs.getBoolean("strechfull")) onsargs.add("--fullscreen2");
            else onsargs.add("--fullscreen");
            if(m_gameargs.getBoolean("disablevideo")) onsargs.add("--no-video");
            if(m_gameargs.getBoolean("usesjis")) onsargs.add("--enc:sjis");
            if(m_gameargs.getBoolean("scopedsavedir")) {
                int idx;
                if(gamedir.charAt(gamedir.length()-1)!='/') idx =gamedir.lastIndexOf('/', gamedir.length()-2);
                else idx =gamedir.lastIndexOf('/');
                String gamename = idx >=0 ? gamedir.substring(idx+1) : gamedir;
                String savedir = getExternalFilesDir(null).toString();
                if(savedir.charAt(savedir.length()-1)!='/') savedir += '/';
                savedir += "save/" + gamename;

                File file = new File(savedir);
                if(file.exists() || file.mkdirs()) {
                    onsargs.add("--save-dir");
                    onsargs.add(savedir);
                }
            }

            if(m_gameargs.getBoolean("sharpness")) {
                String sharpness_value = m_gameargs.getString("sharpness_value");
                onsargs.add("--sharpness");
                onsargs.add(sharpness_value);
            }
        } catch (JSONException e) {
           Log.e("## onsyuri_android", e.toString());
        }
        return onsargs;
    }

    private void startOnsyuri(ArrayList<String> onsargs, boolean usesaf) {
        Intent intent = new Intent();
        intent.setClass(this, ONScripter.class);
        intent.putStringArrayListExtra(SHAREDPREF_GAMECONFIG, onsargs);
        if(usesaf) {
            Uri uri = SafFile.loadDocUri();
            if (uri!=null) intent.putExtra(SHAREDPREF_GAMEURI, uri.toString());
        }
        startActivity(intent);
    }
    // onsyuri android ui function

    private void initUiGameConfig(){
        updateGameConfig(true);
        LinearLayout layout_config = findViewById(R.id.layout_gameconfig);

        // checkbox or editbox
        for(int i=0; i<layout_config.getChildCount(); i++){
            View v = layout_config.getChildAt(i);
            if(v instanceof LinearLayout){
                for (int j=0; j<((LinearLayout)v).getChildCount();j++){
                    View v2 =((LinearLayout)v).getChildAt(j);
                    if(v2 instanceof CheckBox) v2.setOnClickListener(view -> updateGameConfig(false));
                    else if (v2 instanceof EditText) ((EditText)v2).addTextChangedListener(new TextWatcher() {
                        @Override
                        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                        @Override
                        public void onTextChanged(CharSequence s, int start, int before, int count) {}
                        @Override
                        public void afterTextChanged(Editable s) {updateGameConfig(false);}
                    });
                }
            }
            else if(v instanceof CheckBox) { // config checkout
                v.setOnClickListener(view -> updateGameConfig(false));
            }
        }

        // config, check update, about button
        Button button_checkupdate = findViewById(R.id.button_checkupdate);
        button_checkupdate.setOnClickListener(view -> {
            Intent intent = new Intent();
            intent.setAction("android.intent.action.VIEW");
            intent.setData(Uri.parse("https://github.com/YuriSizuku/OnscripterYuri/releases"));
            startActivity(intent);
        });
        Button button_about = findViewById(R.id.button_about);
        button_about.setOnClickListener(view -> {
            AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
            builder.setTitle(getResources().getString(R.string.app_name));
            builder.setMessage(getResources().getString(R.string.app_about));
            builder.setCancelable(true);
            builder.setPositiveButton("Ok", (dialog, which) -> dialog.dismiss());
            AlertDialog dialog = builder.create();
            dialog.show();
        });
        ImageButton button_config = findViewById(R.id.button_config);
        button_config.setOnClickListener(view->{
            int visibility = layout_config.getVisibility();
            if(visibility==View.VISIBLE)  {
                layout_config.setVisibility(View.GONE);
            }
            else {
                layout_config.setAlpha(0);
                layout_config.setVisibility(View.VISIBLE);
                layout_config.animate().alpha(1f).setDuration(200).setListener(null);
            }
        });
    }
    private void initUiGameDir(){
        // gamedir button
        ImageButton button_explore = findViewById(R.id.button_explore);
        button_explore.setOnClickListener(view -> SafFile.requestDocUri(this, ACTIVITY_SAF));

        // game dir
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

        // game list
        ListView listgame = findViewById(R.id.list_game);
        m_listgameadaptor = new GamelistAdapter();
        listgame.setAdapter(m_listgameadaptor);
    }

    private void updateGameConfig(boolean init) {

        CheckBox button_strechfull = findViewById(R.id.button_stretchfull);
        CheckBox button_gles2sharpness =findViewById(R.id.button_gles2sharpness);
        TextView text_gles2sharpness = findViewById(R.id.text_gles2sharpness);
        CheckBox button_disablevideo = findViewById(R.id.button_disablevideo);
        CheckBox button_usesjis = findViewById(R.id.button_usesjis);
        CheckBox button_scopedsavedir = findViewById(R.id.button_scopedsavedir);
        CheckBox button_alloweditargs = findViewById(R.id.button_alloweditargs);
        try {
            if(init) { // config -> view
                String jsonstr = m_sharedpref.getString(SHAREDPREF_GAMECONFIG, null);
                if(jsonstr==null) m_gameargs = new JSONObject();
                else m_gameargs = new JSONObject(jsonstr);
                button_strechfull.setChecked(m_gameargs.getBoolean("strechfull"));
                button_gles2sharpness.setChecked(m_gameargs.getBoolean("sharpness"));
                button_disablevideo.setChecked(m_gameargs.getBoolean("disablevideo"));
                button_usesjis.setChecked(m_gameargs.getBoolean("usesjis"));
                button_scopedsavedir.setChecked(m_gameargs.getBoolean("scopedsavedir"));
                button_alloweditargs.setChecked(m_gameargs.getBoolean("alloweditargs"));
                text_gles2sharpness.setText(m_gameargs.getString("sharpness_value"));
            }
            else { // view -> config
                if(m_gameargs == null) m_gameargs = new JSONObject();
                m_gameargs.put("strechfull", button_strechfull.isChecked());
                m_gameargs.put("sharpness", button_gles2sharpness.isChecked());
                m_gameargs.put("disablevideo", button_disablevideo.isChecked());
                m_gameargs.put("usesjis", button_usesjis.isChecked());
                m_gameargs.put("scopedsavedir", button_scopedsavedir.isChecked());
                m_gameargs.put("alloweditargs", button_alloweditargs.isChecked());
                String sharpness_value = text_gles2sharpness.getText().toString();
                try {
                    Double.valueOf(sharpness_value);

                } catch (NumberFormatException e){
                    sharpness_value = "10.0";
                }
                m_gameargs.put("sharpness_value",  sharpness_value);
                m_sharedpref.edit().putString(SHAREDPREF_GAMECONFIG, m_gameargs.toString()).apply();
            }
        }
        catch (JSONException e){
            Log.e("## onsyuri_android", e.toString());
        }

        // immediate change
        EditText text_gamedir = findViewById(R.id.text_gamedir);
        text_gamedir.setEnabled(button_alloweditargs.isChecked());
    }

    private DIR_TYPE updateGameDir(){
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
        String _gamedir = m_textgamedir.getText().toString();
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
                if(file.getName().equals("save")) continue;
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