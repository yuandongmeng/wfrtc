#include <yangp2p/YangP2pFactory.h>
#include <yvrtc_interface.h>
#include <yangplayer/YangPlayerHandle.h>
#include <iostream>
#include <string>
#include <thread>

static std::unique_ptr<YangP2pFactory> g_p2p;
static std::unique_ptr<YangPlayerHandle> g_player;

#define ZRSLEEP(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))

void g_qt_Engine_receiveData(void *context, YangFrame *msgFrame)
{
    yvrtc::IStreamingRTCEngine *engin = (yvrtc::IStreamingRTCEngine *)context;
    yvrtc::YVRFrame yvrFrame = {0};
    yvrFrame.payload = msgFrame->payload;
    yvrFrame.size = msgFrame->nb;
    engin->m_rtcEventHandler->OnP2PBinaryMessageReceived(msgFrame->payload, msgFrame->nb);
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

void yvrtc::YVRTCEngine::setReceiveDataChannel(YVRDataChannelRecvCallback callback,void* pUser)
{
    m_callback = callback;
    m_pUser = pUser;
}

void yvrtc::YVRTCEngine::sendDataChannelData(YVRFrame* yvrFrame)
{
    YangFrame YFrame;
    YFrame.payload = yvrFrame->payload;
    YFrame.nb = yvrFrame->size;
    YFrame.mediaType=YANG_DATA_CHANNEL_STRING;
    if (g_p2p)
    {
        g_p2p->sendDataChannelData(&YFrame);
    }
}

int32_t yvrtc::YVRTCEngine::putVideoFrame(YVRFrame *pFrame)
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

void g_qt_play_receiveData(void* context,YangFrame* msgFrame){
    yvrtc::YVPlayEngine* play=(yvrtc::YVPlayEngine*)context;
    yvrtc::YVRFrame yvrFrame = {0};
    yvrFrame.payload = msgFrame->payload;
    yvrFrame.size = msgFrame->nb;
    play->m_callback(&yvrFrame, play->m_pUser);
}

yvrtc::YVPlayEngine::YVPlayEngine()
{
    if (!g_player)
    {
        g_player.reset(YangPlayerHandle::createPlayerHandle());
        g_player->initDataChannel(g_qt_play_receiveData,this);
    }

}

yvrtc::YVPlayEngine::~YVPlayEngine()
{

}

void yvrtc::YVPlayEngine::SetP2PBinaryMessageReceived(YVRDataChannelRecvCallback callback,void* pUser)
{
    m_callback = callback;
    m_pUser = pUser;
}

void yvrtc::YVPlayEngine::SendP2PBinaryMessage(const uint8_t* message, int length, bool reliable)
{
    YangFrame YFrame;
    YFrame.payload = (uint8_t*)message;
    YFrame.nb = length;
    YFrame.mediaType=YANG_DATA_CHANNEL_STRING;
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

int32_t yvrtc::YVPlayEngine::PollVideoFrame(YVRFrame *pFrame)
{
    int32_t ret = -1;
    if (nullptr != g_player)
    {
        YangFrame Frame = {0};
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


//******************************************************************************************//

yvrtc::IStreamingRTCEngine* yvrtc::IStreamingRTCEngine::CreateStreamingRTCEngine(const yvrtc::EngineConfig& config,
                                        yvrtc::IStreamingRTCEventHandler* handler, const char* params)
{
    yvrtc::IStreamingRTCEngine* obj = new yvrtc::IStreamingRTCEngine();
    if (!obj->Init(handler))
    {
        return nullptr;
    }


    return obj;

}

yvrtc::IStreamingRTCEngine::IStreamingRTCEngine()
{

}

bool yvrtc::IStreamingRTCEngine::Init(IStreamingRTCEventHandler* rtcHandle)
{
    m_rtcEventHandler = rtcHandle;
    if (!g_p2p) {
        g_p2p.reset(new YangP2pFactory());
        g_p2p->init(g_qt_Engine_receiveData, this);
    }

    return true;
}

int64_t yvrtc::IStreamingRTCEngine::SendP2PBinaryMessage(const uint8_t* message, int length, bool reliable)
{
    YangFrame YFrame;
    YFrame.payload = (uint8_t*)message;
    YFrame.nb = length;
    YFrame.mediaType=YANG_DATA_CHANNEL_STRING;
    if (g_p2p)
    {
        g_p2p->sendDataChannelData(&YFrame);
    }

    return 0;
}

int yvrtc::IStreamingRTCEngine::PutVideoFrame(YVRFrame *pFrame)
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

int yvrtc::IStreamingRTCEngine::Startup()
{
    return 0;
}
int yvrtc::IStreamingRTCEngine::Shutdown()
{
    return 0;
}

int yvrtc::IStreamingRTCEngine::SetVideoEncoderConfig(yvrtc::StreamIndex stream_index, const VideoSolution* solutions, int solution_num)
{
    return 0;
}

bool yvrtc::IStreamingRTCEngine::PushEncodedVideoFrame(yvrtc::StreamIndex stream_index, int video_index, IEncodedVideoFrame* video_stream)
{
    return true;
}

//////////



