
#include "YVRTCEngine.h"
#include <yangp2p/YangP2pFactory.h>
#include <yangplayer/YangPlayerHandle.h>
#include <iostream>

#include <thread>

#pragma comment(lib, "ws2_32.lib")//
#pragma comment(lib, "yuv.lib")
#pragma comment(lib, "opus.lib")
#pragma comment(lib, "openh264.lib")
#pragma comment(lib, "speexdsp.lib")
#pragma comment(lib, "usrsctp.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "ksuser.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "srtp2.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "YVRTCCore.lib")
#pragma comment(lib, "RTCEngine.lib")

static std::unique_ptr<YangP2pFactory> g_p2p;
static std::unique_ptr<YangPlayerHandle> g_player;



void g_qt_Engine_receiveData(void* context, YangFrame* msgFrame)
{
    yvrtc::YVRTCEngine* engin = (yvrtc::YVRTCEngine*)context;
    yvrtc::YVRFrame yvrFrame = { 0 };
    yvrFrame.payload = msgFrame->payload;
    yvrFrame.size = msgFrame->nb;
    engin->m_callback(&yvrFrame, engin->m_pUser);
}


yvrtc::YVRTCEngine::YVRTCEngine()
{
    if (!g_p2p) {
        g_p2p.reset(new YangP2pFactory());
        g_p2p->init(g_qt_Engine_receiveData, this);
    }
}

yvrtc::YVRTCEngine::~YVRTCEngine()
{
    
}

void yvrtc::YVRTCEngine::SetP2PBinaryMessageReceived(YVRDataChannelRecvCallback callback, void* pUser)
{
    m_callback = callback;
    m_pUser = pUser;
}

void yvrtc::YVRTCEngine::SendP2PBinaryMessage(yvrtc::YVRFrame* yvrFrame)
{
    YangFrame YFrame;
    YFrame.payload = yvrFrame->payload;
    YFrame.nb = yvrFrame->size;
    YFrame.mediaType = YANG_DATA_CHANNEL_STRING;
    if (g_p2p)
    {
        g_p2p->sendDataChannelData(&YFrame);
    }
}

int32_t yvrtc::YVRTCEngine::putVideoFrame(yvrtc::YVRFrame* pFrame)
{
    int32_t ret = 0;
    YangFrame YFrame;
    YFrame.payload = pFrame->payload;
    YFrame.nb = pFrame->size;
    YFrame.mediaType = 9;
    YFrame.dts = pFrame->timestamp;
    YFrame.pts = pFrame->timestamp;
    YFrame.uid = pFrame->uid;
    YFrame.frametype = pFrame->frameType;
    if (g_p2p)
    {
        ret = g_p2p->putTxVideo(&YFrame);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

void g_qt_play_receiveData(void* context, YangFrame* msgFrame) {
    yvrtc::YVPlayEngine* play = (yvrtc::YVPlayEngine*)context;
    yvrtc::YVRFrame yvrFrame = { 0 };
    yvrFrame.payload = msgFrame->payload;
    yvrFrame.size = msgFrame->nb;
    play->m_callback(&yvrFrame, play->m_pUser);
}

yvrtc::YVPlayEngine::YVPlayEngine()
{
    if (!g_player)
    {
        g_player.reset(YangPlayerHandle::createPlayerHandle());
        g_player->initDataChannel(g_qt_play_receiveData, this);
    }

}

yvrtc::YVPlayEngine::~YVPlayEngine()
{

}

void yvrtc::YVPlayEngine::SetP2PBinaryMessageReceived(yvrtc::YVRDataChannelRecvCallback callback, void* pUser)
{
    m_callback = callback;
    m_pUser = pUser;
}

void yvrtc::YVPlayEngine::SendP2PBinaryMessage(yvrtc::YVRFrame* yvrFrame)
{
    YangFrame YFrame;
    YFrame.payload = yvrFrame->payload;
    YFrame.nb = yvrFrame->size;
    YFrame.mediaType = YANG_DATA_CHANNEL_STRING;
    if (g_player)
    {
        g_player->dataChannelSend(&YFrame);
    }
}

int32_t yvrtc::YVPlayEngine::YVPlayStart(std::string url)
{
    int ret = -1;
    if (g_player->play((char*)url.c_str()) == Yang_Ok)
    {
        ret = 0;
    }
    return ret;
}

int32_t yvrtc::YVPlayEngine::YVPlayStop()
{
    if (nullptr != g_player)
    {
        g_player->stopPlay();
    }
    return 0;
}

int32_t yvrtc::YVPlayEngine::PollVideoFrame(YVRFrame* pFrame)
{
    int32_t ret = -1;
    if (nullptr != g_player)
    {
        YangFrame Frame = { 0 };
        ret = g_player->getVideoFrame(&Frame);
        if (ret == 0) {
            pFrame->payload = Frame.payload;
            pFrame->frameType = (VideoFrameType)Frame.frametype;
            pFrame->size = Frame.nb;
            pFrame->timestamp = Frame.dts;
            pFrame->uid = Frame.uid;
        }
        else {
            pFrame->payload = nullptr;
            pFrame->frameType = VideoFrameUnknow;
            pFrame->size = 0;
            pFrame->timestamp = 0;
            pFrame->uid = 0;
        }
    }
    return ret;
}

