package io.hexhacking.xunwind.sample;

import android.app.Application;
import android.content.Context;

import io.hexhacking.xunwind.XUnwind;

public class MyCustomApplication extends Application {

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);

        XUnwind.init();

        System.loadLibrary("sample");
    }
}
