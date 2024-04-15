#include <jni.h>
#include <string>
#include "android/native_window_jni.h"
#include "android/asset_manager_jni.h"

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
    //app.SetUpSocketWebRTC();
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