#ifndef YVR_COMM_H
#define YVR_COMM_H

#include <stdint.h>
#include <iostream>
#include <string>

#define SEND_KBPS_AUTO_CALCULATE -1

namespace yvrtc
{
enum P2PRole {
       kP2PRoleClient = 0,
       kP2PRoleServer = 1,
   };

   struct EngineConfig {
       const char* app_id = nullptr;
       P2PRole role = kP2PRoleClient;
       const char* server_ip = nullptr;
       int server_port_min = 50000;
       int server_port_max = 50004;
   };

   typedef enum stStreamIndex{
       kStreamIndexIndex0,
       kStreamIndexIndex1
   }StreamIndex;

   enum ConnectionState {
       kConnectionStateDisconnected = 1,
       kConnectionStateConnecting = 2,
       kConnectionStateConnected = 3,
       kConnectionStateReconnecting = 4,
       kConnectionStateReconnected = 5,
   };

   enum VideoRotation {
       /**
        * @brief 顺时针旋转 0 度
       */
       kVideoRotation0 = 0,
       /**
        * @brief 顺时针旋转 90 度
       */
       kVideoRotation90 = 90,
       /**
        * @brief 顺时针旋转 180 度
       */
       kVideoRotation180 = 180,
       /**
        * @brief 顺时针旋转 270 度
       */
       kVideoRotation270 = 270
   };

   enum VideoCodecType {
       /**
        * @hidden
        * @brief 未知类型
        */
       kVideoCodecTypeUnknown = 0,
       /**
        * @brief 标准 H264 编码器
        */
       kVideoCodecTypeH264 = 1,
       /**
        * @brief 标准 ByteVC1 编码器
        */
       kVideoCodecTypeByteVC1 = 2,
   };

   enum VideoPictureType {
       /**
        * @brief 未知类型
        */
       kVideoPictureTypeUnknown = 0,
       /**
        * @brief I帧，关键帧，编解码不需要参考其他视频帧
        */
       kVideoPictureTypeI,
       /**
        * @brief P帧，向前参考帧，编解码需要参考前一帧视频帧
        */
       kVideoPictureTypeP,
       /**
        * @brief B帧，前后参考帧，编解码需要参考前后两帧视频帧
        */
       kVideoPictureTypeB
   };

   typedef struct EncodedVideoFrameBuilder {
       /**
        * @hidden
        * @brief 视频帧编码格式，详见 VideoCodecType:{@link #VideoCodecType}
        */
       VideoCodecType codec_type = kVideoCodecTypeUnknown;
       /**
        * @hidden
        * @brief 视频帧编码类型，详见 VideoPictureType:{@link #VideoPictureType}
        */
       VideoPictureType picture_type = kVideoPictureTypeUnknown;
       /**
        * @brief 视频旋转角度
        * @notes 目前支持 0，90，180，270 几种旋转角度
        */
       VideoRotation rotation = kVideoRotation0;
       /**
        * @brief 视频帧数据指针
        * @notes IEncodedVideoFrame会获取数据的所有权
        */
       uint8_t* data;
       /**
        * @brief 视频帧内存空间大小
        */
       int size = 0;
       /**
        * @brief 视频帧宽度
        */
       int width = 0;
       /**
        * @brief 视频帧高度
        */
       int height = 0;
       /**
        * @brief 视频采集时间戳
        */
       int64_t timestamp_us = 0;
       /**
        * @brief 视频编码时间戳，单位：微秒
        */
       int64_t timestamp_dts_us = 0;
       /**
        * @brief 用户定义的视频帧释放回调函数指针，如果指针不为空，方法会被调用用来释放内存空间
        */
       int (*memory_deleter)(uint8_t* data, int size, void* user_opaque) = nullptr;
   } EncodedVideoFrameBuilder;

   class IEncodedVideoFrame {
   public:
       /**
        * @brief 获取视频编码类型
        * @return VideoCodecType 视频编码类型，参看 VideoCodecType{@link #VideoCodecType}
        */
       virtual VideoCodecType codec_type() const = 0;
       /**
        * @brief 获取视频采集时间戳
        * @return 视频采集时间戳，单位：微秒
        */
       virtual int64_t timestamp_us() const = 0;
       /**
        * @brief 获取视频编码时间戳
        * @return 视频编码时间戳，单位：微秒
        */
       virtual int64_t timestamp_dts_us() const = 0;
       /**
        * @brief 获取视频帧宽度
        * @return 视频帧宽度，单位：px
        */
       virtual int width() const = 0;
       /**
        * @brief 获取视频帧高度
        * @return 视频帧高度，单位：px
        */
       virtual int height() const = 0;
       /**
        * @brief 获取视频编码帧类型
        * @return 视频编码格式，参看 VideoPictureType{@link #VideoPictureType}
        */
       virtual VideoPictureType picture_type() const = 0;
       /**
        * @brief 获取视频帧旋转角度
        * @return 视频帧旋转角度，参看 VideoRotation{@link #VideoRotation}
        */
       virtual VideoRotation rotation() const = 0;
       /**
        * @brief 获取视频帧数据指针地址
        * @return 视频帧数据指针地址
        */
       virtual uint8_t* data() const = 0;
       /**
        * @brief 获取视频帧数据大小
        * @return 视频帧数据大小
        */
       virtual int data_size() const = 0;

        /**
        * @brief 浅拷贝视频帧并返回指针
        */
       virtual IEncodedVideoFrame* shallow_copy() const = 0;
       /**
        * @brief 释放视频帧
        */
       virtual void release() = 0;
       /**
        * @brief 根据视频帧参数创建视频帧并返回指针
        * @param [in] VideoFrameBuilder 视频帧创建类对象，详见 VideoFrameBuilder：{@link #VideoFrameBuilder}
        */
       static IEncodedVideoFrame* build_encoded_video_frame(const EncodedVideoFrameBuilder& builder) {};
       /**
        * @hidden
        */
   protected:
       /**
        * @brief 析构函数
        */
       virtual ~IEncodedVideoFrame() = default;
   };

   enum VideoStreamScaleMode {
       /**
        * @brief 自动模式，默认值为 FitWithCropping。
        */
       kVideoStreamScaleModeAuto = 0,
       /**
        * @brief 对视频帧进行缩放，直至充满和视窗分辨率一致为止。这一过程不保证等比缩放。
        */
       kVideoStreamScaleModeStretch = 1,
       /**
        * @brief 视窗填满优先。<br>
        *        视频帧等比缩放，直至视窗被视频填满。如果视频帧长宽比例与视窗不同，视频帧的多出部分将无法显示。<br>
        *        缩放完成后，视频帧的一边长和视窗的对应边长一致，另一边长大于等于视窗对应边长。
        */
       kVideoStreamScaleModeFitWithCropping = 2,
       /**
        * @brief 视频帧内容全部显示优先。<br>
        *        视频帧等比缩放，直至视频帧能够在视窗上全部显示。如果视频帧长宽比例与视窗不同，视窗上未被视频帧填满区域将被涂黑。<br>
        *        缩放完成后，视频帧的一边长和视窗的对应边长一致，另一边长小于等于视窗对应边长。
        */
       kVideoStreamScaleModeFitWithFilling = 3,
   };

   /**
    * @type keytype
    * @brief 网络不好时的视频编码降级策略
    */
   enum VideoEncodePreference {
       /**
        * @brief 不降级
        */
       kVideoEncodePreferenceDisabled = 0,
       /**
        * @brief 优先保证帧率，以保证视频流畅度
        */
       kVideoEncodePreferenceFramerate,
       /**
        * @brief 优先保证画质
        */
       kVideoEncodePreferenceQuality,
       /**
        * @brief 平衡模式
        */
       kVideoEncodePreferenceBalance,
   };

   struct VideoSolution {
       int width;
       int height;
       int fps;
       int max_send_kbps = SEND_KBPS_AUTO_CALCULATE;
       VideoStreamScaleMode scale_mode = VideoStreamScaleMode::kVideoStreamScaleModeAuto;
       VideoEncodePreference encode_preference = VideoEncodePreference::kVideoEncodePreferenceFramerate;
   };

}

#endif // YVR_COMM_H
