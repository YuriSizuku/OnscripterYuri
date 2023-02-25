/**
 * Android write file in extsd card by Storage Access Framework(SAF)
 * This warppper can be used in both java and native (by hook)
 *  v0.7, developed by devseed
 */

package com.yuri.onscripter;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
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

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;
public class SafFile {

    public static String LOGTAG="## yurisaf";
    @SuppressLint("StaticFieldLeak")
    public static Context g_context = null;
    public static SharedPreferences g_sharedpref = null;
    public static String g_sharedprefname ="yurisaf"; //shared preference name
    public static String g_sharedprefkey ="uri";

    /**
     * Use for register jni file hook with SAF, such as fopen
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
    public static void init(@NonNull Context context, @NonNull SharedPreferences sharedpref) {
        g_context = context;
        g_sharedpref = sharedpref;
    }

    public static void init(@NonNull Context context, @NonNull String sharedPrefName, @NonNull String sharedPrefKey) {
        g_sharedprefname = sharedPrefName;
        g_sharedprefkey = sharedPrefKey;
        final SharedPreferences share = context.getSharedPreferences(g_sharedprefname, Context.MODE_PRIVATE);
        init(context, share);
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
    public static String[] getStorageDirectories(@NonNull Context context){
        String[] storageDirectories;
        final String rawSecondaryStoragesStr = System.getenv("SECONDARY_STORAGE");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            final List<String> results = new ArrayList<>();
            final File[] externalDirs = context.getExternalFilesDirs(null);
            if (externalDirs != null) {
                for (final File file : externalDirs) {
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
    public static String[] getAppDirectories(@NonNull Context context) {
        // internal storage
        final List<String> dirs = new ArrayList<>();
        File file = context.getFilesDir();
        if (file!=null) dirs.add(file.toString());

        // external storage, including ext sdcard
        File[] files = context.getExternalFilesDirs(null);
        for(File _file : Objects.requireNonNull(files)){
            dirs.add(_file.toString());
        }
        return dirs.toArray(new String[0]);
    }

    public static String uri2Path(@NonNull Uri uri){
        // uri format
        return "";
    }

    // uri functions
    public static void requestDocUri(@NonNull Activity activity, int requestCode) {
        final Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        activity.startActivityForResult(intent, requestCode);
    }
    public static void saveDocUri(Uri uri){
        saveDocUri(g_sharedpref, g_sharedprefkey, uri);
    }
    public static void saveDocUri(@NonNull Intent intent, final int responseCode){
        saveDocUri(g_context, g_sharedpref, g_sharedprefkey, intent, responseCode);
    }
    @SuppressLint("WrongConstant")
    public static void saveDocUri(@NonNull Context context, @NonNull SharedPreferences sharedpref,
            @NonNull String sharedprefkey, @NonNull Intent intent, int responseCode) {
        Uri uri;
        uri = intent.getData();
        if (uri != null) {
            final int takeFlags = intent.getFlags()
                    & (Intent.FLAG_GRANT_READ_URI_PERMISSION
                    | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            context.getContentResolver().takePersistableUriPermission(uri, takeFlags);
            saveDocUri(sharedpref, sharedprefkey, uri);
        }
        else Log.e(LOGTAG, "SafFile.saveDocUriFromResult, Get uri failed!");
    }
    @SuppressLint("ApplySharedPref")
    public static void saveDocUri(@NonNull SharedPreferences sharedpref, @NonNull String sharedprefkey, Uri uri) {
        if(uri==null){
            sharedpref.edit().remove(sharedprefkey).commit();
        }
        else{
            sharedpref.edit().putString(sharedprefkey, uri.toString()).commit();
        }
    }

    public static Uri loadDocUri(){
        return loadDocUri(g_sharedpref, g_sharedprefkey);
    }
    public static Uri loadDocUri(@NonNull SharedPreferences sharedpref, @NonNull String sharedprefkey) {
        final String p = sharedpref.getString(sharedprefkey, null);
        Uri uri = null;
        if (p != null)  uri = Uri.parse(p);
        return uri;
    }
    public static void removeDocUri(){
        removeDocUri(g_sharedpref, g_sharedprefkey);
    }
    public static void removeDocUri(@NonNull SharedPreferences sharedpref, @NonNull String sharedprefkey){
        saveDocUri(sharedpref, sharedprefkey, null);
    }

    /**
     *
     * @return extsdcard root DocumentFile
     */
    public static DocumentFile getBaseDocumentFile() {
        return getBaseDocumentFile(g_context);
    }
    public static DocumentFile getBaseDocumentFile(@NonNull Context context){
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
    public static DocumentFile getDirDocumentFile(@NonNull DocumentFile base, String pathtobase) {
        DocumentFile target = base;
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
    public static DocumentFile mkdirsSaf(@NonNull DocumentFile base, String path) {
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
    public static DocumentFile createFileSaf(@NonNull Context context, @NonNull DocumentFile base, String path, String mode) {
        if(path==null) path="";
        if(mode==null) mode="r";

        boolean iswrite = false;
        boolean isappend = false;
        if (mode.indexOf('w')>=0 || mode.indexOf('+')>=0)  iswrite = true;
        if (mode.indexOf('a')>=0) isappend = true;
        path = path.replace("\\", "/");

        if(iswrite || isappend){
            DocumentFile docfile = null;
            final String[] paths = path.split("/");
            final DocumentFile dirdoc = getDirDocumentFile(base, path);
            if (dirdoc != null) {
                String filename = paths[paths.length - 1];
                docfile = dirdoc.findFile(filename);
                if(docfile == null || !docfile.exists()) {
                    docfile = dirdoc.createFile("application/octet-stream", filename);
                }
            }
            return docfile;
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
    public static DocumentFile createFileSaf2(@NonNull DocumentFile base, String path, String mode) {
        if(path==null) path="";
        if(mode==null) mode="r";
        
        DocumentFile docfile = null;
        path = path.replace("\\", "/");
        final String[] paths = path.split("/");
        final DocumentFile dirdoc = getDirDocumentFile(base, path);
        if (dirdoc != null) {
            boolean iswrite = false;
            boolean isappend = false;
            if (mode.indexOf('w')>=0 || mode.indexOf('+')>=0)  iswrite = true;
            if (mode.indexOf('a')>=0) isappend = true;
            String filename = paths[paths.length - 1];

            docfile = dirdoc.findFile(filename);
            if(iswrite || isappend) {
                if (docfile != null && docfile.exists()) { // find file
                    if (isappend)  return docfile;
                    else docfile.delete();
                }
                docfile = dirdoc.createFile("application/octet-stream", filename);
            }
        }
        return docfile;
    }
    
    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static boolean deleteFileSaf(@NonNull DocumentFile base, String path) {
        if(path==null) path="";

        boolean ret = false;
        DocumentFile docfile;
        path = path.replace("\\", "/");
        final String[] paths = path.split("/");
        final int end = paths[paths.length - 1].length() > 0 ? paths.length - 1 : paths.length - 2;
        final DocumentFile target = getDirDocumentFile(base, path);
        if (target != null) {
            docfile = target.findFile(paths[end]);
            if (docfile != null && docfile.exists()) ret = docfile.delete();
        }
        return ret;
    }

    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */

    @SuppressLint("Recycle")
    public static int getFdSaf(@NonNull Context context, @NonNull DocumentFile base, final String path, final String mode) {
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
    public static OutputStream getOutputStreamSaf(@NonNull Context context,
             @NonNull DocumentFile base, String path, boolean append) {
        OutputStream out;
        final String mode = append ? "wa" : "w";
        final DocumentFile docfile = createFileSaf(context, base, path, mode);
        if (docfile == null) return null;
        try {
            out = context.getContentResolver().openOutputStream(docfile.getUri(), mode);
        } catch (final FileNotFoundException e) {
            return null;
        }
        return out;
    }

    /**
     * 
     * @param base, extsdcard DocumentFile
     * @param path, absolute path
     */
    public static boolean writeFileSaf(@NonNull Context context, @NonNull DocumentFile base,
            String path, byte[] fileContent, boolean isappend) {
        try {
            OutputStream out = getOutputStreamSaf(context, base, path, isappend);
            assert out != null;
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