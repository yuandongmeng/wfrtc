#ifndef INCLUDE_YVRTC_INTERFACE_H_
#define INCLUDE_YVRTC_INTERFACE_H_

#include <stdint.h>
#include <iostream>
#include <string>
#include "yvr_comm.h"

#define SEND_KBPS_AUTO_CALCULATE -1

namespace yvrtc
{
enum VideoFrameType
    {
        VideoFrameTypeP = 0,
        VideoFrameTypeI,
        VideoFrameTypeB,
        VideoFrameUnknow
    };

    typedef struct
    {
        int32_t uid;
        VideoFrameType frameType;
        int32_t size;
        int64_t timestamp;
        uint8_t *payload;
        VideoCodecType videoCodecType;
    } YVRFrame;

    typedef void (*YVRDataChannelRecvCallback)(YVRFrame* msgFrame, void* pUser);

    class YVRTCEngine {
    public:
        YVRTCEngine();
        virtual ~YVRTCEngine();

        void setReceiveDataChannel(YVRDataChannelRecvCallback callback,void* pUser);
        void sendDataChannelData(YVRFrame* yvrFrame);

        int32_t putVideoFrame(YVRFrame *pFrame);

    public:
        YVRDataChannelRecvCallback m_callback;
        void* m_pUser;
    };


//**********android ***********//
    class YVPlayEngine {
    public:
        YVPlayEngine();
        virtual ~YVPlayEngine();

        int32_t YVPlayStart(std::string url);
        int32_t YVPlayStop();
        int32_t PollVideoFrame(YVRFrame *pFrame);

        void SetP2PBinaryMessageReceived(YVRDataChannelRecvCallback callback,void* pUser);
        void SendP2PBinaryMessage(const uint8_t* message, int length, bool reliable);

    public:
        YVRDataChannelRecvCallback m_callback;
        void* m_pUser;
    };

//*********************server******************
    class IStreamingRTCEngine;
    class IStreamingRTCEventHandler;

    class IStreamingRTCEngine {
    public:
        static IStreamingRTCEngine* CreateStreamingRTCEngine(const EngineConfig& config,
                                        IStreamingRTCEventHandler* handler, const char* params);

        int64_t SendP2PBinaryMessage(const uint8_t* message, int length, bool reliable);

        int PutVideoFrame(YVRFrame *pFrame);

        virtual int Startup();
        virtual int Shutdown();
        virtual int SetVideoEncoderConfig(yvrtc::StreamIndex stream_index, const VideoSolution* solutions, int solution_num);
        virtual bool PushEncodedVideoFrame(yvrtc::StreamIndex stream_index, int video_index, IEncodedVideoFrame* video_stream);

        virtual void DestroyStreamingRTCEngine(IStreamingRTCEngine* engine){};

    private:
        IStreamingRTCEngine();
        ~IStreamingRTCEngine() {}

        bool Init(IStreamingRTCEventHandler* rtcHandle);
    public:
        IStreamingRTCEventHandler* m_rtcEventHandler;

    };



    class IStreamingRTCEventHandler {
    public:
        virtual void OnP2PBinaryMessageReceived(const uint8_t* message, int size) {
            (void)message;
            (void)size;
        }

        virtual void OnConnectionStateChanged(yvrtc::ConnectionState state) {
                (void)state;
            }

        virtual ~IStreamingRTCEventHandler() {}
    };
}

#endif /* INCLUDE_YVRTC_INTERFACE_H_ */
