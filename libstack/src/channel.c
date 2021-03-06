#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"
#include "channel.h"
#include "tlv.h"
//#include "sys_conf.h"

static CHANNEL_LIST g_used_channel_list; 
static CHANNEL_LIST g_free_channel_list; 
static int g_channel_num = 0;


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

void expire_channel(COMM_CHANNEL *pChn)
{
    if (pChn != NULL)
    {
        pChn->expire_time = 0;
    }
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

    if (pChn->proto_type == E_PROTO_TYPE_TCP)
    {
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
    }
    else
    {
        //for udp, receive data once
        len = tl_rcv_udp(sock, (char *)pTLV, M_TLV_BUF_LEN, &tv, &pChn->peer_addr);
        if (len <= 0)
        {
            return -1;
        }
    }

    pEvtNode->pChannel = (void *)pChn;
    pChn->expire_time = 0xffffffff;

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

    if (pChn->proto_type == E_PROTO_TYPE_TCP)
    {
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
    }
    else
    {
        //Send udp data
        char szBuf[M_TLV_BUF_LEN];
        int send_len = 0;

        memset(szBuf, 0, M_TLV_BUF_LEN);
        memcpy(szBuf, &type, sizeof(type));
        send_len += sizeof(type);
        memcpy(szBuf+send_len, &length, sizeof(length));
        send_len += sizeof(length);
        memcpy(szBuf+send_len, pData, length);
        send_len += length;

        len = tl_snd_udp(sock, (unsigned char *)szBuf, send_len, &tv, &pChn->peer_addr);
        if (len != send_len)
        {
            return -1;
        }
    }
    
    return 0;
}

void init_channel_list(int chn_num)
{
    COMM_CHANNEL *pChnList = NULL;
    g_channel_num = chn_num;
    int i = 0;
    
    memset(&g_used_channel_list, 0, sizeof(CHANNEL_LIST));
    memset(&g_free_channel_list, 0, sizeof(CHANNEL_LIST));
    _init_chain((CHAIN_HEAD *)&g_used_channel_list);
    _init_chain((CHAIN_HEAD *)&g_free_channel_list);

    pthread_mutex_init(&g_used_channel_list.mutex, NULL);
    pthread_mutex_init(&g_free_channel_list.mutex, NULL);

    pChnList = (COMM_CHANNEL *)malloc(g_channel_num * sizeof(COMM_CHANNEL));
    if (pChnList == NULL)
    {
        SYS_LOG_PRINTF("Malloc channel failed, channel count:%d\n", g_channel_num);
        exit(0);
    }

    for (i=0; i<g_channel_num; i++)
    {
        pChnList[i].channel_id = i;
        pthread_mutex_init(pChnList[i].mutex);
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

void update_select_sets(fd_set *rd_set, fd_set *except_set)
{
    COMM_CHANNEL *pChn = get_used_channel_head();

    if (rd_set == NULL)
    {
        return;
    }

    FD_ZERO(rd_set);
    FD_ZERO(except_set);

    while(pChn != NULL)
    {
        FD_SET(pChn->sock, rd_set);
        FD_SET(pChn->sock, except_set);
        pChn = get_next_channel(pChn);
    }
}

void loop_channel_data(fd_set *rd_set, fd_set *except_set)
{
    COMM_CHANNEL *pChn = get_used_channel_head();
    COMM_CHANNEL *pTmpChn = NULL;
    int iRet = 0;
    time_t t;

    if (rd_set == NULL)
    {
        return;
    }        

    while (pChn != NULL)
    {
        time(&t);
        if (pChn->expire_time < t)
        {
            SYS_LOG_PRINTF("Channel expires!\n");
            pTmpChn = pChn;        
        }
        else if (FD_ISSET(pChn->sock, except_set))
        {
            SYS_LOG_PRINTF("Channel exception, sock:%d!\n", pChn->sock);
            pTmpChn = pChn;
        }
        else if (FD_ISSET(pChn->sock, rd_set))
        {
            SYS_LOG_PRINTF("Begin to receive channel data!\n");
            iRet = channel_recv_tlv_msg(pChn);
            if (iRet < 0)
            {
                pTmpChn = pChn;
            }
        }

        pChn = get_next_channel(pChn);
        if (pTmpChn != NULL)
        {
            SYS_LOG_PRINTF("dequeue unused channel!\n");
            tl_destroy(pTmpChn->sock);
            release_channel(pTmpChn);
            pTmpChn = NULL;
        }
    }
}

void loop_expire_channel()
{
    COMM_CHANNEL *pChn = get_used_channel_head();
    COMM_CHANNEL *pTmpChn = NULL;
    time_t t;
    time(&t);
    
    while (pChn != NULL)
    {
        
        if (pChn->expire_time < t)
        {
            SYS_LOG_PRINTF("Channel expires!\n");
            pTmpChn = pChn;        
        }

        pChn = get_next_channel(pChn);
        if (pTmpChn != NULL)
        {
            SYS_LOG_PRINTF("dequeue unused channel!\n");
            tl_destroy(pTmpChn->sock);
            release_channel(pTmpChn);
            pTmpChn = NULL;
        }
    }
}


