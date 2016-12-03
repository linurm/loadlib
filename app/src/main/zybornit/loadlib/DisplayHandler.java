package zybornit.loadlib;

import android.os.Handler;
import android.os.Message;

/**
 * Created by Administrator on 2016/12/1.
 */
public final class DisplayHandler extends Handler {
    private final MainActivity mainActivity;


    DisplayHandler(MainActivity mMainActivity){
        this.mainActivity = mMainActivity;
    }
    @Override
    public void handleMessage(Message message) {
        switch (message.what) {
            case R.id.display_textview:
                String m = (String) message.obj;
                mainActivity.setMessage(m);
                break;
        }
    }
}
