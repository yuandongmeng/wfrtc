#ifndef YVRTCENGINE_H
#define YVRTCENGINE_H

#include <string>

//宏定义导出
#ifdef YVRTCENGINE_H
#define YVRTCENGINE _declspec(dllexport) //如果没有定义YVRTCENGINE,就定义 YVRTCENGINE _declspec(dllexport)
#else
#define YVRTCENGINE _declspec(dllimport)
#endif // YVRTCENGINE

namespace yvrtc
{
    enum VideoFrameType
    {
        VideoFrameTypeP = 0,
        VideoFrameTypeI,
        VideoFrameTypeB,
        VideoFrameUnknow
    };

    typedef struct stYVRFrame
    {
        int32_t uid;
        VideoFrameType frameType;
        int32_t size;
        int64_t timestamp;
        uint8_t* payload;
    } YVRFrame;

    typedef void (*YVRDataChannelRecvCallback)(YVRFrame* msgFrame, void* pUser);

	class YVRTCENGINE YVRTCEngine
	{
	public:
		YVRTCEngine();
		virtual ~YVRTCEngine();

        void SetP2PBinaryMessageReceived(YVRDataChannelRecvCallback callback, void* pUser);
        void SendP2PBinaryMessage(YVRFrame* yvrFrame);

        int32_t putVideoFrame(YVRFrame* pFrame);

    public:
        YVRDataChannelRecvCallback m_callback;
        void* m_pUser;

	};

    class YVRTCENGINE YVPlayEngine {
    public:
        YVPlayEngine();
        virtual ~YVPlayEngine();

        int32_t YVPlayStart(std::string url);
        int32_t YVPlayStop();
        int32_t PollVideoFrame(YVRFrame* pFrame);

        void SetP2PBinaryMessageReceived(YVRDataChannelRecvCallback callback, void* pUser);
        void SendP2PBinaryMessage(YVRFrame* yvrFrame);

    public:
        YVRDataChannelRecvCallback m_callback;
        void* m_pUser;
    };

}


#endif
