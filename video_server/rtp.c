#include <stddef.h>
#include "rtp.h"


//��ʼ��RTPͷ,�����в����ֶ�����Ϊȱʡֵ
void InitRtpHdr(RTP_HEADER_T *prtphdr, int pt)
{
    if (NULL == prtphdr)
    {
        return;
    }
    memset(prtphdr, 0, sizeof(RTP_HEADER_T));

    prtphdr->version = M_RTP_VER;
    prtphdr->p = 0;
    prtphdr->x = 0;
    prtphdr->pt = pt;
    prtphdr->ts = 0;
}

