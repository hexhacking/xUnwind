package io.hexhacking.xunwind;

public class XUnwind {

    private static final int currentProcess = -1;
    private static final int currentThread = -1;
    private static final int allThreads = -2;

    private static boolean initialized = false;

    public interface ILibLoader {
        void loadLibrary(String libName);
    }

    public static synchronized void init() throws SecurityException, UnsatisfiedLinkError {
        init(null);
    }

    public static synchronized void init(XUnwind.ILibLoader libLoader) throws SecurityException, UnsatisfiedLinkError {
        if (initialized) {
            return;
        }

        if (libLoader == null) {
            System.loadLibrary("xunwind");
        } else {
            libLoader.loadLibrary("xunwind");
        }

        initialized = true;
    }

    // dump to logcat
    private static void log(int pid, int tid, String logtag, int priority, String prefix) {
        if (!initialized) {
            return;
        }
        nativeLog(pid, tid, logtag, priority, prefix);
    }

    public static void logLocalCurrentThread(String logtag, int priority, String prefix) {
        log(currentProcess, currentThread, logtag, priority, prefix);
    }

    public static void logLocalCurrentThread(String logtag, int priority) {
        log(currentProcess, currentThread, logtag, priority, null);
    }

    public static void logLocalThread(int tid, String logtag, int priority, String prefix) {
        log(currentProcess, tid, logtag, priority, prefix);
    }

    public static void logLocalThread(int tid, String logtag, int priority) {
        log(currentProcess, tid, logtag, priority, null);
    }

    public static void logLocalAllThread(String logtag, int priority, String prefix) {
        log(currentProcess, allThreads, logtag, priority, prefix);
    }

    public static void logLocalAllThread(String logtag, int priority) {
        log(currentProcess, allThreads, logtag, priority, null);
    }

    public static void logRemoteThread(int pid, int tid, String logtag, int priority, String prefix) {
        log(pid, tid, logtag, priority, prefix);
    }

    public static void logRemoteThread(int pid, int tid, String logtag, int priority) {
        log(pid, tid, logtag, priority, null);
    }

    public static void logRemoteAllThread(int pid, String logtag, int priority, String prefix) {
        log(pid, allThreads, logtag, priority, prefix);
    }

    public static void logRemoteAllThread(int pid, String logtag, int priority) {
        log(pid, allThreads, logtag, priority, null);
    }


    // dump to FD
    private static void dump(int pid, int tid, int fd, String prefix) {
        if (!initialized) {
            return;
        }
        nativeDump(pid, tid, fd, prefix);
    }

    public static void dumpLocalCurrentThread(int fd, String prefix) {
        dump(currentProcess, currentThread, fd, prefix);
    }

    public static void dumpLocalCurrentThread(int fd) {
        dump(currentProcess, currentThread, fd, null);
    }

    public static void dumpLocalThread(int tid, int fd, String prefix) {
        dump(currentProcess, tid, fd, prefix);
    }

    public static void dumpLocalThread(int tid, int fd) {
        dump(currentProcess, tid, fd, null);
    }

    public static void dumpLocalAllThread(int fd, String prefix) {
        dump(currentProcess, allThreads, fd, prefix);
    }

    public static void dumpLocalAllThread(int fd) {
        dump(currentProcess, allThreads, fd, null);
    }

    public static void dumpRemoteThread(int pid, int tid, int fd, String prefix) {
        dump(pid, tid, fd, prefix);
    }

    public static void dumpRemoteThread(int pid, int tid, int fd) {
        dump(pid, tid, fd, null);
    }

    public static void dumpRemoteAllThread(int pid, int fd, String prefix) {
        dump(pid, allThreads, fd, prefix);
    }

    public static void dumpRemoteAllThread(int pid, int fd) {
        dump(pid, allThreads, fd, null);
    }


    // return as string
    private static String get(int pid, int tid, String prefix) {
        if (!initialized) {
            return null;
        }
        return nativeGet(pid, tid, prefix);
    }

    public static String getLocalCurrentThread(String prefix) {
        return get(currentProcess, currentThread, prefix);
    }

    public static String getLocalCurrentThread() {
        return get(currentProcess, currentThread, null);
    }

    public static String getLocalThread(int tid, String prefix) {
        return get(currentProcess, tid, prefix);
    }

    public static String getLocalThread(int tid) {
        return get(currentProcess, tid, null);
    }

    public static String getLocalAllThread(String prefix) {
        return get(currentProcess, allThreads, prefix);
    }

    public static String getLocalAllThread() {
        return get(currentProcess, allThreads, null);
    }

    public static String getRemoteThread(int pid, int tid, String prefix) {
        return get(pid, tid, prefix);
    }

    public static String getRemoteThread(int pid, int tid) {
        return get(pid, tid, null);
    }

    public static String getRemoteAllThread(int pid, String prefix) {
        return get(pid, allThreads, prefix);
    }

    public static String getRemoteAllThread(int pid) {
        return get(pid, allThreads, null);
    }


    private static native void nativeLog(int pid, int tid, String logtag, int priority, String prefix);

    private static native void nativeDump(int pid, int tid, int fd, String prefix);

    private static native String nativeGet(int pid, int tid, String prefix);
}
