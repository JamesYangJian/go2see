#ifndef _RTP_H__
#define _RTP_H__

//RTPͷ�ṹ
typedef struct _rtp_header
{
    unsigned int version;       //RTP�汾��
    unsigned int p;             //PADDING��־
    unsigned int x;             //ͷ��չ��־
    unsigned int cc;            //CSRC����
    unsigned int m;             //��־λ
    unsigned int pt;            //PAYLOAD����
    unsigned int seq;           //�����к�
    unsigned int ts;            //ʱ���
    unsigned int ssrc;          //ͬ��SRC
    unsigned int csrc[1];       //��ѡCSRC�б�
}RTP_HEADER_T;

//��غ궨��
#define M_RTP_VER       2
#define M_PT_MJPEG      26
#define M_PT_MPEG4      96
#define M_PT_TS         99

//��ʼ��RTPͷ�ṹ����
void InitRtpHdr(RTP_HEADER_T *rtphdr, int pt);


#endif //_RTP_H__
