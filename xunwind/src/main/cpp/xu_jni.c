// Copyright (c) 2020-2021 HexHacking Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2020-10-21.

#include <android/log.h>
#include <inttypes.h>
#include <jni.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xunwind.h"

#define XU_JNI_VERSION    JNI_VERSION_1_6
#define XU_JNI_CLASS_NAME "io/hexhacking/xunwind/XUnwind"

static void xu_jni_log(JNIEnv *env, jobject thiz, jint pid, jint tid, jstring logtag, jint priority,
                       jstring prefix) {
  (void)env, (void)thiz;

  const char *c_logtag = NULL;
  const char *c_prefix = NULL;
  if (logtag && NULL == (c_logtag = (*env)->GetStringUTFChars(env, logtag, 0))) goto end;
  if (prefix && NULL == (c_prefix = (*env)->GetStringUTFChars(env, prefix, 0))) goto end;

  xunwind_cfi_log((pid_t)pid, (pid_t)tid, NULL, c_logtag, (android_LogPriority)priority, c_prefix);

end:
  if (logtag && c_logtag) (*env)->ReleaseStringUTFChars(env, logtag, c_logtag);
  if (prefix && c_prefix) (*env)->ReleaseStringUTFChars(env, prefix, c_prefix);
}

static void xu_jni_dump(JNIEnv *env, jobject thiz, jint pid, jint tid, jint fd, jstring prefix) {
  (void)env, (void)thiz;

  const char *c_prefix = NULL;
  if (prefix && NULL == (c_prefix = (*env)->GetStringUTFChars(env, prefix, 0))) goto end;

  xunwind_cfi_dump((pid_t)pid, (pid_t)tid, NULL, (int)fd, c_prefix);

end:
  if (prefix && c_prefix) (*env)->ReleaseStringUTFChars(env, prefix, c_prefix);
}

static jstring xu_jni_get(JNIEnv *env, jobject thiz, jint pid, jint tid, jstring prefix) {
  (void)env, (void)thiz;

  jstring str = NULL;

  const char *c_prefix = NULL;
  if (prefix && NULL == (c_prefix = (*env)->GetStringUTFChars(env, prefix, 0))) goto end;

  char *c_str = xunwind_cfi_get((pid_t)pid, (pid_t)tid, NULL, c_prefix);
  if (NULL != c_str) {
    str = (*env)->NewStringUTF(env, c_str);
    free(c_str);
  }

end:
  if (prefix && c_prefix) (*env)->ReleaseStringUTFChars(env, prefix, c_prefix);
  return str;
}

static JNINativeMethod sample_jni_methods[] = {
    {"nativeLog", "(IILjava/lang/String;ILjava/lang/String;)V", (void *)xu_jni_log},
    {"nativeDump", "(IIILjava/lang/String;)V", (void *)xu_jni_dump},
    {"nativeGet", "(IILjava/lang/String;)Ljava/lang/String;", (void *)xu_jni_get}};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  jclass cls;

  (void)reserved;

  if (NULL == vm) return JNI_ERR;
  if (JNI_OK != (*vm)->GetEnv(vm, (void **)&env, XU_JNI_VERSION)) return JNI_ERR;
  if (NULL == env || NULL == *env) return JNI_ERR;
  if (NULL == (cls = (*env)->FindClass(env, XU_JNI_CLASS_NAME))) return JNI_ERR;
  if (0 != (*env)->RegisterNatives(env, cls, sample_jni_methods,
                                   sizeof(sample_jni_methods) / sizeof(sample_jni_methods[0])))
    return JNI_ERR;

  return XU_JNI_VERSION;
}
