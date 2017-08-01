package zybornit.inject;

/**
 * Created by Administrator on 2017/8/1.
 */

public final class LoadInjectLib {
    static {
        System.loadLibrary("inject");
    }

    public static native String injectLib(String pkgName, String libName, String funcName);

    public static native String addHalfHour(String name);

    public static native String decHalfHour(String name);

    public static native String addHour(String name);

    public static native String decHour(String name);
}
