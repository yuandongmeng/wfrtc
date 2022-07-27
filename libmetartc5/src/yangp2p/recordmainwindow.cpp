//
// Copyright (c) 2019-2022 yanggaofeng
//
#include <yangp2p/recordmainwindow.h>
#include <yangp2p/YangP2pCommon.h>
#include <yangutil/sys/YangLog.h>
#include <yangutil/sys/YangFile.h>
#include <yangutil/sys/YangSocket.h>
#include <yangutil/sys/YangIni.h>
#include <yangutil/sys/YangString.h>
#include <yangp2p/YangP2pFactory.h>
#include <iostream>

void g_qt_p2p_receiveData(void *context, YangFrame *msgFrame)
{
    RecordMainWindow *win = (RecordMainWindow *)context;
    win->m_datachannelCallback(win->m_pUser,msgFrame);
    //win->setRecvText((char *)msgFrame->payload, msgFrame->nb);
}
RecordMainWindow::RecordMainWindow()
{
    m_context = new YangContext();
    m_context->init((char *)"yvr_config.ini");
    m_context->avinfo.video.videoEncoderFormat = YangI420;
#if Yang_Using_Openh264
    m_context->avinfo.enc.createMeta = 0;
#else
    m_context->avinfo.enc.createMeta = 1;
#endif
#if Yang_GPU_Encoding
    // using gpu encode
    m_context->avinfo.video.videoEncHwType = YangV_Hw_Nvdia; // YangV_Hw_Intel,  YangV_Hw_Nvdia,
    m_context->avinfo.video.videoEncoderFormat = YangArgb;   // YangI420;//YangArgb;
    m_context->avinfo.enc.createMeta = 0;
#endif
#if Yang_HaveVr
    // using vr bg file name
    memset(m_context->avinfo.bgFilename, 0, sizeof(m_ini->bgFilename));
    QSettings settingsread((char *)"yvr_config.ini", QSettings::IniFormat);
    strcpy(m_context->avinfo.bgFilename, settingsread.value("sys/bgFilename", QVariant("d:\\bg.jpeg")).toString().toStdString().c_str());
#endif

    init();
    yang_setLogLevle(m_context->avinfo.sys.logLevel);
    yang_setLogFile(m_context->avinfo.sys.hasLogFile);

    m_context->avinfo.sys.httpPort = 1988;
    m_context->avinfo.sys.mediaServer = Yang_Server_P2p; // Yang_Server_Srs/Yang_Server_Zlm/Yang_Server_P2p
    m_context->avinfo.sys.rtcLocalPort = 10000 + yang_random() % 15000;
    memset(m_context->avinfo.sys.localIp, 0, sizeof(m_context->avinfo.sys.localIp));
    yang_getLocalInfo(m_context->avinfo.sys.localIp);

    m_hasAudio = false;
    m_isStartpushplay = false;
    // using h264 h265
    m_context->avinfo.video.videoEncoderType = Yang_VED_264; // Yang_VED_265;

    m_context->streams.m_streamState = this;
    m_context->avinfo.audio.hasAec = 1;

    //m_context->channeldataRecv.context = this;
    //m_context->channeldataRecv.receiveData = g_qt_p2p_receiveData;

    //m_context->avinfo.rtc.usingDatachannel = 1;

    m_context->initDataChannel(g_qt_p2p_receiveData, this);//wf_yvr

    strcpy(m_context->avinfo.rtc.iceServerIP, "182.92.163.143");
    m_context->avinfo.rtc.iceStunPort = 3478;
    m_context->avinfo.rtc.hasIceServer = 0;
    m_pUser = nullptr;
}

RecordMainWindow::~RecordMainWindow()
{
    closeAll();
}

void RecordMainWindow::setRecvText(char *data, int32_t nb)
{
    printf("dataChannel receive:%s\n", data);
}

void RecordMainWindow::closeAll()
{

    if (m_context)
    {
        yang_delete(m_message);
        yang_delete(m_context);
    }
}

void RecordMainWindow::streamStateNotify(int32_t puid, YangStreamOptType opt, bool isConnect)
{
    std::cout << "online user play count==" << m_context->streams.getPlayOnlineCount() << ",push count==" << m_context->streams.getPushOnlineCount();
}
void RecordMainWindow::init()
{
    m_context->avinfo.audio.usingMono = 0;
    m_context->avinfo.audio.sample = 48000;
    m_context->avinfo.audio.channel = 2;
    m_context->avinfo.audio.hasAec = 1;
    m_context->avinfo.audio.audioCacheNum = 8;
    m_context->avinfo.audio.audioCacheSize = 8;
    m_context->avinfo.audio.audioPlayCacheNum = 8;

    m_context->avinfo.video.videoCacheNum = 10;
    m_context->avinfo.video.evideoCacheNum = 10;
    m_context->avinfo.video.videoPlayCacheNum = 10;

    m_context->avinfo.audio.audioEncoderType = Yang_AED_OPUS;
    m_context->avinfo.sys.rtcLocalPort = 17000;
    m_context->avinfo.enc.enc_threads = 4;
}

void RecordMainWindow::success()
{
    std::cout << "RecordMainWindow::success" << std::endl;
}

void RecordMainWindow::failure(int32_t errcode)
{
    std::cout << "RecordMainWindow::failure" << std::endl;
}

void RecordMainWindow::receiveSysMessage(YangSysMessage *mss, int32_t err)
{
    switch (mss->messageId)
    {
    case YangM_P2p_Connect:
    {
        if (err)
        {
            std::cout << "receiveSysMessage YangM_P2p_Connect err:" << err << std::endl;
        }
    }
    break;
    case YangM_P2p_Disconnect:
        break;
    case YangM_P2p_Play_Start:
        break;
    case YangM_P2p_Play_Stop:
        break;
    case YangM_P2p_StartVideoCapture:
        break;
    }
    std::cout << "receiveSysMessage::mss->messageId:" << mss->messageId << std::endl;
}

void RecordMainWindow::setDataChannelCallback(dataChannelRecvCallback callback, void*pUser)
{
    m_datachannelCallback = callback;
    m_pUser = pUser;
}

void RecordMainWindow::sendDataChannelData(YangFrame* msgFrame)
{
    if (m_context->channeldataSend.sendData)
    {
        m_context->channeldataSend.sendData(m_context->channeldataSend.context,msgFrame);
    }
}
