
#include <iostream>
#include <errno.h>
#include <sys/timeb.h>
#include <yvrtc_interface.h>
#include <windows.h>
#include <pthread.h>
#include <QDebug>


typedef struct stYvrSendInfo
{
    long long timestamp;
}YvrSendInfo;

#define TEST_DATACHANNEL

uint64_t getCurrentTimeMillis()
{
    timeb t;
    ftime(&t); //获取毫秒

    return t.time * 1000 + t.millitm;
}

void *pollingThread(void *arg)
{
    yvrtc::YVPlayEngine* yvplay = (yvrtc::YVPlayEngine*) arg;
    yvplay->YVPlayStart("webrtc://172.18.0.27:1988/live/livestream");

    yvrtc::YVRFrame Frame;

    int32_t ret = 0;

    FILE* fp = fopen("./yvplay_0718.h264", "ab+");

    while (1) {

        ret = yvplay->PollVideoFrame(&Frame);
        if (ret == 0 && Frame.size > 0) {
            fwrite(Frame.payload, Frame.size, 1, fp);
        }

        Sleep(10);
    }
    fclose(fp);
}

void ReceiveData(yvrtc::YVRFrame* msgFrame,void* pUser)
{
    YvrSendInfo* pInfo = (YvrSendInfo*)msgFrame->payload;
    char* pBuffer = new char[ msgFrame->size - sizeof(YvrSendInfo)+1];
    memset(pBuffer, 0, msgFrame->size - sizeof(YvrSendInfo));
    memcpy(pBuffer, msgFrame->payload+sizeof(YvrSendInfo) ,msgFrame->size - sizeof(YvrSendInfo));
    pBuffer[msgFrame->size - sizeof(YvrSendInfo)] = '\0';
    qDebug() << "debug....."<< pBuffer;
    delete []pBuffer;
    pBuffer = NULL;
    return;

}

void Send(yvrtc::YVPlayEngine* rtc)
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
    rtc->SendP2PBinaryMessage(frame.payload,frame.size, true);

    delete []pBuffer;
    pBuffer = NULL;
}

int main(int argc, char *argv[])
{
    yvrtc::YVPlayEngine* yvplay = new yvrtc::YVPlayEngine();
#ifdef TEST_DATACHANNEL
    yvplay->SetP2PBinaryMessageReceived(ReceiveData,NULL);
    //yvplay->YVPlayStart("webrtc://172.18.1.42:1988/live/livestream");
    //Sleep(15000);
    while(1)
    {
        Send(yvplay);
        Sleep(5000);
    }
#else

    pthread_t m_thread;
    pthread_create(&m_thread, 0, pollingThread, yvplay);

    pthread_join(m_thread, NULL);
#endif

    return 0;
}

