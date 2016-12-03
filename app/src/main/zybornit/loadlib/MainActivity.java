package zybornit.loadlib;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class MainActivity extends Activity {
    public static final String FTP_UPLOAD_SUCCESS = "ftp文件上传成功";
    public static final String FTP_UPLOAD_FAIL = "ftp文件上传失败";
    public static final String FTP_UPLOAD_LOADING = "ftp文件正在上传";
    private static final String TAG = "main";
    private static final String APPLIST = "applist.txt";
    public TextView tv = null;
    public Button refresh_btn = null;
    public Handler handler;
    LoadLibrary llb = null;
    private String libso;

    static private void print(String msg) {
        Log.e("main", msg);
    }

    public static String getAppSize() {
        String appsize = "";
        try {
            final Process m_process = Runtime.
                    getRuntime().exec("ls -l /data/app");
            BufferedReader bufferedReader = new BufferedReader(
                    new InputStreamReader(m_process.getInputStream()), 8192);
            String ls_1 = null;
            try {
                while ((ls_1 = bufferedReader.readLine()) != null) {
//                    String size = ls_1.subSequence(ls_1.lastIndexOf("system") +
//                            7, ls_1.indexOf(":") - 14).toString();
                    /*pkgname = ls_1.subSequence(ls_1.indexOf(":") + 4,
                            ls_1.indexOf(".apk") - 2).toString();


                    if (pkgname.equalsIgnoreCase(packagename)) {
                        appsize = size;

                    }*/
                    Log.e("22", "ls_1 = " + ls_1);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    bufferedReader.close();
                    Log.e("23", "close");
                } catch (IOException e) {
                    e.printStackTrace();// TODO: 异常处理
                }
            }
        } catch (java.io.IOException ee) {
            Log.e("24", "RunScript have a IO error :" + ee.getMessage());
            return null;
        } catch (Exception ex) {
            Log.e("25", "RunScript have a error :" + ex.getMessage());
            return null;
        }
        return appsize;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tv = (TextView) findViewById(R.id.display_message);
        tv.setText("");
        tv.setText(tv.getText(), TextView.BufferType.EDITABLE);

        refresh_btn = (Button) findViewById(R.id.refresh);
        refresh_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                tv.setText("");
                test2(libso);
            }
        });
        //test2(this, libso);
        //listApp(this);
        //uploadFile(this, APPLIST);
    }

    public void setMessage(String m) {
        Editable text = (Editable) tv.getText();
        text.append(m + "\n");
        //Log.e("222222222222222","1111111111111111111");
    }

    private void uploadFile(Context mContext, String name) {
        final Context mc = mContext;
        new Thread(new Runnable() {
            @Override
            public void run() {
                FileInputStream fin = null;
                File file = null;
                try {
                    fin = mc.openFileInput(APPLIST);
                    fileop.inputstreamToFile(fin, file);
                } catch (IOException e) {
                    e.printStackTrace();
                    return;
                }
                try {
                    new FTP().uploadSingleFile(file, "/fff", new FTP.UploadProgressListener() {
                        @Override
                        public void onUploadProgress(String currentStep, long uploadSize, File file) {
                            // TODO Auto-generated method stub
                            Log.d(TAG, currentStep);
                            if (currentStep.equals(MainActivity.FTP_UPLOAD_SUCCESS)) {
                                Log.d(TAG, "-----shanchuan--successful");
                            } else if (currentStep.equals(MainActivity.FTP_UPLOAD_LOADING)) {
                                long fize = file.length();
                                float num = (float) uploadSize / (float) fize;
                                int result = (int) (num * 100);
                                Log.d(TAG, "-----shangchuan---" + result + "%");
                            }
                        }
                    });
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void listApp(Context mContext) {
        ArrayList<ResolveInfo> mApps = getAllApp();
        int n = mApps.size();
        int i = 0;
        StringBuilder Sb = new StringBuilder();
        while (i < n) {
            ResolveInfo app = mApps.get(i++);

            String appDir = null;
            try {
                //
                appDir = getPackageManager().getApplicationInfo(
                        app.activityInfo.packageName, 0).sourceDir;
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }
            if (app.activityInfo.packageName.equals("dascom.telecom.vipclub")) {
                ;
                try {
                    Log.e("243", "app is " + fileop.readFileLen(mContext, appDir));
                    fileop.copyFileToPrivate(mContext, appDir);
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
                //break;
            }
            Sb.append(appDir + "\n");
        }
        //write the file
        try {
            fileop.writePrivateFile(mContext, APPLIST, Sb.toString());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private ArrayList<ResolveInfo> getAllApp() {
        // 获取android设备的应用列表
        ArrayList<ResolveInfo> mApps;
        Intent intent = new Intent(Intent.ACTION_MAIN); // 动作匹配
        intent.addCategory(Intent.CATEGORY_LAUNCHER); // 类别匹配
        mApps = (ArrayList<ResolveInfo>) getPackageManager().queryIntentActivities(intent, 0);
        // 排序
        Collections.sort(mApps, new Comparator<ResolveInfo>() {

            @Override
            public int compare(ResolveInfo a, ResolveInfo b) {
                // 排序规则
                PackageManager pm = getPackageManager();
                return String.CASE_INSENSITIVE_ORDER.compare(a.loadLabel(pm)
                        .toString(), b.loadLabel(pm).toString()); // 忽略大小写
            }
        });
        return mApps;
    }

    private void getAppInfo() {
        // 获取手机已安装的所有应用package的信息(其中包括用户自己安装的，还有系统自带的)
        List<PackageInfo> packages = getPackageManager()
                .getInstalledPackages(0);
        for (int i = 0; i < packages.size(); i++) {
            PackageInfo packageInfo = packages.get(i);
            if ((packageInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
                Log.e("44", packageInfo.applicationInfo.loadLabel(
                        getPackageManager()).toString() + " " + packageInfo.packageName + " " + packageInfo.versionName + " " + packageInfo.versionCode + " " + packageInfo.applicationInfo
                        .loadIcon(getPackageManager()));
                Log.e("45", getPackageCodePath());
            }
        }
    }

    protected void attachBaseContext(Context paramContext) {
        super.attachBaseContext(paramContext);
        print("ATTACHBASECONTEXT");
        handler = new DisplayHandler(this);
        String str = paramContext.getFilesDir().getAbsolutePath();
        Context context = paramContext;
        String soName = "libjiagu";
        fileop.copy(context, soName + ".so", str, soName + "_copy.so");

        libso = str + "/" + soName + "_copy.so";

        llb = new LoadLibrary(this);

    }

    void sendMessageToTextView(String arg, int delayMS) {
        //String arg = "ssss";
        Message message = Message.obtain(handler, R.id.display_textview, arg);
        if (delayMS > 0L) {
            handler.sendMessageDelayed(message, delayMS);
        } else {
            handler.sendMessage(message);
        }
    }

    void test2(String s) {

        llb.setJNIEnv();
        Log.e("ddd", "sssssssss:" + llb.loadlib(s));
        llb.releaseJNIEnv();
    }
}
