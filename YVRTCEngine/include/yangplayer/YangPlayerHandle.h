//
// Copyright (c) 2019-2022 yanggaofeng
//
#ifndef INCLUDE_YANGPLAYER_YANGPLAYERHANDLE_H_
#define INCLUDE_YANGPLAYER_YANGPLAYERHANDLE_H_

#include <yangutil/buffer/YangVideoBuffer.h>
#include <yangutil/sys/YangSysMessageI.h>
#include <yangutil/yangavinfotype.h>
class YangPlayerHandle {
public:
	YangPlayerHandle(){};
	virtual ~YangPlayerHandle(){};
	virtual YangVideoBuffer* getVideoBuffer()=0;
	virtual int play(char* url)=0;
	virtual int32_t playRtc(int32_t puid,char* localIp, char* server, int32_t pport,char* app,char* stream)=0;
    virtual void stopPlay()=0;
    static YangPlayerHandle* createPlayerHandle(YangContext* pcontext);
	static YangPlayerHandle* createPlayerHandle();
	virtual int32_t getVideoFrame(YangFrame* pFrame) = 0;
        virtual int32_t checkConnectionState() = 0;

	virtual void initDataChannel(dataChannelRecvCallback callback, void* pUser) = 0;//wf_yvr
    virtual void dataChannelSend(YangFrame* msgFrame) = 0;//wf_yvr
};

#endif /* INCLUDE_YANGPLAYER_YANGPLAYERHANDLE_H_ */
