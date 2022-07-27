//
// Copyright (c) 2019-2022 yanggaofeng
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <yangutil/yangavinfotype.h>

#include <yangutil/sys/YangSysMessageI.h>
#include <yangutil/sys/YangSysMessageHandle.h>
// #include <yangp2p/YangP2pFactory.h>
#include "yangrecliving/YangLivingType.h"
#include "yangplayer/YangWinPlayFactroy.h"
#include "yangutil/buffer/YangVideoBuffer.h"

class RecordMainWindow : public YangSysMessageI,public YangSysMessageHandleI,public YangStreamStateNotify
{
public:
    RecordMainWindow();
    ~RecordMainWindow();
    YangContext *m_context;

    bool m_hasAudio;
    YangSysMessageHandle *m_message; 
    dataChannelRecvCallback m_datachannelCallback;
    void* m_pUser;
public:
    void setRecvText(char* data,int32_t nb);
    void initPreview();
    void streamStateNotify(int32_t puid,YangStreamOptType opt,bool isConnect);
    void success();
    void failure(int32_t errcode);
    void receiveSysMessage(YangSysMessage *psm, int32_t phandleRet);
    void setDataChannelCallback(dataChannelRecvCallback callback, void*pUser);
    void sendDataChannelData(YangFrame* msgFrame);


private:
    void init();
    void closeAll();
    void read_ip_address();

private:
    bool m_isStartpushplay;
   // bool m_isStartplay;
    // YangP2pFactory m_p2pfactory;

};
#endif // MAINWINDOW_H
