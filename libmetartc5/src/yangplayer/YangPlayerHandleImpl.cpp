//
// Copyright (c) 2019-2022 yanggaofeng
//
#include "YangPlayerHandleImpl.h"
#include <yangutil/sys/YangLog.h>
#include <string.h>
#include <yangutil/yang_unistd.h>
#include <yangutil/sys/YangSocket.h>
#include <yangutil/sys/YangMath.h>

YangPlayerHandle* YangPlayerHandle::createPlayerHandle(YangContext* pcontext){
	return new YangPlayerHandleImpl(pcontext);
}

YangPlayerHandle* YangPlayerHandle::createPlayerHandle(){
	YangContext* context = new YangContext();
    context->init();
	// strcpy(context->avinfo.rtc.iceServerIP,"127.0.0.1");//stun server ip
    // context->avinfo.rtc.iceStunPort=3478;
    // context->avinfo.rtc.hasIceServer=0;
    context->streams.m_playBuffer = new YangSynBuffer();
    context->avinfo.rtc.usingDatachannel = 0;
    context->avinfo.sys.mediaServer = Yang_Server_Srs; // Yang_Server_Srs/Yang_Server_Zlm

    // YangV_Hw_Android mediacodec
    // Yang_Hw_Soft yangh264decoder
    context->avinfo.video.videoDecHwType = Yang_Hw_Soft;

    context->avinfo.sys.rtcLocalPort = 10000 + yang_random() % 15000;
    memset(context->avinfo.sys.localIp, 0, sizeof(context->avinfo.sys.localIp));
    yang_getLocalInfo(context->avinfo.sys.localIp);
    yang_setLogLevle(5);
    yang_setLogFile(1);
	return new YangPlayerHandleImpl(context);
}

YangPlayerHandleImpl::YangPlayerHandleImpl(YangContext* pcontext) {
	m_context=pcontext;
	// m_message=pmessage;
	m_recv = NULL;
	m_play = NULL;
	m_rtcRecv=NULL;
	m_outVideoBuffer = NULL;
	m_outAudioBuffer = NULL;
	m_url.netType=0;
	m_url.port=1935;

	externalBuffer = new uint8_t[150 * 1024];
}

YangPlayerHandleImpl::~YangPlayerHandleImpl() {
	if(m_rtcRecv) m_rtcRecv->disConnect();
	yang_delete(m_recv);
	yang_delete(m_play);
	yang_delete(m_rtcRecv);
	yang_delete(m_outVideoBuffer);
	yang_delete(m_outAudioBuffer);
	yang_delete(m_context);
	yang_delete(externalBuffer);
}

void YangPlayerHandleImpl::stopPlay(){
	if(m_rtcRecv) {
		m_rtcRecv->disConnect();
	}
	if(m_recv){
		m_recv->disConnect();
	}
	if(m_play) m_play->stopAll();
	if(m_rtcRecv){
		yang_stop(m_rtcRecv);
		yang_stop_thread(m_rtcRecv);
		yang_delete(m_rtcRecv);
	}

	if(m_recv){
		yang_stop(m_recv);
		yang_stop_thread(m_recv);
		yang_delete(m_recv);
	}

    yang_delete(m_play);

}
int YangPlayerHandleImpl::play(char* url) {
	memset(m_url.server,0,sizeof(m_url.server));
	m_url.port=0;
	if(yang_url_parse(url,&m_url)) return 1;

	stopPlay();
	yang_trace("\nnetType==%d,server=%s,port=%d,app=%s,stream=%s\n",m_url.netType,m_url.server,m_url.port,m_url.app,m_url.stream);
	m_context->avinfo.sys.transType=m_url.netType;
	if(m_context->streams.m_playBuffer) m_context->streams.m_playBuffer->setTranstype(m_url.netType);
    if(m_url.netType ==Yang_Webrtc){

        return playRtc(0,m_url.server,m_url.server,m_url.port,m_url.app,m_url.stream);

    }

	if (!m_play)	{
		m_play = new YangPlayerBase();

        m_context->avinfo.audio.sample=44100;
        m_context->avinfo.audio.channel=2;
        m_context->avinfo.audio.audioDecoderType=Yang_AED_AAC;
        m_context->avinfo.audio.usingMono=0;
        m_context->avinfo.audio.aIndex=-1;
        m_play->init(m_context);
	}
	initList();
	m_play->startAudioDecoder(m_outAudioBuffer);
	m_play->startVideoDecoder(m_outVideoBuffer);
	m_play->startAudioPlay(m_context);

	if (!m_recv) {

		m_recv = new YangPlayReceive(m_context);
		m_recv->setBuffer(m_outAudioBuffer, m_outVideoBuffer);
	}
	if(m_recv->init(m_url.netType, m_url.server, m_url.port,m_url.app, m_url.stream)){
		printf("\n connect server failure!");
		return 1;
	}

	m_recv->start();
	return Yang_Ok;


}

int32_t YangPlayerHandleImpl::playRtc(int32_t puid,char* localIp,char* server, int32_t pport,char* app,char* stream){

	stopPlay();
	if (!m_play)	{
		m_play = new YangPlayerBase();

		m_context->avinfo.audio.sample=48000;
		m_context->avinfo.audio.channel=2;
		m_context->avinfo.audio.audioDecoderType=Yang_AED_OPUS;//3;
		m_context->avinfo.audio.usingMono=0;
        m_context->avinfo.audio.aIndex=-1;
		m_play->init(m_context);
	}
	initList();
	m_play->startAudioDecoder(m_outAudioBuffer);
	m_play->startVideoDecoder(m_outVideoBuffer);

	m_play->startAudioPlay(m_context);


	if(m_rtcRecv==NULL) {
		m_rtcRecv=new YangRtcReceive(m_context);
		m_rtcRecv->setBuffer(m_outAudioBuffer, m_outVideoBuffer);
		m_rtcRecv->init(puid,localIp,server,pport,app,stream);
	}

	 m_rtcRecv->start();

	 return Yang_Ok;
}

YangVideoBuffer* YangPlayerHandleImpl::getVideoBuffer(){
	if(m_play) return m_play->m_ydb->getOutVideoBuffer();
	return NULL;
}

void YangPlayerHandleImpl::initList() {
	if (m_outAudioBuffer == NULL) {
		m_outAudioBuffer = new YangAudioEncoderBuffer(10);
	}
	if (m_outVideoBuffer == NULL)
		m_outVideoBuffer = new YangVideoDecoderBuffer();

}

#include <iostream>

int32_t YangPlayerHandleImpl::getVideoFrame(YangFrame* pFrame)
{
	int32_t ret = -1;
	if (m_outVideoBuffer == NULL) {
		return -2;
	}
	if (m_outVideoBuffer->size() == 0) {
		return -3;
	}
	// std::cout << "checkConnectionState:" << ret << std::endl;
	if (checkConnectionState() == 1) {
		pFrame->payload = externalBuffer;
		ret = m_outVideoBuffer->getEVideo(pFrame);
	}
	else {
		ret = -4;
	}
	return ret;
}

int32_t YangPlayerHandleImpl::checkConnectionState()
{
	int32_t ret = 0;
	if(m_rtcRecv != NULL) {
		ret = m_rtcRecv->isConnected();
	}
	return ret;
}

//wf_yvr 他的基类里要声明一个纯虚函数
void YangPlayerHandleImpl::initDataChannel(dataChannelRecvCallback callback, void* pUser)
{
    //init recv
    m_context->channeldataRecv.context = pUser;
    m_context->channeldataRecv.receiveData = callback;
    m_context->avinfo.rtc.usingDatachannel=1;
}

//wf_yvr 他的基类里要声明一个纯虚函数
void YangPlayerHandleImpl::dataChannelSend(YangFrame* msgFrame)
{
    if (m_rtcRecv)
    {
       m_rtcRecv->publishMsg(msgFrame);
    }

}
