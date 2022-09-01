#ifndef __RTMP_H__
#define __RTMP_H__
/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *      Copyright (C) 2008-2009 Andrej Stepanchuk
 *      Copyright (C) 2009-2010 Howard Chu
 *
 *  This file is part of librtmp.
 *
 *  librtmp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1,
 *  or (at your option) any later version.
 *
 *  librtmp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with librtmp see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/lgpl.html
 */

#if !defined(NO_CRYPTO) && !defined(CRYPTO)
#define CRYPTO
#endif

#include <errno.h>
#include <stdint.h>
#include <stddef.h>

#include "amf.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RTMP_LIB_VERSION	0x020300	/* 2.3 */

#define RTMP_FEATURE_HTTP	0x01
#define RTMP_FEATURE_ENC	0x02
#define RTMP_FEATURE_SSL	0x04
#define RTMP_FEATURE_MFP	0x08	/* not yet supported */
#define RTMP_FEATURE_WRITE	0x10	/* publish, not play */
#define RTMP_FEATURE_HTTP2	0x20	/* server-side rtmpt */

#define RTMP_PROTOCOL_UNDEFINED	-1
#define RTMP_PROTOCOL_RTMP      0
#define RTMP_PROTOCOL_RTMPE     RTMP_FEATURE_ENC
#define RTMP_PROTOCOL_RTMPT     RTMP_FEATURE_HTTP
#define RTMP_PROTOCOL_RTMPS     RTMP_FEATURE_SSL
#define RTMP_PROTOCOL_RTMPTE    (RTMP_FEATURE_HTTP|RTMP_FEATURE_ENC)
#define RTMP_PROTOCOL_RTMPTS    (RTMP_FEATURE_HTTP|RTMP_FEATURE_SSL)
#define RTMP_PROTOCOL_RTMFP     RTMP_FEATURE_MFP

#define RTMP_DEFAULT_CHUNKSIZE	128

/* needs to fit largest number of bytes recv() may return */
#define RTMP_BUFFER_CACHE_SIZE (16*1024)

#define	RTMP_CHANNELS	65600

  extern const char RTMPProtocolStringsLower[][7];
  extern const AVal RTMP_DefaultFlashVer;
  extern int RTMP_ctrlC;

  uint32_t RTMP_GetTime(void);

/*      RTMP_PACKET_TYPE_...                0x00 */
#define RTMP_PACKET_TYPE_CHUNK_SIZE         0x01
/*      RTMP_PACKET_TYPE_...                0x02 */
#define RTMP_PACKET_TYPE_BYTES_READ_REPORT  0x03
#define RTMP_PACKET_TYPE_CONTROL            0x04
#define RTMP_PACKET_TYPE_SERVER_BW          0x05
#define RTMP_PACKET_TYPE_CLIENT_BW          0x06
/*      RTMP_PACKET_TYPE_...                0x07 */
#define RTMP_PACKET_TYPE_AUDIO              0x08
#define RTMP_PACKET_TYPE_VIDEO              0x09
/*      RTMP_PACKET_TYPE_...                0x0A */
/*      RTMP_PACKET_TYPE_...                0x0B */
/*      RTMP_PACKET_TYPE_...                0x0C */
/*      RTMP_PACKET_TYPE_...                0x0D */
/*      RTMP_PACKET_TYPE_...                0x0E */
#define RTMP_PACKET_TYPE_FLEX_STREAM_SEND   0x0F
#define RTMP_PACKET_TYPE_FLEX_SHARED_OBJECT 0x10
#define RTMP_PACKET_TYPE_FLEX_MESSAGE       0x11
#define RTMP_PACKET_TYPE_INFO               0x12
#define RTMP_PACKET_TYPE_SHARED_OBJECT      0x13
#define RTMP_PACKET_TYPE_INVOKE             0x14
/*      RTMP_PACKET_TYPE_...                0x15 */
#define RTMP_PACKET_TYPE_FLASH_VIDEO        0x16

#define RTMP_MAX_HEADER_SIZE 18

#define RTMP_PACKET_SIZE_LARGE    0
#define RTMP_PACKET_SIZE_MEDIUM   1
#define RTMP_PACKET_SIZE_SMALL    2
#define RTMP_PACKET_SIZE_MINIMUM  3

  typedef struct RTMPChunk
  {
    int c_headerSize;
    int c_chunkSize;
    char *c_chunk;
    char c_header[RTMP_MAX_HEADER_SIZE];
  } RTMPChunk;

  typedef struct RTMPPacket
  {
    uint8_t m_headerType;// 块头类型
    uint8_t m_packetType;// 负载格式
    uint8_t m_hasAbsTimestamp;	/* timestamp absolute or relative? */// 是否绝对时间戳
    int m_nChannel;// 块流ID
    uint32_t m_nTimeStamp;	/* timestamp */// 时间戳
    int32_t m_nInfoField2;	/* last 4 bytes in a long header */// 块流ID
    uint32_t m_nBodySize; // 负载大小
    uint32_t m_nBytesRead;// 读入负载大小
    RTMPChunk *m_chunk;// 在RTMP_ReadPacket()调用时，若该字段非NULL，表示关心原始块的信息，通常设为NULL
    char *m_body;// 负载指针
  } RTMPPacket;

  typedef struct RTMPSockBuf
  {
    int sb_socket;
    int sb_size;		/* number of unprocessed bytes in buffer */
    char *sb_start;		/* pointer into sb_pBuffer of next byte to process */
    char sb_buf[RTMP_BUFFER_CACHE_SIZE];	/* data read from socket */
    int sb_timedout;
    void *sb_ssl;
  } RTMPSockBuf;

  void RTMPPacket_Reset(RTMPPacket *p);// 重置报文
  void RTMPPacket_Dump(RTMPPacket *p);
  int RTMPPacket_Alloc(RTMPPacket *p, uint32_t nSize);// 为报文分配负载空间
  void RTMPPacket_Free(RTMPPacket *p);// 释放负载空间

#define RTMPPacket_IsReady(a)	((a)->m_nBytesRead == (a)->m_nBodySize)

  typedef struct RTMP_LNK
  {
    AVal hostname;
    AVal sockshost;

    AVal playpath0;	/* parsed from URL */
    AVal playpath;	/* passed in explicitly */
    AVal tcUrl;
    AVal swfUrl;
    AVal pageUrl;
    AVal app;
    AVal auth;
    AVal flashVer;
    AVal subscribepath;
    AVal usherToken;
    AVal token;
    AVal pubUser;
    AVal pubPasswd;
    AMFObject extras;
    int edepth;

    int seekTime;
    int stopTime;

#define RTMP_LF_AUTH	0x0001	/* using auth param */
#define RTMP_LF_LIVE	0x0002	/* stream is live */
#define RTMP_LF_SWFV	0x0004	/* do SWF verification */
#define RTMP_LF_PLST	0x0008	/* send playlist before play */
#define RTMP_LF_BUFX	0x0010	/* toggle stream on BufferEmpty msg */
#define RTMP_LF_FTCU	0x0020	/* free tcUrl on close */
#define RTMP_LF_FAPU	0x0040	/* free app on close */
    int lFlags;

    int swfAge;

    int protocol;
    int timeout;		/* connection timeout in seconds */

    int pFlags;			/* unused, but kept to avoid breaking ABI */

    unsigned short socksport;
    unsigned short port;

#ifdef CRYPTO
#define RTMP_SWF_HASHLEN	32
    void *dh;			/* for encryption */
    void *rc4keyIn;
    void *rc4keyOut;

    uint32_t SWFSize;
    uint8_t SWFHash[RTMP_SWF_HASHLEN];
    char SWFVerificationResponse[RTMP_SWF_HASHLEN+10];
#endif
  } RTMP_LNK;

  /* state for read() wrapper */
  typedef struct RTMP_READ
  {
    char *buf;
    char *bufpos;
    unsigned int buflen;
    uint32_t timestamp;
    uint8_t dataType;
    uint8_t flags;
#define RTMP_READ_HEADER	0x01
#define RTMP_READ_RESUME	0x02
#define RTMP_READ_NO_IGNORE	0x04
#define RTMP_READ_GOTKF		0x08
#define RTMP_READ_GOTFLVK	0x10
#define RTMP_READ_SEEKING	0x20
    int8_t status;
#define RTMP_READ_COMPLETE	-3
#define RTMP_READ_ERROR	-2
#define RTMP_READ_EOF	-1
#define RTMP_READ_IGNORE	0

    /* if bResume == TRUE */
    uint8_t initialFrameType;
    uint32_t nResumeTS;
    char *metaHeader;
    char *initialFrame;
    uint32_t nMetaHeaderSize;
    uint32_t nInitialFrameSize;
    uint32_t nIgnoredFrameCounter;
    uint32_t nIgnoredFlvFrameCounter;
  } RTMP_READ;

  typedef struct RTMP_METHOD
  {
    AVal name;
    int num;
  } RTMP_METHOD;

  typedef struct RTMP
  {
    int m_inChunkSize;// 最大接收块大小
    int m_outChunkSize;// 最大发送块大小
    int m_nBWCheckCounter;// 带宽检测计数器
    int m_nBytesIn; // 接收数据计数器
    int m_nBytesInSent;// 当前数据已回应计数器
    int m_nBufferMS; // 当前缓冲的时间长度，以MS为单位
    int m_stream_id;		/* returned in _result from createStream */// 当前连接的流ID
    int m_mediaChannel;// 当前连接媒体使用的块流ID
    uint32_t m_mediaStamp;// 当前连接媒体最新的时间戳
    uint32_t m_pauseStamp;// 当前连接媒体暂停时的时间戳
    int m_pausing; // 是否暂停状态
    int m_nServerBW; // 服务器带宽
    int m_nClientBW; // 客户端带宽
    uint8_t m_nClientBW2;// 客户端带宽调节方式
    uint8_t m_bPlaying;// 当前是否推流或连接中
    uint8_t m_bSendEncoding;// 连接服务器时发送编码
    uint8_t m_bSendCounter;// 设置是否向服务器发送接收字节应答

    int m_numInvokes;// 0x14命令远程过程调用计数
    int m_numCalls; // 0x14命令远程过程请求队列数量
    RTMP_METHOD *m_methodCalls;	/* remote method calls queue */// 远程过程调用请求队列


    int m_channelsAllocatedIn;
    int m_channelsAllocatedOut;
    RTMPPacket **m_vecChannelsIn;// 对应块流ID上一次接收的报文
    RTMPPacket **m_vecChannelsOut;// 对应块流ID上一次发送的报文
    int *m_channelTimestamp;	/* abs timestamp of last packet */ // 对应块流ID媒体的最新时间戳

    double m_fAudioCodecs;	/* audioCodecs for the connect packet */// 音频编码器代码
    double m_fVideoCodecs;	/* videoCodecs for the connect packet */ // 视频编码器代码
    double m_fEncoding;		/* AMF0 or AMF3 */

    double m_fDuration;		/* duration of stream in seconds */// 当前媒体的时长

    int m_msgCounter;		/* RTMPT stuff */ // 使用HTTP协议发送请求的计数器
    int m_polling; // 使用HTTP协议接收消息主体时的位置
    int m_resplen; // 使用HTTP协议接收消息主体时的未读消息计数
    int m_unackd;// 使用HTTP协议处理时无响应的计数
    AVal m_clientID; // 使用HTTP协议处理时的身份ID

    RTMP_READ m_read;// RTMP_Read()操作的上下文
    RTMPPacket m_write;// RTMP_Write()操作使用的可复用报文对象
    RTMPSockBuf m_sb;// RTMP_ReadPacket()读包操作的上下文
    RTMP_LNK Link;// RTMP连接上下文
  } RTMP;

  // 解析流地址
  int RTMP_ParseURL(const char *url, int *protocol, AVal *host,
		     unsigned int *port, AVal *playpath, AVal *app);

  void RTMP_ParsePlaypath(AVal *in, AVal *out);
  void RTMP_SetBufferMS(RTMP *r, int size);// 连接前，设置服务器发送给客户端的媒体缓存时长
  void RTMP_UpdateBufferMS(RTMP *r);// 连接后，更新服务器发送给客户端的媒体缓存时长

  int RTMP_SetOpt(RTMP *r, const AVal *opt, AVal *arg);// 更新RTMP上下文中的相应选项
  int RTMP_SetupURL(RTMP *r, char *url);// 设置流地址
  // 设置RTMP上下文播放地址和相应选项，不关心的可以设为NULL
  void RTMP_SetupStream(RTMP *r, int protocol,
			AVal *hostname,
			unsigned int port,
			AVal *sockshost,
			AVal *playpath,
			AVal *tcUrl,
			AVal *swfUrl,
			AVal *pageUrl,
			AVal *app,
			AVal *auth,
			AVal *swfSHA256Hash,
			uint32_t swfSize,
			AVal *flashVer,
			AVal *subscribepath,
			AVal *usherToken,
			int dStart,
			int dStop, int bLiveStream, long int timeout);

  int RTMP_Connect(RTMP *r, RTMPPacket *cp);// 客户端连接及握手
  struct sockaddr;
  int RTMP_Connect0(RTMP *r, struct sockaddr *svc);
  int RTMP_Connect1(RTMP *r, RTMPPacket *cp);
  int RTMP_Serve(RTMP *r);// 服务端握手
  int RTMP_TLS_Accept(RTMP *r, void *ctx);

  int RTMP_ReadPacket(RTMP *r, RTMPPacket *packet);// 接收一个报文
  int RTMP_SendPacket(RTMP *r, RTMPPacket *packet, int queue);// 发送一个报文，queue为1表示当包类型为0x14时，将加入队列等待响应
  int RTMP_SendChunk(RTMP *r, RTMPChunk *chunk);// 直接发送块
  int RTMP_IsConnected(RTMP *r);// 检查网络是否连接
  int RTMP_Socket(RTMP *r);// 返回套接字
  int RTMP_IsTimedout(RTMP *r);// 检查连接是否超时
  double RTMP_GetDuration(RTMP *r);// 获取当前媒体的时长
  int RTMP_ToggleStream(RTMP *r);// 暂停与播放切换控制

  int RTMP_ConnectStream(RTMP *r, int seekTime);// 连接流，并指定开始播放的位置
  int RTMP_ReconnectStream(RTMP *r, int seekTime);// 重新创建流
  void RTMP_DeleteStream(RTMP *r);// 删除当前流
  int RTMP_GetNextMediaPacket(RTMP *r, RTMPPacket *packet);// 获取第一个媒体包
  int RTMP_ClientPacket(RTMP *r, RTMPPacket *packet);// 处理客户端的报文交互，即处理报文分派逻辑

  void RTMP_Init(RTMP *r);// 初使化RTMP上下文，设默认值
  void RTMP_Close(RTMP *r);// 关闭RTMP上下文
  RTMP *RTMP_Alloc(void);// 分配RTMP上下文
  void RTMP_Free(RTMP *r);// 释放RTMP上下文
  void RTMP_EnableWrite(RTMP *r);// 开启客户端的RTMP写开关，用于推流

  void *RTMP_TLS_AllocServerContext(const char* cert, const char* key);
  void RTMP_TLS_FreeServerContext(void *ctx);

  int RTMP_LibVersion(void);// 返回RTMP的版本
  void RTMP_UserInterrupt(void);	/* user typed Ctrl-C */// 开启RTMP工作中断

  // 发送0x04号命令的控制消息
  int RTMP_SendCtrl(RTMP *r, short nType, unsigned int nObject,
		     unsigned int nTime);

  /* caller probably doesn't know current timestamp, should
   * just use RTMP_Pause instead
   */
  // 发送0x14号远程调用控制暂停
  int RTMP_SendPause(RTMP *r, int DoPause, int dTime);
  int RTMP_Pause(RTMP *r, int DoPause);

  // 递归在一个对象中搜索指定的属性
  int RTMP_FindFirstMatchingProperty(AMFObject *obj, const AVal *name,
				      AMFObjectProperty * p);

  // 底层套接口的网络读取、发送、关闭连接操作
  int RTMPSockBuf_Fill(RTMPSockBuf *sb);
  int RTMPSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len);
  int RTMPSockBuf_Close(RTMPSockBuf *sb);

  int RTMP_SendCreateStream(RTMP *r);// 发送建流操作
  int RTMP_SendSeek(RTMP *r, int dTime);// 发送媒体时间定位操作
  int RTMP_SendServerBW(RTMP *r);// 发送设置服务器应答窗口大小操作
  int RTMP_SendClientBW(RTMP *r);// 发送设置服务器输出带宽操作
  void RTMP_DropRequest(RTMP *r, int i, int freeit);// 删除0x14命令远程调用队列中的请求
  int RTMP_Read(RTMP *r, char *buf, int size);// 读取FLV格式数据
  int RTMP_Write(RTMP *r, const char *buf, int size);// 发送FLV格式数据

/* hashswf.c */
  int RTMP_HashSWF(const char *url, unsigned int *size, unsigned char *hash,
		   int age);

#ifdef __cplusplus
};
#endif

#endif
