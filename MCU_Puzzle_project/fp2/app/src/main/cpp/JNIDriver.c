//
// Created by andykim02 on 2024-11-06.
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include <string.h>

int fd = 0;

JNIEXPORT jint JNICALL
Java_com_example_fp2_JNIDriver_openDriver(JNIEnv *env, jclass class, jstring path) {
    // TODO: implement openDriver()
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd = open(path_utf, O_RDWR);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if (fd < 0) return -1;
    else return 1;
}

JNIEXPORT void JNICALL
Java_com_example_fp2_JNIDriver_closeDriver(JNIEnv *env, jclass class) {
// TODO: implement closeDriver()
if (fd < 0) close(fd);
}

JNIEXPORT jchar JNICALL
Java_com_example_fp2_JNIDriver_readDriver(JNIEnv *env, jobject obj) {
    // TODO: implement readDriver()
    char ch = 0;

    if(fd>0){
        read(fd, &ch, 1);
    }
    return ch;
}

JNIEXPORT jint JNICALL
Java_com_example_fp2_JNIDriver_getInterrupt(JNIEnv *env, jobject obj) {
    // TODO: implement getInterrupt()
    int ret = 0;
    char value[100];
    char* ch1 = "Up";
    char* ch2 = "Down";
    char* ch3 = "Left";
    char* ch4 = "Right";
    char* ch5 = "Center";
    ret = read(fd,&value, 100);

    if(ret<0)
        return -1;
    else{
        if(strcmp(ch1, value) == 0)
            return 1;
        else if(strcmp(ch2, value) == 0)
            return 2;
        else if(strcmp(ch3, value) == 0)
            return 3;
        else if(strcmp(ch4, value) == 0)
            return 4;
        else if(strcmp(ch5, value) == 0)
            return 5;
    }
    return 0;
}



JNIEXPORT void JNICALL
Java_com_example_fp2_JNIDriver_writeDriver(JNIEnv *env, jclass clazz, jbyteArray data,
                                           jint length) {
    // TODO: implement writeDriver()
    jbyte *chars = (*env)->GetByteArrayElements(env, data, 0);
    if (fd > 0) write(fd, (unsigned char *)chars, length);
    (*env)->ReleaseByteArrayElements(env, data, chars, 0);
}

JNIEXPORT void JNICALL
Java_com_example_fp2_PuzzleActivity_writeDriver(JNIEnv *env, jclass clazz, jbyteArray data,
                                                jint length) {
    // TODO: implement writeDriver()
    jbyte *chars = (*env)->GetByteArrayElements(env, data, 0);
    if (fd > 0) write(fd, (unsigned char *)chars, length);
    (*env)->ReleaseByteArrayElements(env, data, chars, 0);
}


JNIEXPORT jint JNICALL
Java_com_example_fp2_PuzzleActivity_openDriver(JNIEnv *env, jclass clazz, jstring path) {
    // TODO: implement openDriver()
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd = open(path_utf, O_RDWR);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if (fd < 0) return -1;
    else return 1;
}