#ifndef _RTP_H__
#define _RTP_H__

//RTP头结构
typedef struct _rtp_header
{
    unsigned int version;       //RTP版本号
    unsigned int p;             //PADDING标志
    unsigned int x;             //头扩展标志
    unsigned int cc;            //CSRC个数
    unsigned int m;             //标志位
    unsigned int pt;            //PAYLOAD类型
    unsigned int seq;           //包序列号
    unsigned int ts;            //时间戳
    unsigned int ssrc;          //同步SRC
    unsigned int csrc[1];       //可选CSRC列表
}RTP_HEADER_T;

//相关宏定义
#define M_RTP_VER       2
#define M_PT_MJPEG      26
#define M_PT_MPEG4      96
#define M_PT_TS         99

//初始化RTP头结构定义
void InitRtpHdr(RTP_HEADER_T *rtphdr, int pt);


#endif //_RTP_H__
