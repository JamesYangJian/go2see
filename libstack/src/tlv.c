#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"
#include "event_manage.h"

#include "tlv.h"
#include "channel.h"

static PROTO_MSG_DISPATCHER xml_message_dispatcher = NULL;
static PROTO_MSG_DISPATCHER ts_message_dispatcher = NULL;

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
            if (xml_message_dispatcher != NULL)
            {
                iRet = xml_msg_dispatcher(pTLV->v, pTLV->l, (void *)pChn);
            }
            else
            {
                SYS_LOG_PRINTF("XML message dispatcher not registered!\n");
                return -1;
            }
        }
        break;

        case E_PROT_TYPE_TS:
        {
            if (ts_message_dispatcher != NULL)
            {
                iRet = ts_message_dispatcher(pTLV->v, pTLV->l, (void *)pChn);
            }
            else
            {
                SYS_LOG_PRINTF("TS message dispatcher not registered!\n");
                return -1;            
            }
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

void register_xml_msg_dispatcher_to_tlv(PROTO_MSG_DISPATCHER dispatcher)
{
    xml_message_dispatcher = dispatcher;
}

void register_ts_msg_dispatcher_to_tlv(PROTO_MSG_DISPATCHER dispatcher)
{
    ts_message_dispatcher = dispatcher;
}
