package zybornit.loadlib;

import android.util.Log;

/**
 * Created by Administrator on 2016/10/28.
 */

public final class LoadLibrary {
    static {
        System.loadLibrary("Loadlib");
    }

    public MainActivity mainActivity = null;

    LoadLibrary(MainActivity activity) {
        this.mainActivity = activity;
    }

    void displayTextView(int m) {
        Log.e("tt", "dsfff " + m);
    }

    public native String loadlib(String name);

    public native void setJNIEnv();

    public native void releaseJNIEnv();
    //private native void

}
