#include <jni.h>
#include <string>
#include "android/native_window_jni.h"
#include "android/asset_manager_jni.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "CV_Manager.h"
#include "Socket_client.h"

static CV_Manager app;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    // TODO: implements setSurface()
    app.SetNativeWindow(ANativeWindow_fromSurface(env,surface));
    app.SetUpCamera();
    app.SetUpSocket();
    app.SetUpEncoder();

    std::thread loopthread( &CV_Manager::CameraLoop, &app);
    loopthread.detach();

    //pthread_create(&mDecoderThread, NULL, s_handleOutput, this);
    //std::thread t (&Encoder::handleOutput, Encoder());
    //t.detach();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_releaseCVMain(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseCVMain()
    app.ReleaseMats();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_flipCamera(JNIEnv *env, jobject thiz, jobject surface) {
    app.PauseCamera();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_initNativeCode(JNIEnv *env, jobject thiz, jstring file_path, jobject assetManager) {
    const char *path = env->GetStringUTFChars(file_path, nullptr);
    app.initFilePath(path);

    env->ReleaseStringUTFChars(file_path, path);

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    app.initAssetManager(mgr);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_faceDetectionCV(JNIEnv *env, jobject thiz) {
    app.switchCVFaceDetection();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_mathias_MainActivity_faceDetectionAI(JNIEnv *env, jobject thiz) {
    app.switchAIFaceDetection();
}