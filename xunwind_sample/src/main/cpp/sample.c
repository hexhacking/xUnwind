#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <setjmp.h>
#include <jni.h>
#include <android/log.h>
#include "xunwind.h"

#define SAMPLE_JNI_VERSION    JNI_VERSION_1_6
#define SAMPLE_JNI_CLASS_NAME "io/hexhacking/xunwind/sample/NativeSample"
#define SAMPLE_LOG_TAG        "xunwind_tag"
#define SAMPLE_LOG_PRIORITY   ANDROID_LOG_INFO

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define SAMPLE_LOG(fmt, ...) __android_log_print(SAMPLE_LOG_PRIORITY, SAMPLE_LOG_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop

#define SAMPLE_TEMP_FAILURE_RETRY(exp) ({  \
    __typeof__(exp) _rc;                   \
    do {                                   \
        errno = 0;                         \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })

#define SAMPLE_SOLUTION_CFI 1
#define SAMPLE_SOLUTION_FP  2
#define SAMPLE_SOLUTION_EH  3

// parameters from JNI
static int g_solution = 0;
static bool g_remote_unwind = false;
static bool g_with_context = false;
static bool g_signal_interrupted = false;

// for sigsegv
static struct sigaction g_sigsegv_oldact;
static pid_t g_cur_tid = 0;
static sigjmp_buf g_jb;

static uintptr_t g_frames[128];
static size_t g_frames_sz = 0;

//
// ** Notice **
// It is not reliable to do CFI unwinding directly in a signal handler.
// This is just an example.
//

#pragma clang optimize off
static void sample_make_sigsegv()
{
    *((volatile int *)0) = 0;
}
#pragma clang optimize on

static void sample_sigabrt_handler(int signum, siginfo_t *siginfo, void *context)
{
    (void)signum, (void)siginfo;

    if(g_solution == SAMPLE_SOLUTION_FP || g_solution == SAMPLE_SOLUTION_EH)
    {
        if(!g_signal_interrupted)
        {
            if(g_solution == SAMPLE_SOLUTION_FP)
            {
                // FP local unwind
#ifdef __aarch64__
                size_t frames_sz = xunwind_fp_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]), g_with_context ? context : NULL);
                __atomic_store_n(&g_frames_sz, frames_sz, __ATOMIC_SEQ_CST);
#else
                SAMPLE_LOG("FP unwinding is only supported on arm64.");
#endif
            }
            else if(g_solution == SAMPLE_SOLUTION_EH)
            {
                // EH local unwind
                size_t frames_sz = xunwind_eh_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]), g_with_context ? context : NULL);
                __atomic_store_n(&g_frames_sz, frames_sz, __ATOMIC_SEQ_CST);
            }
        }
        else
        {
            // trigger a segfault, we will do "FP local unwind" in the sigsegv's signal handler
            __atomic_store_n((pid_t *)&g_cur_tid, gettid(), __ATOMIC_SEQ_CST);
            if(0 == sigsetjmp(g_jb, 1))
                sample_make_sigsegv();
        }
    }
    else if(!g_remote_unwind)
    {
        // CFI local unwind
        xunwind_cfi_log(XUNWIND_CURRENT_PROCESS, XUNWIND_CURRENT_THREAD, context,
            SAMPLE_LOG_TAG, SAMPLE_LOG_PRIORITY, NULL);
    }
    else
    {
        // CFI remote unwind
        int notifier[2];
        if(0 != pipe(notifier)) return;

        pid_t pid = getpid();
        pid_t tid = gettid();

        int child_pid = fork();
        if(child_pid < 0) return;

        if(0 == child_pid)
        {
            // child
            write(notifier[1], "a", 1);
            xunwind_cfi_log(pid, tid, context, SAMPLE_LOG_TAG, SAMPLE_LOG_PRIORITY, NULL);
            _exit(0);
        }

        //parent
        close(notifier[1]);
        char buf;
        read(notifier[0], &buf, sizeof(buf));
        close(notifier[0]);

        int status = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-statement-expression"
        if(SAMPLE_TEMP_FAILURE_RETRY(waitpid(child_pid, &status, __WALL)) < 0)
#pragma clang diagnostic pop
        {
            SAMPLE_LOG("waitpid failed, errno: %d", errno);
            return;
        }
        if(!(WIFEXITED(status)) || 0 != WEXITSTATUS(status))
        {
            if(WIFEXITED(status) && 0 != WEXITSTATUS(status))
                SAMPLE_LOG("child terminated normally with non-zero exit status: %d", WEXITSTATUS(status));
            else if(WIFSIGNALED(status))
                SAMPLE_LOG("child terminated by a signal: %d", WTERMSIG(status));
            else
                SAMPLE_LOG("child terminated with other error status: %d", status);
        }
    }
}

static void sample_sigsegv_handler(int signum, siginfo_t *siginfo, void *context)
{
    (void)signum, (void)siginfo;

    pid_t tid = gettid();
    if(__atomic_compare_exchange_n(&g_cur_tid, &tid, 0, true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
    {

        if(g_solution == SAMPLE_SOLUTION_FP)
        {
            // FP local unwind
#ifdef __aarch64__
            size_t frames_sz = xunwind_fp_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]), g_with_context ? context : NULL);
            __atomic_store_n(&g_frames_sz, frames_sz, __ATOMIC_SEQ_CST);
#else
            (void)context;
            SAMPLE_LOG("FP unwinding is only supported on arm64.");
#endif
        }
        else if(g_solution == SAMPLE_SOLUTION_EH)
        {
            // EH local unwind
            size_t frames_sz = xunwind_eh_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]), g_with_context ? context : NULL);
            __atomic_store_n(&g_frames_sz, frames_sz, __ATOMIC_SEQ_CST);
        }
        siglongjmp(g_jb, 1);
    }
    else
    {
        sigaction(SIGSEGV, &g_sigsegv_oldact, NULL);
    }
}

static void sample_signal_register(void)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGSEGV);
    act.sa_sigaction = sample_sigabrt_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGABRT, &act, NULL);

    memset(&act, 0, sizeof(act));
    sigfillset(&act.sa_mask);
    act.sa_sigaction = sample_sigsegv_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &act, &g_sigsegv_oldact);
}

static void sample_test(int solution, jboolean remote_unwind, jboolean with_context, jboolean signal_interrupted)
{
    g_solution = solution;
    g_remote_unwind = (JNI_TRUE == remote_unwind ? true : false);
    g_with_context = (JNI_TRUE == with_context ? true : false);
    g_signal_interrupted = (JNI_TRUE == signal_interrupted ? true : false);

    __atomic_store_n(&g_frames_sz, 0, __ATOMIC_SEQ_CST);

    tgkill(getpid(), gettid(), SIGABRT);

    if((solution == SAMPLE_SOLUTION_FP || solution == SAMPLE_SOLUTION_EH) && __atomic_load_n(&g_frames_sz, __ATOMIC_SEQ_CST) > 0)
        xunwind_frames_log(g_frames, g_frames_sz, SAMPLE_LOG_TAG, SAMPLE_LOG_PRIORITY, NULL);
}

static void sample_test_cfi(JNIEnv *env, jobject thiz, jboolean remote_unwind)
{
    (void)env, (void)thiz;

    sample_test(SAMPLE_SOLUTION_CFI, remote_unwind, JNI_TRUE, JNI_FALSE);
}

static void sample_test_fp(JNIEnv *env, jobject thiz, jboolean with_context, jboolean signal_interrupted)
{
    (void)env, (void)thiz;

    sample_test(SAMPLE_SOLUTION_FP, JNI_FALSE, with_context, signal_interrupted);
}

static void sample_test_eh(JNIEnv *env, jobject thiz, jboolean with_context, jboolean signal_interrupted)
{
    (void)env, (void)thiz;

    sample_test(SAMPLE_SOLUTION_EH, JNI_FALSE, with_context, signal_interrupted);
}

static JNINativeMethod sample_jni_methods[] = {
    {
        "nativeTestCfi",
        "(Z)V",
        (void *)sample_test_cfi
    },
    {
        "nativeTestFp",
        "(ZZ)V",
        (void *)sample_test_fp
    },
    {
        "nativeTestEh",
        "(ZZ)V",
        (void *)sample_test_eh
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    jclass cls;

    (void)reserved;

    if(NULL == vm) return JNI_ERR;
    if(JNI_OK != (*vm)->GetEnv(vm, (void **)&env, SAMPLE_JNI_VERSION)) return JNI_ERR;
    if(NULL == env || NULL == *env) return JNI_ERR;
    if(NULL == (cls = (*env)->FindClass(env, SAMPLE_JNI_CLASS_NAME))) return JNI_ERR;
    if(0 != (*env)->RegisterNatives(env, cls, sample_jni_methods, sizeof(sample_jni_methods) / sizeof(sample_jni_methods[0]))) return JNI_ERR;

    sample_signal_register();

    return SAMPLE_JNI_VERSION;
}
