/**
 * Android write file in extsd card by Storage Access Framework(SAF)
 * This warppper can be used in both java and native (by hook)
 *  v0.7, developed by devseed
 */

package com.yuri.onscripter;

import java.io.File;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;
import android.util.Log;
import androidx.documentfile.provider.DocumentFile;
public class SafFile {

    public static String LOGTAG="## yurisaf";
    @SuppressLint("StaticFieldLeak")
    public static Context g_context = null;
    public static SharedPreferences g_sharedpref = null;
    public static String g_sharedprefname ="yurisaf"; //shared preference name
    public static String g_sharedprefkey ="uri";

    /**
     * Use for regist jni file hook with SAF, such as fopen
     * @param hooksoStr, regexp str passing to xhook register 
     * @param soPath, dlopen this path before xhook register
     */
    public native static void nativeHookFile(String hooksoStr, String soPath);

    /**
     * Init java callbacks for jni hook
     */
    public native static void nativeInitSafJavaCallbacks();
    public native static String getcwd();

    /**
     *  This function shound call at first;
     */
    public static void init(final Context context, final SharedPreferences sharedpref) {
        g_context = context;
        g_sharedpref = sharedpref;
    }

    public static void init(final Context context, String sharedPrefName, String sharedPrefKey) {
        if (sharedPrefName != null)  g_sharedprefname = sharedPrefName;
        if (sharedPrefKey != null) g_sharedprefkey = sharedPrefKey;
        if (context != null) {
            final SharedPreferences share = context.getSharedPreferences(g_sharedprefname, Context.MODE_PRIVATE);
            init(context, share);
        }
    }

    /**
     * Use refection to get the application context
     * 
     * @return the Application context
     */
    @SuppressLint("PrivateApi")
    public static Context getApplicationContext() throws Exception {
        Application app;
        app = (Application) Class.forName("android.app.ActivityThread").getMethod("currentApplication").invoke(null,
                (Object[]) null);
        if (app == null)
            app = (Application) Class.forName("android.app.AppGlobals").getMethod("getInitialApplication")
                    .invoke(null, (Object[]) null);
        assert app != null;
        return app.getApplicationContext();
    }

    /**
     *
     * @return paths to all available external SD-Card roots in the system.
     */
    public static String[] getStorageDirectories() {
        return getStorageDirectories(g_context);
    }

    @SuppressLint("ObsoleteSdkInt")
    public static String[] getStorageDirectories(Context context){
        if (context == null) {
            Log.e(LOGTAG, "SafFile.getStorageDirections context is null!");
            return null;
        }

        String[] storageDirectories;
        final String rawSecondaryStoragesStr = System.getenv("SECONDARY_STORAGE");
        // Log.i(LOGTAG, "secondary_storage "+rawSecondaryStoragesStr);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            final List<String> results = new ArrayList<>();
            final File[] externalDirs = context.getExternalFilesDirs(null);
            if (externalDirs != null) {
                for (final File file : externalDirs) {
                    // Log.i(LOGTAG, "getExt...dirs " + file.getPath());
                    final String path = file.getPath().split("/Android")[0];
                    if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP
                            && Environment.isExternalStorageRemovable(file))
                            || rawSecondaryStoragesStr != null && rawSecondaryStoragesStr.contains(path)) {
                        results.add(path);
                    }
                }
            }
            storageDirectories = results.toArray(new String[0]); // toArray
        } else {
            final Set<String> rv = new HashSet<>();

            if (rawSecondaryStoragesStr != null && !TextUtils.isEmpty(rawSecondaryStoragesStr)) {
                final String[] rawSecondaryStorages = rawSecondaryStoragesStr.split(File.pathSeparator);
                Collections.addAll(rv, rawSecondaryStorages);
            }
            storageDirectories = rv.toArray(new String[0]);
        }
        return storageDirectories;
    }

    /**
     *
     * @return paths to all app directories
     */
    public static String[] getAppDirectories(){
        return getAppDirectories(g_context);
    }
    public static String[] getAppDirectories(Context context) {
        if (context == null) {
            Log.e(LOGTAG, "SafFile.getAppDirectories context is null!");
            return null;
        }

        // internal storage
        final List<String> dirs = new ArrayList<>();
        File file = context.getFilesDir();
        if (file!=null) dirs.add(file.toString());

        // external storage, including ext sdcard
        File[] files = context.getExternalFilesDirs(null);
        for(File _file : files){
            if (_file!=null) dirs.add(_file.toString());
        }
        return dirs.toArray(new String[0]);
    }

    // uri functions
    public static void requestDocUri(final Activity activity, final int requestCode) {
        if(activity==null){
            Log.e(LOGTAG, "SafFile.requestDocUri activity is null");
            return;
        }
        final Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        activity.startActivityForResult(intent, requestCode);
    }
    public static void saveDocUri(Uri uri){
        saveDocUri(g_sharedpref, uri);
    }
    public static void saveDocUri(final Intent intent, final int responseCode){
        saveDocUri(g_context, g_sharedpref, intent, responseCode);
    }
    public static void saveDocUri(Context context, SharedPreferences sharedpref, final Intent intent, final int responseCode) {
        if(context==null) {
            Log.e(LOGTAG, "SafFile.saveDocUriFromResult context is null!");
            return;
        }
        if(sharedpref==null){
            Log.e(LOGTAG, "SafFile.saveDocUriFromResult sharedpref is null!");
            return;
        } 
        if(intent==null){
            Log.e(LOGTAG, "SafFile.saveDocUriFromResult intent is null!");
            return;
        }

        Uri uri;
        uri = intent.getData();
        if (uri != null) {
            saveDocUri(sharedpref, uri);
            int takeFlags = intent.getFlags(); // Persist access permissions.
            takeFlags |= Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION;
            context.getContentResolver().takePersistableUriPermission(uri, takeFlags);
        } else {
            Log.e(LOGTAG, "SafFile.saveDocUriFromResult, Get uri failed!");
        }
    }
    @SuppressLint("ApplySharedPref")
    public static void saveDocUri(SharedPreferences sharedpref, Uri uri) {
        if(sharedpref==null){
            Log.e(LOGTAG, "SafFile.saveDocUri share is null!");
            return;
        }
        if(uri==null){
            sharedpref.edit().remove(g_sharedprefkey).commit();
        }
        else{
            sharedpref.edit().putString(g_sharedprefkey, uri.toString()).commit();
        }
    }

    public static Uri loadDocUri(){
        return loadDocUri(g_sharedpref);
    }
    public static Uri loadDocUri(SharedPreferences sharedpref) {
        if(sharedpref==null){
            Log.e(LOGTAG, "SafFile.loadDocUri sharedpref is null!");
            return null;
        }

        final String p = sharedpref.getString(g_sharedprefkey, null);
        Uri uri = null;
        if (p != null)  uri = Uri.parse(p);
        return uri;
    }
    public static void removeDocUri(){
        removeDocUri(g_sharedpref);
    }
    public static void removeDocUri(SharedPreferences sharedpref){
        saveDocUri(sharedpref, null);
    }

    /**
     *
     * @return extsdcard root DocumentFile
     */
    public static DocumentFile getBaseDocumentFile() {
        return getBaseDocumentFile(g_context);
    }
    public static DocumentFile getBaseDocumentFile(Context context){
        if (context==null) {
            Log.e(LOGTAG, "SafFile.getBaseDocumentFile context is null!");
            return null;
        }
        DocumentFile base = null;
        Uri docUri = loadDocUri();
        if (docUri != null) base = DocumentFile.fromTreeUri(context, docUri);
        return base;
    }

    /**
     *
     * @param base, extsdcard DocumentFile
     * @param pathtobase, relative pathtobase to base
     * @return the pathtobase's parent pathtobase DocumentFile
     */
    public static DocumentFile getDirDocumentFile(final DocumentFile base, String pathtobase) {
        DocumentFile target = base;
        if (base == null) {
            Log.e(LOGTAG, "SafFile.getTargetDirDocumentFile base is null!");
            return null;
        }
        if(pathtobase==null) pathtobase="";

        pathtobase = pathtobase.replace("\\", "/");
        final String[] paths = pathtobase.split("/");
        for (int i=0; i < paths.length -1; i++) {
            target = target.findFile(paths[i]);
            if (target == null) break;
        }
        return target;
    }
    
    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static DocumentFile mkdirsSaf(final DocumentFile base, String path) {
        if(base==null){
            Log.e(LOGTAG, "SafFile.mkdirSaf base is null!");
            return null;
        }
        if(path==null) path="";
        
        DocumentFile df2;
        path = path.replace("\\", "/");
        final String[] paths = path.split("/");
        final int end = paths[paths.length - 1].length() > 0 ? paths.length - 1 : paths.length - 2;
        int i;
        for (i = 0; i < end; i++) {
            if (paths[i].equals(base.getName())) {
                i++;
                break;
            }
        }

        //Log.i(LOGTAG, "mkdirsSaf paths[end]="+paths[end]+" , "+paths[i]);
        df2 = base.findFile(paths[i++]);
        if(i > end){ // if create on the sdcard root
            if(df2!=null) return df2;
            else
                return base.createDirectory(paths[end]);
        }

        DocumentFile last;
        last = base;
        for (; i <= end; i++) // find existing dir
        {
            Log.i(LOGTAG, "mkdirSaf last "+last.getName()+" path["+ i +"] "+paths[i]);
            if (df2 == null) {
                i--;
                break;
            }
            last = df2;
            df2 = df2.findFile(paths[i]);
        }
        if (df2 != null)
            return df2;
        df2 = last;
        for (; i <= end; i++) // create dir
        {
            df2 = df2.createDirectory(paths[i]);
            if (df2 == null) {
                Log.e(LOGTAG, "SafFile.mkdirsSaf failed at " + last.getName() + " " + paths[i]);
                break;
            }
        }
        return df2;
    }
    /**
     *
     * @param base, extsdcard DocumentFile
     * @param path, absoluate path
     * @param mode, "r", "rw", "w", "a"
     */
    public static DocumentFile createFileSaf(Context context, final DocumentFile base, String path, String mode) {
        if(base==null){
            Log.e(LOGTAG, "SafFile.createFileSaf base is null!");
            return null;
        }
        if(path==null) path="";

        boolean iswrite = false;
        boolean isappend = false;
        if (mode.indexOf('w')>=0 || mode.indexOf('+')>=0)  iswrite = true;
        if (mode.indexOf('a')>=0) isappend = true;
        path = path.replace("\\", "/");

        if(iswrite || isappend){
            DocumentFile df2 = null;
            final String[] paths = path.split("/");
            final DocumentFile dirdoc = getDirDocumentFile(base, path);
            if (dirdoc != null) {
                String filename = paths[paths.length - 1];
                df2 = dirdoc.findFile(filename);
                if(df2 == null || !df2.exists()) {
                    df2 = dirdoc.createFile("application/octet-stream", filename);
                }
            }
            return df2;
        }
        else { // faster read without DocumentFile.findName
            String uristr = Uri.encode(path);
            if(path.charAt(0)=='/') uristr = base.getUri() + uristr;
            else uristr = base.getUri() + "%2F" + uristr;
            DocumentFile docfile = DocumentFile.fromSingleUri(context, Uri.parse(uristr));
            if(docfile!=null && docfile.canRead()) return docfile;
            return null;
        }
    }
    
    /**
     *  deprecated, because findfile too slow
     */
    public static DocumentFile createFileSaf2(final DocumentFile base, String path, String mode) {
        if(base==null){
            Log.e(LOGTAG, "SafFile.createFileSaf base is null!");
            return null;
        }
        if(path==null) path="";
        
        DocumentFile df2 = null;
        path = path.replace("\\", "/");
        final String[] paths = path.split("/");
        final DocumentFile dirdoc = getDirDocumentFile(base, path);
        if (dirdoc != null) {
            boolean iswrite = false;
            boolean isappend = false;
            if (mode.indexOf('w')>=0 || mode.indexOf('+')>=0)  iswrite = true;
            if (mode.indexOf('a')>=0) isappend = true;
            String filename = paths[paths.length - 1];

            df2 = dirdoc.findFile(filename);
            if(iswrite || isappend) {
                if (df2 != null && df2.exists()) { // find file
                    if (isappend)  return df2;
                    else df2.delete();
                }
                df2 = dirdoc.createFile("application/octet-stream", filename);
            }
        }
        return df2;
    }
    
    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static boolean deleteFileSaf(final DocumentFile base, String path) {
        if(base==null){
            Log.e(LOGTAG, "SafFile.deleteFileSaf base is null!");
            return false;
        }
        if(path==null) path="";

        boolean ret = false;
        DocumentFile df2;
        path = path.replace("\\", "/");
        final String[] paths = path.split("/");
        final int end = paths[paths.length - 1].length() > 0 ? paths.length - 1 : paths.length - 2;
        final DocumentFile target = getDirDocumentFile(base, path);
        if (target != null) {
            df2 = target.findFile(paths[end]);
            if (df2 != null && df2.exists()) ret = df2.delete();
        }
        return ret;
    }

    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */

    @SuppressLint("Recycle")
    public static int getFdSaf(final Context context, final DocumentFile base, final String path, final String mode) {
        if(context==null) {
            Log.e(LOGTAG, "SafFile.getFdSaf context is null!");
            return 0;
        }
        if(base==null){
            Log.e(LOGTAG, "SafFile.getFdSaf base is null!");
            return 0;
        }
        
        ParcelFileDescriptor pfd;
        DocumentFile docfile = createFileSaf(context, base, path, mode);
        if (docfile == null) return -1;
        try {
            pfd = context.getContentResolver().openFileDescriptor(docfile.getUri(), mode);
        } catch (final Exception e) {
            return -1;
        }
        if (pfd == null) return -1;
        return pfd.detachFd();
    }

    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static OutputStream getOutputStreamSaf(final Context context,
              final DocumentFile base, final String path, final boolean append) {
        if(context==null) {
            Log.e(LOGTAG, "SafFile.getOutputStreamSaf context is null!");
            return null;
        }
        if(base==null){
            Log.e(LOGTAG, "SafFile.getOutputStreamSaf base is null!");
            return null;
        }
        
        OutputStream out;
        final String mode = append ? "wa" : "w";
        final DocumentFile df2 = createFileSaf(context, base, path, mode);
        if (df2 == null) {
            return null;
        }
        try {
            out = context.getContentResolver().openOutputStream(df2.getUri(), mode);
        } catch (final Exception e) {
            return null;
        }
        return out;
    }

    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static boolean writeFileSaf(final Context context, final DocumentFile base, final String path,
            final byte[] fileContent, final boolean isappend) {
        if(context==null) {
            Log.e(LOGTAG, "SafFile.writeFileSaf context is null!");
            return false;
        }
        if(base==null){
            Log.e(LOGTAG, "SafFile.writeFileSaf base is null!");
            return false;
        }

        final OutputStream out = getOutputStreamSaf(context, base, path, isappend);
        if (out == null) {
            if(path!=null) 
                Log.e(LOGTAG, "SafFile.writeFileSaf" + path + " error!");
            else 
                Log.e(LOGTAG, "SafFile.writeFileSaf" + " error!");
            return false;
        }
        try {
            out.write(fileContent);
            out.flush();
            out.close();
        } catch (final Exception e) {
            Log.e(LOGTAG, "SafFile.writeFileSaf " + e.getClass().getName());
            return false;
        }
        return true;
     }
    
     /**
      * fix the g_context, and g_share
      */
    public static void fixGValues() {
        if (g_context == null) {
            try {
                final Context context = getApplicationContext();
                if (context != null) {
                    final SharedPreferences share = context.getSharedPreferences(g_sharedprefname, Context.MODE_PRIVATE);
                    init(context, share);
                }
                else{
                    Log.e(LOGTAG, "SafFile.fixGValues() failed, context is null!");
                }
            } catch (final Exception e) {
                Log.e(LOGTAG, "SafFile.checkValues error! " + e.getClass().getName());
            }
        }
        if (g_sharedpref == null) {
            final SharedPreferences share = g_context.getSharedPreferences(g_sharedprefname, Context.MODE_PRIVATE);
            init(g_context, share);
        }
    }
}