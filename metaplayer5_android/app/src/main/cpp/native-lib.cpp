#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include "yvrtc_interface.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static yvrtc::YVPlayEngine* g_player;

extern "C" JNIEXPORT void JNICALL
Java_com_metartc_player_YangAndroidPlay_setSurface(JNIEnv *env, jobject, jobject surface)
{
    // TODO: implement setSurface()
    pthread_mutex_lock(&mutex);

    if (g_player == NULL)
        g_player = new yvrtc::YVPlayEngine();

    pthread_mutex_unlock(&mutex);
}

extern "C" JNIEXPORT int JNICALL
Java_com_metartc_player_YangAndroidPlay_startPlayer(JNIEnv *env, jobject, jstring purl)
{
    // TODO: implement startPlayer()

    int ret = -1;
    const char *url = env->GetStringUTFChars(purl, nullptr);

    ret = g_player->YVPlayStart(url);
    env->ReleaseStringUTFChars(purl, url);

    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_com_metartc_player_YangAndroidPlay_stopPlayer(JNIEnv *env, jobject)
{
    // TODO: implement stopPlayer()
    if (nullptr != g_player)
    {
        g_player->YVPlayStop();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_metartc_player_YangAndroidPlay_releaseResources(JNIEnv *env, jobject)
{
    // TODO: implement releaseResources()
    pthread_mutex_lock(&mutex);

    if (nullptr != g_player)
    {
        delete g_player;
        g_player = nullptr;
    }

    pthread_mutex_unlock(&mutex);
}
