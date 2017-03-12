#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"

#include "channel.h"
#include "tlv.h"
#include "sys_conf.h"

static CHANNEL_LIST g_used_channel_list; 
static CHANNEL_LIST g_free_channel_list; 


COMM_CHANNEL *apply_channel()
{
    //pop channel from free list and enqueue it to used channel
    COMM_CHANNEL *pChn = NULL;
    pChn = (COMM_CHANNEL *)_pop_chain((CHAIN_HEAD *)&g_free_channel_list);
    if (pChn == NULL)
    {
        SYS_LOG_PRINTF("No free channel left, hold on for a moment!\n");
        return NULL;
    }
    g_free_channel_list.count --;

    memset(pChn, 0, sizeof(COMM_CHANNEL));
    pChn->bUsed = 1;
    add_channel_tail(&g_used_channel_list, pChn);

    SYS_LOG_PRINTF("Channel free:%d, used:%d\n", 
            g_free_channel_list.count, g_used_channel_list.count);

    return pChn;
}

void release_channel(COMM_CHANNEL *pChn)
{
    //dequeue channel from used list and add it to free list
    int iRet = 0;
    if (pChn == NULL)
    {
        return;
    }

    pthread_mutex_lock(&g_used_channel_list.mutex);
    iRet = _dequeue_node((CHAIN_HEAD *)&g_used_channel_list, (CHAIN_NODE *)pChn);
    if (iRet != 0)
    {
        return;
    }
    g_used_channel_list.count --;
    pthread_mutex_unlock(&g_used_channel_list.mutex);

    add_channel_tail(&g_free_channel_list, pChn);
    pChn->bUsed = 0;

    SYS_LOG_PRINTF("Channel free:%d, used:%d\n", 
            g_free_channel_list.count, g_used_channel_list.count);

    return;
}

int channel_recv_tlv_msg(COMM_CHANNEL *pChn)
{
    S_TLV_MSG *pTLV   = NULL;
    S_TLV_EVENT_NODE *pEvtNode = NULL;
    int       sock    = -1;
    int       len     = 0;
    struct    timeval tv;

    if (pChn == NULL || pChn->sock <= 0)
    {
        return -1;
    }

    pEvtNode = (S_TLV_EVENT_NODE *)malloc(sizeof(S_TLV_EVENT_NODE));
    if (pEvtNode == NULL)
    {
        return -1;
    }
    memset(pEvtNode, 0, sizeof(S_TLV_EVENT_NODE));
    sock = pChn->sock;
    pTLV = (S_TLV_MSG *)pEvtNode->szMsgBuf;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    len = tl_rcv(sock, (char *)&pTLV->t, sizeof(int), &tv);
    if (len < 0)
    {
        return -1;
    }

    len = tl_rcv(sock, (char *)&pTLV->l, sizeof(int), &tv);
    if (len < 0)
    {
        return -1;
    }

    len = tl_rcv(sock, (char *)pTLV->v, pTLV->l, &tv);
    if (len < 0)
    {
        return -1;
    }

    pEvtNode->pChannel = (void *)pChn;
  

    add_event_tail((EVENT_NODE *)pEvtNode);

    return 0;
}

int channel_send_tlv_msg(COMM_CHANNEL *pChn, int type, int length, char *pData)
{
    int sock = 0;
    int len = 0;
    struct timeval tv;
    
    
    if (pChn == NULL || pChn->sock <= 0)
    {
        return -1;
    }
    sock = pChn->sock;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    len = tl_snd(sock, (char *)&type, sizeof(type), &tv);
    if (len != sizeof(type))
    {
        return -1;
    }

    len = tl_snd(sock, (char *)&length, sizeof(length), &tv);
    if (len != sizeof(length))
    {
        return -1;
    }

    len = tl_snd(sock, pData, length, &tv);
    if (len != length)
    {
        return -1;
    }
    
    return 0;
}

void init_channel_list()
{
    COMM_CHANNEL *pChnList = NULL;
    int channel_num = get_channel_num();
    int i = 0;
    
    memset(&g_used_channel_list, 0, sizeof(CHANNEL_LIST));
    memset(&g_free_channel_list, 0, sizeof(CHANNEL_LIST));
    _init_chain((CHAIN_HEAD *)&g_used_channel_list);
    _init_chain((CHAIN_HEAD *)&g_free_channel_list);

    pthread_mutex_init(&g_used_channel_list.mutex, NULL);
    pthread_mutex_init(&g_free_channel_list.mutex, NULL);

    pChnList = (COMM_CHANNEL *)malloc(channel_num * sizeof(COMM_CHANNEL));
    if (pChnList == NULL)
    {
        SYS_LOG_PRINTF("Malloc channel failed, channel count:%d\n", channel_num);
        exit(0);
    }

    for (i=0; i<channel_num; i++)
    {
        pChnList[i].channel_id = i;
        add_channel_tail(&g_free_channel_list, &pChnList[i]);
    }
}

void add_channel_tail(CHANNEL_LIST *pList, COMM_CHANNEL *pChn)
{
    int iRet = 0;

    if (pChn == NULL)
    {
        return;
    }

    pthread_mutex_lock(&pList->mutex);

    iRet = _insert_node_tail((CHAIN_HEAD *)pList, (CHAIN_NODE *)pChn);
    if (iRet == 0)
    {
        pList->count ++;
    }

    pthread_mutex_unlock(&pList->mutex);
}

COMM_CHANNEL *get_used_channel_head()
{
    COMM_CHANNEL *pChn = NULL;
    CHAIN_HEAD *pHead = (CHAIN_HEAD *)&g_used_channel_list;

    pthread_mutex_lock(&g_used_channel_list.mutex);
    pChn =(COMM_CHANNEL *)CHAIN_FIRST(pHead);
    pthread_mutex_unlock(&g_used_channel_list.mutex);

    return pChn;
}

COMM_CHANNEL *get_next_channel(COMM_CHANNEL *pChn)
{
    COMM_CHANNEL *pNextChn = NULL;
    CHAIN_NODE *pNode = (CHAIN_NODE *)pChn;
    if (pNode == NULL)
    {
        return NULL;
    }

    pNextChn = (COMM_CHANNEL *)CHAIN_NEXT(pNode);

    return pNextChn;
}

COMM_CHANNEL *search_channel_via_id(int channel_id)
{
    COMM_CHANNEL *pChannel = NULL;

    pChannel = get_used_channel_head();
    
    while (pChannel != NULL)
    {
        if (pChannel->channel_id == channel_id)
        {
            break;
        }

        pChannel = get_next_channel(pChannel);
    }

    return pChannel;
}


