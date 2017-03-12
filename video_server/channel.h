#ifndef _CHANNEL_H__
#define _CHANNEL_H__

#include "event_manage.h"

typedef enum
{
    E_STATE_BEGIN = 0,
    E_STATE_REQ_SENT
}E_CHANNEL_STATE;

typedef enum
{
    E_PROTO_TYPE_TCP = 0,
    E_PROTO_TYPE_UDP
}E_PROTO_TYPE;

typedef struct
{
    CHAIN_HEAD head;
    int count;
    pthread_mutex_t mutex;
}CHANNEL_LIST;

typedef struct
{
    CHAIN_NODE node;
    int bUsed;
    int sock;
    unsigned int required_seq;
    E_CHANNEL_STATE state;
    int channel_id;
    E_PROTO_TYPE proto_type;
    unsigned int expire_time;
    void *video_send_info;
    int peer_port;
}COMM_CHANNEL;

COMM_CHANNEL *apply_channel();
void release_channel(COMM_CHANNEL *pChn);
int channel_recv_tlv_msg(COMM_CHANNEL *pChn);
int channel_send_tlv_msg(COMM_CHANNEL *pChn, int type, int len, char *pData);

void init_channel_list();
void add_channel_tail(CHANNEL_LIST *pList, COMM_CHANNEL *pChn);
COMM_CHANNEL *get_used_channel_head();
COMM_CHANNEL *get_next_channel(COMM_CHANNEL *pChn);
COMM_CHANNEL *search_channel_via_id(int channel_id);

#endif
