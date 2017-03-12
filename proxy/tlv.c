#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"
#include "event_manage.h"

#include "tlv.h"
#include "channel.h"


void init_tlv_module()
{
    register_event_dispatcher(tlv_msg_dispatcher);
}


int tlv_msg_dispatcher(void *data)
{
    S_TLV_EVENT  *pEvt = (S_TLV_EVENT *)data;
    COMM_CHANNEL *pChn = NULL;
    S_TLV_MSG *pTLV    = NULL;
    int iRet           = 0;

    SYS_LOG_PRINTF("TLV message dispatcher is called!\n");

    if (data == NULL)
    {
        return -1;
    }
    
    pChn = (COMM_CHANNEL *)pEvt->pChannel;
    if (pChn == NULL || pChn->bUsed == 0)
    {
        return -1;
    }
    pTLV = (S_TLV_MSG *)pEvt->szMsgBuf;

    SYS_LOG_PRINTF("T:%d, L:%d, V:%s\n", pTLV->t, pTLV->l, pTLV->v);

    switch (pTLV->t)
    {
        case E_PROT_TYPE_XML:
        {
            iRet = xml_msg_dispatcher(pTLV->v, pTLV->l, (void *)pChn);
        }
        break;

        default:
        {
            iRet = -1;
        }
        break;
    }

    return iRet;
}
