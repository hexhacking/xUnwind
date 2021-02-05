package io.hexhacking.xunwind.sample;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.os.Process;

import io.hexhacking.xunwind.XUnwind;

public class MainActivity extends AppCompatActivity {

    private final String TAG = "xunwind_tag";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void onTestClick(View view) {
        int pid = Process.myPid();
        int tid = Process.myTid();

        int id = view.getId();

        if (id == R.id.cifLocalThreadButton) {
            Log.i(TAG, String.format(">>> CIF: Local Process (pid: %d), Single Thread (tid: %d) <<<", pid, tid));
            XUnwind.logLocalCurrentThread(TAG, Log.INFO);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.cifLocalThreadWithUContextButton) {
            Log.i(TAG, String.format(">>> CIF: Local Process (pid: %d), Single Thread with UContext (tid: %d) <<<", pid, tid));
            NativeSample.testCfi(false);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.cfiLocalALLButton) {
            Log.i(TAG, String.format(">>> CIF: Local Process (pid: %d), All Threads <<<", pid));
            XUnwind.logLocalAllThread(TAG, Log.INFO, "  ");
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.cifRemoteThreadButton) {
            Log.i(TAG, String.format(">>> CIF: Remote Process (pid: %d), Single Thread (tid: %d) <<<", pid, tid));
            startService(new Intent(this, MyService.class).putExtra("pid", pid).putExtra("tid", tid));
        } else if (id == R.id.cfiRemoteThreadWithUContextButton) {
            Log.i(TAG, String.format(">>> CIF: Remote Process (pid: %d), Single Thread with UContext (tid: %d) <<<", pid, tid));
            NativeSample.testCfi(true);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.cfiRemoteALLButton) {
            Log.i(TAG, String.format(">>> CIF: Remote Process (pid: %d), All Threads <<<", pid));
            startService(new Intent(this, MyService.class).putExtra("pid", pid));
        } else if (id == R.id.fpLocalThreadButton) {
            Log.i(TAG, String.format(">>> FP: Local Process (pid: %d), Current Thread (tid: %d) <<<", pid, pid));
            NativeSample.testFP(false, false);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.fpLocalThreadWithUContextButton) {
            Log.i(TAG, String.format(">>> FP: Local Process (pid: %d), Current Thread with UContext (tid: %d) <<<", pid, pid));
            NativeSample.testFP(true, false);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.fpLocalThreadSignalInterruptedButton) {
            Log.i(TAG, String.format(">>> FP: Local Process (pid: %d), Current Thread (Signal Interrupted) (tid: %d) <<<", pid, pid));
            NativeSample.testFP(false, true);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.fpLocalThreadWithUContextSignalInterruptedButton) {
            Log.i(TAG, String.format(">>> FP: Local Process (pid: %d), Current Thread with UContext (Signal Interrupted) (tid: %d) <<<", pid, pid));
            NativeSample.testFP(true, true);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.ehLocalThreadButton) {
            Log.i(TAG, String.format(">>> EH: Local Process (pid: %d), Current Thread (tid: %d) <<<", pid, pid));
            NativeSample.testEH(false, false);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.ehLocalThreadWithUContextButton) {
            Log.i(TAG, String.format(">>> EH: Local Process (pid: %d), Current Thread with UContext (tid: %d) <<<", pid, pid));
            NativeSample.testEH(true, false);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.ehLocalThreadSignalInterruptedButton) {
            Log.i(TAG, String.format(">>> EH: Local Process (pid: %d), Current Thread (Signal Interrupted) (tid: %d) <<<", pid, pid));
            NativeSample.testEH(false, true);
            Log.i(TAG, ">>> finished <<<");
        } else if (id == R.id.ehLocalThreadWithUContextSignalInterruptedButton) {
            Log.i(TAG, String.format(">>> EH: Local Process (pid: %d), Current Thread with UContext (Signal Interrupted) (tid: %d) <<<", pid, pid));
            NativeSample.testEH(true, true);
            Log.i(TAG, ">>> finished <<<");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopService(new Intent(this, MyService.class));
    }
}
