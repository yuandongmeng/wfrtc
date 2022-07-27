#include "testdatachannel.h"
#include <string>
#include <thread>
#include <QDebug>
#define ZRSLEEP(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))

testDatachannel::testDatachannel()
{
    yvrtc::EngineConfig engine;
    std::string para = "";
    m_engine = yvrtc::IStreamingRTCEngine::CreateStreamingRTCEngine(engine,this,para.c_str());
    if (m_engine)
    {
        std::thread _thead(TheadFun, this);
        _thead.detach();
    }
}

void testDatachannel::TheadFun(void* pUser)
{
    testDatachannel* pThis = (testDatachannel*)pUser;
    ZRSLEEP(15000);
    while(1)
    {
        ZRSLEEP(5000);
        //pThis->Send();
    }
}

void testDatachannel::OnP2PBinaryMessageReceived(const uint8_t* message, int size)
{
    YvrSendInfo* pInfo = (YvrSendInfo*)message;
    char* pBuffer = new char[ size - sizeof(YvrSendInfo)+1];
    memset(pBuffer, 0, size - sizeof(YvrSendInfo));
    memcpy(pBuffer, message+sizeof(YvrSendInfo) ,size - sizeof(YvrSendInfo));
    pBuffer[size - sizeof(YvrSendInfo)] = '\0';
    qDebug() << "debug....."<< pBuffer;
    delete []pBuffer;
    pBuffer = NULL;
}

void testDatachannel::OnConnectionStateChanged(yvrtc::ConnectionState state)
{

}

void testDatachannel::Send()
{
    YvrSendInfo sendInfo = { 0 };
    sendInfo.timestamp = 2222;

    std::string cstr = "11111111111";
    uint8_t* pContent = new uint8_t[cstr.size()];
    memset(pContent, 0, cstr.size());
    memcpy(pContent, cstr.c_str(), cstr.size());

    int total = cstr.size() + sizeof(YvrSendInfo);

    uint8_t* pBuffer = new uint8_t[total];
    uint8_t* pTemp = pBuffer;
    memset(pBuffer, 0, total);

    memcpy(pTemp, &sendInfo, sizeof(YvrSendInfo));
    pTemp += sizeof(YvrSendInfo);
    memcpy(pTemp, pContent, cstr.size());

    yvrtc::YVRFrame frame;
    memset(&frame,0,sizeof(yvrtc::YVRFrame));
    frame.payload = pBuffer;
    frame.size = total;
    m_engine->SendP2PBinaryMessage(frame.payload,frame.size, true);
    delete []pBuffer;
}

