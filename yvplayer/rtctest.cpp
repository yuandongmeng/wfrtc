#include "rtctest.h"
#include <iostream>
#include <errno.h>
#include <sys/timeb.h>
#include <yvrtc_interface.h>
#include <windows.h>
#include <thread>

rtctest::rtctest()
{
    yvrtc::EngineConfig config;
    std::string params ="";
    engine = yvrtc::IStreamingRTCEnginePlayer::CreateStreamingRTCEngine(config,this,params.c_str());
    if (engine)
    {
        engine->YVPlayStart("webrtc://192.168.1.102:1988/live/livestream");
    }
}

void rtctest::OnP2PBinaryMessageReceived(const uint8_t* message, int length)
{

}

bool rtctest::OnRemoteVideoFrame(yvrtc::YVRFrame *pFrame)
{
    static FILE* fp = nullptr;
    if (fp == nullptr)
    {
        fp = fopen("./yvplay_0718.h264", "ab+");
    }

    if (pFrame->size > 0) {
        fwrite(pFrame->payload, pFrame->size, 1, fp);
    }
    return true;
}
