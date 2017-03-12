#ifndef _PROGRAM_MANAGE_H__
#define _PROGRAM_MANAGE_H__

#include "chain.h"
#include "data_presentation.h"
#include "channel.h"

#define M_RTP_PKT_LEN               800
#define M_TS_PKT_LEN                188

#define M_AUTH_STR_LEN                  128
#define M_PROTOTYPE_STR_LEN         16

#define M_MIN_VIDEO_PORT              8000
#define M_MAX_VIDEO_PORT             60000

#define M_PG_REQ_EXPIRE_INTERVAL   10

typedef struct
{
    CHAIN_NODE node;
    S_PROGRAM_INFO stProgramInfo;
}S_PROGRAM_INFO_NODE;

typedef struct
{
    int video_port;
    char szAuth[M_AUTH_STR_LEN+1];
    char szProtoType[M_PROTOTYPE_STR_LEN+1];
    S_PROGRAM_INFO stProgramInfo;
    FILE *fp;
}S_PROGRAM_TRANSFER_INFO;


int find_program_list_via_dev_id(S_COMMON_LIST *list_head, char *pDevID);
S_PROGRAM_INFO *find_program_record(int program_id, int *record_num);
int program_exist(int program_id);

int alloc_video_port();
int gene_video_req_auth(char *pDevID, int program_id, char *pAuth);

void module_program_manage_init();
void add_send_channel(COMM_CHANNEL *pChn);
void remvoe_send_channel(COMM_CHANNEL *pChn);

void *loop_send_program_data(void *arg);


#endif
