#ifndef RTCTEST_H
#define RTCTEST_H

#include <yvrtc_interface.h>

class rtctest :public yvrtc::IStreamingRTCEventHandler
{
public:
    rtctest();

    virtual void OnP2PBinaryMessageReceived(const uint8_t* message, int length) override;//
    virtual bool OnRemoteVideoFrame(yvrtc::YVRFrame *pFrame) override;

private:
    static void senderThread(void *arg);

public:
    yvrtc::IStreamingRTCEngine* engine;
};

#endif // RTCTEST_H
