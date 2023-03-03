package io.github.hexhacking.xunwind.sample;

public class NativeSample {
    public static void testCfi(boolean remoteUnwind) {
        nativeTestCfi(remoteUnwind);
    }

    public static void testFP(boolean withContext, boolean signalInterrupted) {
        nativeTestFp(withContext, signalInterrupted);
    }

    public static void testEH(boolean withContext, boolean signalInterrupted) {
        nativeTestEh(withContext, signalInterrupted);
    }

    private static native void nativeTestCfi(boolean remoteUnwind);
    private static native void nativeTestFp(boolean withContext, boolean signalInterrupted);
    private static native void nativeTestEh(boolean withContext, boolean signalInterrupted);

}
