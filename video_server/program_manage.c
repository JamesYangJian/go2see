#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "md5.h"
#include "base64.h"
#include "sys_utils.h"
#include "program_manage.h"
#include "channel.h"
#include "tlv.h"
#include "chain.h"

static S_COMMON_LIST g_send_channel_list;
static int g_video_port = M_MIN_VIDEO_PORT;

static pthread_t g_send_data_thread;
static sem_t      g_enqueue_sem;


typedef struct
{
    struct timeval tv;
    unsigned int counter;
}S_RANDOM_SEED;

static S_RANDOM_SEED g_seed;

void module_program_manage_init()
{
    _init_chain((CHAIN_HEAD *)&g_send_channel_list);

    memset(&g_seed, 0, sizeof(S_RANDOM_SEED));

    sem_init(&g_enqueue_sem, 0, 0);
}

void add_send_channel(COMM_CHANNEL *pChn)
{
    if (pChn != NULL)
    {
        _insert_node_tail((CHAIN_HEAD *)&g_send_channel_list, (CHAIN_NODE *)pChn);
        sem_post(&g_enqueue_sem);
    }
}

void remvoe_send_channel(COMM_CHANNEL *pChn)
{
    if (pChn != NULL)
    {
        _dequeue_node((CHAIN_HEAD *)&g_send_channel_list, (CHAIN_NODE *)pChn);
    }
}

int find_program_list_via_dev_id(S_COMMON_LIST *list_head, char *pDevID)
{
    int *pGroupIDList = NULL;
    int *pPGIDList = NULL;
    int iGroupIDRecordNum = 0;
    int iProgramIDRecordNum = 0;
    int iProgramRecordNum = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int group_id = 0;
    int program_id = 0;
    int program_count = 0;
    S_PROGRAM_INFO_NODE *pPGInfoNode = NULL;
    S_PROGRAM_INFO *pPGInfo = NULL;
    
    if (list_head == NULL || pDevID == NULL)
    {
        return 0;
    }

    //1. find group via device id
    pGroupIDList = query_device_groups(pDevID, &iGroupIDRecordNum);
    if (pGroupIDList == NULL)
    {
        return 0;
    }

    //2. find program via groups
    for (i=0; i<iGroupIDRecordNum; i++)
    {
        group_id = pGroupIDList[i];
        pPGIDList = query_group_programs(group_id, &iProgramIDRecordNum);
        if (pPGIDList != NULL)
        {
            for(j=0; j<iProgramIDRecordNum; j++)
            {
                program_id = pPGIDList[j];
                pPGInfo = query_program_info_by_id(program_id, &iProgramRecordNum);
                if (pPGInfo != NULL)
                {
                    pPGInfoNode = (S_PROGRAM_INFO_NODE *)malloc(iProgramRecordNum * sizeof(S_PROGRAM_INFO_NODE));
                    memset(pPGInfoNode, 0, iProgramRecordNum * sizeof(S_PROGRAM_INFO_NODE));
                    for (k=0; k<iProgramRecordNum; k++)
                    {
                        memcpy(&pPGInfoNode[k].stProgramInfo, &pPGInfo[k], sizeof(S_PROGRAM_INFO));
                        _insert_node_tail((CHAIN_HEAD *)list_head, (CHAIN_NODE *)(&pPGInfoNode[k]));
                        list_head->count ++;
                    }
                }
            }
        }
    }

    if (pGroupIDList != NULL)
    {
        free(pGroupIDList);
        pGroupIDList = NULL;
    }

    if (pPGIDList != NULL)
    {
        free(pPGIDList);
        pPGIDList = NULL;
    }

    if (pPGInfo != NULL)
    {
        free(pPGInfo);
        pPGInfo = NULL;
    }

    return list_head->count;
}

int program_exist(int program_id)
{
    int record_num = 0;
    S_PROGRAM_INFO *pProgram = NULL;
    
    pProgram = query_program_info_by_id(program_id, &record_num);

    if (pProgram != NULL)
    {
        free(pProgram);
        pProgram = NULL;
        return 1;
    }

    return 0;
}

S_PROGRAM_INFO *find_program_record(int program_id, int *record_num)
{
    S_PROGRAM_INFO *pProgram = NULL;
    
    pProgram = query_program_info_by_id(program_id, record_num);

    return pProgram;
}

int alloc_video_port()
{
    int port = 0;
    
    port = g_video_port;
    if (g_video_port == M_MAX_VIDEO_PORT)
    {
        g_video_port = M_MIN_VIDEO_PORT;
    }
    else
    {
        g_video_port ++;
    }

    return port;
}

int gene_video_req_auth(char *pDevID, int program_id, char *pAuth)
{
   unsigned char szTmp[16];
    struct MD5Context ctx;
    
    if (pDevID == NULL || pAuth == NULL)
    {
        return -1;
    }

    memset(szTmp, 0, sizeof(szTmp));

    gettimeofday(&g_seed.tv, NULL);
    g_seed.counter ++;

    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char *) pDevID, strlen(pDevID));
    MD5Update(&ctx, (unsigned char *) &program_id, sizeof(int));
    MD5Update(&ctx, (unsigned char *) &g_seed, sizeof(S_RANDOM_SEED));
    MD5Final(szTmp, &ctx);

    base64encode(szTmp, pAuth, sizeof(szTmp));

    return 0;
}

void *loop_send_program_data(void *arg)
{
    COMM_CHANNEL *pSendChn = NULL;
    COMM_CHANNEL *pFinishChn = NULL;
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    char szSendBuf[M_TS_PKT_LEN];
    int len = 0;
    S_PROGRAM_TRANSFER_INFO *pPGTransInfo = NULL;

    pHead = (CHAIN_HEAD *)&g_send_channel_list;

    while(1)
    {
        sem_wait(&g_enqueue_sem);
        pSendChn = (COMM_CHANNEL *)CHAIN_FIRST(pHead);
        while(pSendChn != NULL)
        {
            //Send channel data
            pPGTransInfo = (S_PROGRAM_TRANSFER_INFO *)pSendChn->video_send_info;
            len = fread(pPGTransInfo->fp, szSendBuf, 1,M_TS_PKT_LEN);
            if (len != M_TS_PKT_LEN)
            {
                //reach the file end, send finish message and remove this channel from send list
                pFinishChn = pSendChn;
            }
            else
            {
                channel_send_tlv_msg(pSendChn, E_PROT_TYPE_TS, len, szSendBuf);
            }
 

            pNode = (CHAIN_NODE *)pSendChn;
            pSendChn = (COMM_CHANNEL *)CHAIN_NEXT(pNode);

            if (pFinishChn != NULL)
            {
                //Todo: add below function remove_send_channel
                //remove_send_channel(pFinishChn);
                fclose(pPGTransInfo->fp);
                free(pPGTransInfo);
                release_channel(pFinishChn);
                tl_destroy(pFinishChn->sock);
                pFinishChn = NULL;
                
            }
            
            if (pSendChn == NULL)
            {
                pSendChn = (COMM_CHANNEL *)CHAIN_FIRST(pHead);
                if (pSendChn)
                {
                    //No waiting channel, break to wait new semaphore
                    break;
                }
            }
        }
    }
}
