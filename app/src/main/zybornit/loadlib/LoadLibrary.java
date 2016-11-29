package zybornit.loadlib;

/**
 * Created by Administrator on 2016/10/28.
 */

public class LoadLibrary {
    static {
        System.loadLibrary("Loadlib");
    }

    public native String loadlib(String name);

}
