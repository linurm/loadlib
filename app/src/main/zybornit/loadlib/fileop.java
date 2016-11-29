package zybornit.loadlib;

import android.content.Context;
import android.util.Log;

import org.apache.http.util.EncodingUtils;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import static android.content.ContentValues.TAG;
import static android.content.Context.MODE_PRIVATE;

/**
 * Created by Administrator on 2016/11/15.
 */

public class fileop {
    public static void writePrivateFile(Context mContext, String fileName, String writestr) throws IOException {
        try {

            FileOutputStream fout = mContext.openFileOutput(fileName, MODE_PRIVATE);

            byte[] bytes = writestr.getBytes();

            fout.write(bytes);

            fout.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static String readPrivateFile(Context mContext, String fileName) throws IOException {
        String res = "";
        try {
            FileInputStream fin = mContext.openFileInput(fileName);
            int length = fin.available();
            byte[] buffer = new byte[length];
            fin.read(buffer);
            res = EncodingUtils.getString(buffer, "UTF-8");
            fin.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return res;
    }

    public static int readFileLen(Context mContext, String pathfileName) throws IOException {
        File file = new File(pathfileName);

        FileInputStream fis = new FileInputStream(file);

        int length = fis.available();
        fis.close();
        return length;
    }

    public static void copyFileToPrivate(Context mContext, String pathfileName) throws IOException {
        InputStream ipstr = new FileInputStream(new File(pathfileName));
        int index = pathfileName.split("/").length - 1;
        String name = pathfileName.split("/")[index];
        String str = mContext.getFilesDir().getAbsolutePath();
        String dest_name = str + "/" + name;

        InputStream localObject = null;

        Log.e(TAG, "copy " + name + " to " + dest_name);
        try {
            localObject = new FileInputStream(dest_name);
            BufferedInputStream localBufferedInputStream1 = new BufferedInputStream(ipstr);
            BufferedInputStream localBufferedInputStream2 = new BufferedInputStream((InputStream) localObject);
            if (isSameFile(localBufferedInputStream1, localBufferedInputStream2)) {
                ipstr.close();
                ((InputStream) localObject).close();
                localBufferedInputStream1.close();
                localBufferedInputStream2.close();
                Log.e(TAG, "there is the Same file");
                return;
            }
        } catch (FileNotFoundException e) {

        }


        OutputStream opstr = new FileOutputStream(dest_name);
        Log.e(TAG, "copy " + pathfileName + " to " + dest_name);
        byte[] abs = new byte['ᰀ'];
        for (; ; ) {
            int i = ipstr.read(abs);
            if (i <= 0) {
                break;
            }
            opstr.write(abs, 0, i);
        }
        Log.e(TAG, "copy apk to sdcard ");
        opstr.close();

        return;
    }

    public static void writeSDFile(String fileName, String write_str) throws IOException {

        File file = new File(fileName);

        FileOutputStream fos = new FileOutputStream(file);

        byte[] bytes = write_str.getBytes();

        fos.write(bytes);

        fos.close();
    }

    public static boolean isSameFile(BufferedInputStream paramBufferedInputStream1, BufferedInputStream paramBufferedInputStream2) {
        try {
            int j = paramBufferedInputStream1.available();
            int i = paramBufferedInputStream2.available();
            byte[] arrayOfByte1 = null;
            byte[] arrayOfByte2 = null;
            if (j == i) {
                arrayOfByte1 = new byte[j];
                arrayOfByte2 = new byte[i];
                paramBufferedInputStream1.read(arrayOfByte1);
                paramBufferedInputStream2.read(arrayOfByte2);
                i = 0;
            }
            while (i < j) {
                int k = arrayOfByte1[i];
                int m = arrayOfByte2[i];
                if (k != m) {
                    return false;
                }
                i += 1;
            }
            return true;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    public static boolean copy(Context paramContext, String paramString1, String paramString2, String paramString3) {
        String paramString4 = paramString2 + "/" + paramString3;
        InputStream strs;
        InputStream isstr;
        File paramS = new File(paramString2);
        if (!paramS.exists()) {
            paramS.mkdir();
        }
        try {
            Object localObject = new File(paramString4);
            if (((File) localObject).exists()) {
                strs = paramContext.getResources().getAssets().open(paramString1);
                localObject = new FileInputStream((File) localObject);
                BufferedInputStream localBufferedInputStream1 = new BufferedInputStream(strs);
                BufferedInputStream localBufferedInputStream2 = new BufferedInputStream((InputStream) localObject);
                if (isSameFile(localBufferedInputStream1, localBufferedInputStream2)) {
                    strs.close();
                    ((InputStream) localObject).close();
                    localBufferedInputStream1.close();
                    localBufferedInputStream2.close();
                    Log.e(TAG, "is the Same file");
                    return true;
                }
                strs.close();
            }
            isstr = paramContext.getResources().getAssets().open(paramString1);
            OutputStream opstr = new FileOutputStream(paramString4);
            byte[] abs = new byte['ᰀ'];
            for (; ; ) {
                int i = isstr.read(abs);
                if (i <= 0) {
                    break;
                }
                opstr.write(abs, 0, i);
            }
            Log.e(TAG, "copy so ");
            opstr.close();
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        //isstr.close();
        return true;
    }

    public static boolean inputstreamToFile(InputStream ins, File file) {
        OutputStream os = null;
        try {
            os = new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        }
        int bytesRead = 0;
        byte[] buffer = new byte[8192];
        try {
            while ((bytesRead = ins.read(buffer, 0, 8192)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
            os.close();
            ins.close();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

}
