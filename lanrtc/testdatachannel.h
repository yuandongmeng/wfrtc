#ifndef TESTDATACHANNEL_H
#define TESTDATACHANNEL_H

#include "yvrtc_interface.h"

typedef struct stYvrSendInfo
{
    long long timestamp;
}YvrSendInfo;

class testDatachannel : public yvrtc::IStreamingRTCEventHandler
{
public:
    testDatachannel();

    virtual void OnP2PBinaryMessageReceived(const uint8_t* message, int size) override;

    virtual void OnConnectionStateChanged(yvrtc::ConnectionState state) override;

    static void TheadFun(void* pUser);

    void Send();

    yvrtc::IStreamingRTCEngine* m_engine;
};

#endif // TESTDATACHANNEL_H
