#ifndef _CHANNEL_H__
#define _CHANNEL_H__

#include <sys/socket.h>
#include <netinet/in.h>

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

typedef enum
{
    E_SERVER_CHANNEL = 0, 
    E_CLIENT_CHANNEL
}E_CHANNEL_TYPE;

typedef struct
{
    CHAIN_HEAD head;
    int count;
    pthread_mutex_t mutex;
}CHANNEL_LIST;

typedef struct
{
    CHAIN_NODE              node;
    pthread_mutex_t         mutex;
    int                              bUsed;
    E_CHANNEL_TYPE        channel_type;
    int                              sock;
    E_CHANNEL_STATE     state;
    int                              channel_id;
    E_PROTO_TYPE           proto_type;
    unsigned int                expire_time;
    int                              peer_port;
    struct sockaddr          peer_addr;
    void                           *video_send_info;
    unsigned int required_seq;
}COMM_CHANNEL;

COMM_CHANNEL *apply_channel();
void expire_channel(COMM_CHANNEL *pChn);
void release_channel(COMM_CHANNEL *pChn);
int channel_recv_tlv_msg(COMM_CHANNEL *pChn);
int channel_send_tlv_msg(COMM_CHANNEL *pChn, int type, int len, char *pData);

void init_channel_list(int chn_num);
void add_channel_tail(CHANNEL_LIST *pList, COMM_CHANNEL *pChn);
COMM_CHANNEL *get_used_channel_head();
COMM_CHANNEL *get_next_channel(COMM_CHANNEL *pChn);
COMM_CHANNEL *search_channel_via_id(int channel_id);
void loop_channel_data(fd_set *rd_set, fd_set *except_set);
void loop_expire_channel();
void update_select_sets(fd_set *rd_set, fd_set *except_set);

#endif
