# xUnwind

![](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/release-2.0.0-red.svg?style=flat)
![](https://img.shields.io/badge/Android-4.1%20--%2013-blue.svg?style=flat)
![](https://img.shields.io/badge/arch-armeabi--v7a%20%7C%20arm64--v8a%20%7C%20x86%20%7C%20x86__64-blue.svg?style=flat)

xUnwind is a collection of Android native stack unwinding solutions.

[README 中文版](README.zh-CN.md)


## Features

* Support unwinding by:
    * CFI (Call Frame Info): Provided by Android system library.
    * EH (Exception handling GCC extension): Provided by compiler.
    * FP (Frame Pointer): ARM64 only.
* Support unwinding from:
    * Current execution position.
    * A specified context (which may be obtained from a signal handler).
* Support unwinding for process:
    * Local process.
    * Remote process: CFI only.
* Support unwinding for thread(s):
    * Current thread.
    * Specified thread: CFI only.
    * All threads: CFI only.
* Provide java method to get native backtrace directly in java code.
* Support Android 4.1 - 13 (API level 16 - 33).
* Support armeabi-v7a, arm64-v8a, x86 and x86_64.
* MIT licensed.


## Usage

### 1. Add dependency in build.gradle

xUnwind is published on [Maven Central](https://search.maven.org/), and uses [Prefab](https://google.github.io/prefab/) package format for [native dependencies](https://developer.android.com/studio/build/native-dependencies), which is supported by [Android Gradle Plugin 4.0+](https://developer.android.com/studio/releases/gradle-plugin?buildsystem=cmake#native-dependencies).

```Gradle
android {
    buildFeatures {
        prefab true
    }
}

dependencies {
    implementation 'io.github.hexhacking:xunwind:2.0.0'
}
```

**NOTE**:

1. Starting from version `2.0.0` of xUnwind, group ID changed from `io.hexhacking` to `io.github.hexhacking`.

| version range  | group ID                 | artifact ID | Repository URL |
|:---------------|:-------------------------|:------------| :--------------|
| [1.0.1, 1.1.1] | io.hexhacking            | xunwind     | [repo](https://repo1.maven.org/maven2/io/hexhacking/xunwind/) |
| [2.0.0, )      | **io.github.hexhacking** | xunwind     | [repo](https://repo1.maven.org/maven2/io/github/hexhacking/xunwind/) |

2. xUnwind uses the [prefab package schema v2](https://github.com/google/prefab/releases/tag/v2.0.0), which is configured by default since [Android Gradle Plugin 7.1.0](https://developer.android.com/studio/releases/gradle-plugin?buildsystem=cmake#7-1-0). If you are using Android Gradle Plugin earlier than 7.1.0, please add the following configuration to `gradle.properties`:

```
android.prefabVersion=2.0.0
```

### 2. Add dependency in CMakeLists.txt or Android.mk

If you only use the java interface of xUnwind, please skip this step.

> CMakeLists.txt

```CMake
find_package(xunwind REQUIRED CONFIG)

add_library(mylib SHARED mylib.c)
target_link_libraries(mylib xunwind::xunwind)
```

> Android.mk

```
include $(CLEAR_VARS)
LOCAL_MODULE           := mylib
LOCAL_SRC_FILES        := mylib.c
LOCAL_SHARED_LIBRARIES += xunwind
include $(BUILD_SHARED_LIBRARY)

$(call import-module,prefab/xunwind)
```

### 3. Specify one or more ABI(s) you need

```Gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
}
```

### 4. Add packaging options

If you are using xUnwind in an SDK project, you may need to avoid packaging libxunwind.so into your AAR, so as not to encounter duplicate libxunwind.so file when packaging the app project.

```Gradle
android {
    packagingOptions {
        exclude '**/libxunwind.so'
    }
}
```

On the other hand, if you are using xUnwind in an APP project, you may need to add some options to deal with conflicts caused by duplicate libxunwind.so file.

```Gradle
android {
    packagingOptions {
        pickFirst '**/libxunwind.so'
    }
}
```

There is a sample app in the [xunwind-sample](xunwind_sample) folder you can refer to.


## Native API

```C
#include "xunwind.h"
```

### 1. CFI unwinding

```C
#define XUNWIND_CURRENT_PROCESS (-1)
#define XUNWIND_CURRENT_THREAD (-1)
#define XUNWIND_ALL_THREADS (-2)

void xunwind_cfi_log(pid_t pid, pid_t tid, void *context, const char *logtag, android_LogPriority priority, const char *prefix);
void xunwind_cfi_dump(pid_t pid, pid_t tid, void *context, int fd, const char *prefix);
char *xunwind_cfi_get(pid_t pid, pid_t tid, void *context, const char *prefix);
```

These three functions correspond to three ways of obtaining backtrace. They are:

* `log` to Android logcat.
* `dump` to a place associated with FD (such as file, pipe, socket, etc.).
* `get` and return a string (which is allocated on the heap with `malloc()`, you need to `free()` it yourself).

The `pid` parameter is used to specify the backtrace of which process needs to be obtained, which can be the current process (`XUNWIND_CURRENT_PROCESS` / `getpid()`) or another process.

The `tid` parameter is used to specify the backtrace of which thread or threads need to be obtained, which can be the current thread (`XUNWIND_CURRENT_THREAD` / `gettid()`), a specified thread, or all threads (`XUNWIND_ALL_THREADS`).

The optional `context` parameter is used to pass a register context information. For example, in signal handler, you may need to start unwinding from a specific context.

The optional `prefix` parameter is used to specify a prefix string for each line of backtrace.

### 2. FP and EH unwinding

```C
size_t xunwind_fp_unwind(uintptr_t* frames, size_t frames_cap, void *context);
size_t xunwind_eh_unwind(uintptr_t* frames, size_t frames_cap, void *context);

void xunwind_frames_log(uintptr_t* frames, size_t frames_sz, const char *logtag, android_LogPriority priority, const char *prefix);
void xunwind_frames_dump(uintptr_t* frames, size_t frames_sz, int fd, const char *prefix);
char *xunwind_frames_get(uintptr_t* frames, size_t frames_sz, const char *prefix);
```

Currently, FP unwinding is ARM64 only.

`xunwind_fp_unwind` and `xunwind_eh_unwind` saves the absolute-PCs of the unwinding result in the array pointed to by `frames` (`frames_cap` is the size of the array), and returns the number of absolute-PCs actually obtained.

The meaning of the optional `context` parameter is the same as that of CFI unwinding.

The remaining three functions are used to convert the absolute-PCs in the `frames` array into backtrace (the size of the array is specified by `frames_sz`). Same as CFI unwinding, respectively output to Android logcat, FD, and return as a string.


## Java API

```Java
import io.github.hexhacking.xunwind.XUnwind;
```

### 1. Initialize

```Java
public static void init();
```

The only thing `init()` does is `System.loadLibrary("xunwind")`. If you only use xUnwind in native code, no initialization is required.

### 2. CFI unwinding

```Java
public static void logLocalCurrentThread(String logtag, int priority, String prefix);
public static void logLocalThread(int tid, String logtag, int priority, String prefix);
public static void logLocalAllThread(String logtag, int priority, String prefix);
public static void logRemoteThread(int pid, int tid, String logtag, int priority, String prefix);
public static void logRemoteAllThread(int pid, String logtag, int priority, String prefix);

public static void dumpLocalCurrentThread(int fd, String prefix);
public static void dumpLocalThread(int tid, int fd, String prefix);
public static void dumpLocalAllThread(int fd, String prefix);
public static void dumpRemoteThread(int pid, int tid, int fd, String prefix);
public static void dumpRemoteAllThread(int pid, int fd, String prefix);

public static String getLocalCurrentThread(String prefix);
public static String getLocalThread(int tid, String prefix);
public static String getLocalAllThread(String prefix);
public static String getRemoteThread(int pid, int tid, String prefix);
public static String getRemoteAllThread(int pid, String prefix);
```

All native CFI unwinding capabilities have corresponding java functions. They call the native CFI unwinding functions through JNI.

FP and EH unwinding do not have corresponding java functions. Because compared to CFI unwinding, their main advantage is faster execution speed (but the backtrace is not as complete as CFI unwinding), so they are always only used in native code. If you are a java programmer, just use the CFI unwinding functions here.


## Support

* [GitHub Issues](https://github.com/hexhacking/xUnwind/issues)
* [GitHub Discussions](https://github.com/hexhacking/xUnwind/discussions)
* [Telegram Public Group](https://t.me/android_native_geeks)


## Contributing

* [Code of Conduct](CODE_OF_CONDUCT.md)
* [Contributing Guide](CONTRIBUTING.md)
* [Reporting Security vulnerabilities](SECURITY.md)


## License

xUnwind is MIT licensed, as found in the [LICENSE](LICENSE) file.


## History

[xCrash 2.x](https://github.com/hexhacking/xCrash/tree/4748d183c1395c54bfb760ec6c454966d52ab73f) contains a set of methods [xcc\_unwind\_*](https://github.com/hexhacking/xCrash/tree/4748d183c1395c54bfb760ec6c454966d52ab73f/src/native/common) to get backtrace, which is used to try to get backtrace directly from the signal handler when the dumper child process fails. Now we have improved and expanded this set of functions so that they can be used in more scenarios.
