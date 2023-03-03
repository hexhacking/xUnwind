package io.github.hexhacking.xunwind.sample;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import io.github.hexhacking.xunwind.XUnwind;

public class MyService extends Service {

    private final String TAG = "xunwind_tag";

    public MyService() {
    }

    @Override
    public void onCreate() {
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int pid = intent.getIntExtra("pid", -1);
        int tid = intent.getIntExtra("tid", -1);

        if (tid >= 0) {
            XUnwind.logRemoteThread(pid, tid, TAG, Log.INFO);
        } else {
            XUnwind.logRemoteAllThread(pid, TAG, Log.INFO, "  ");
        }
        Log.i(TAG, ">>> finished <<<");

        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
