#ifndef _TLV_H__
#define _TLV_H__

#include "chain.h"

#define M_TLV_BUF_LEN 1024

typedef int (*PROTO_MSG_DISPATCHER)(char *v, int l, void *pChn);

typedef enum
{
    E_PROT_TYPE_XML = 1,
    E_PROT_TYPE_RTP,
    E_PROT_TYPE_TS,
    E_PROT_TYPE_INVD
}E_PROT_TYPE;


typedef struct
{
    int t;
    int l;
    char v[1];
}S_TLV_MSG;

typedef struct
{
    CHAIN_NODE node;
    void       *pChannel;
    char       szMsgBuf[M_TLV_BUF_LEN];
}S_TLV_EVENT_NODE;

typedef struct
{
    void       *pChannel;
    char       szMsgBuf[M_TLV_BUF_LEN];
}S_TLV_EVENT;

void init_tlv_module();

int tlv_msg_dispatcher(void *data);


void register_xml_msg_dispatcher_to_tlv(PROTO_MSG_DISPATCHER dispatcher);
void register_ts_msg_dispatcher_to_tlv(PROTO_MSG_DISPATCHER dispatcher);

#endif
