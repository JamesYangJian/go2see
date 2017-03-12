#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "buf_manage.h"


static S_COMMON_LIST g_free_buf_list;
static BUF_NODE g_buf_node[M_BUF_CNT];

static pthread_mutex_t g_buf_list_mutex;


void module_buf_manage_init()
{
    int i    = 0;
    int iRet = 0;

    memset(&g_free_buf_list, 0, sizeof(g_free_buf_list));
    _init_chain((CHAIN_HEAD *)&g_free_buf_list);

    memset(g_buf_node, 0, sizeof(g_buf_node));
    for(i=0; i< M_BUF_CNT; i++)
    {
        iRet = _insert_node_tail((CHAIN_HEAD *)&g_free_buf_list, (CHAIN_NODE *)&g_buf_node[i]);
        if (iRet == 0)
        {
            g_free_buf_list.count ++;
        }
    }

    pthread_mutex_init(&g_buf_list_mutex, NULL);
}

char* alloc_buf(int len)
{
    BUF_NODE *pNode = NULL;

    if (len > M_BUF_LEN)
    {
        return NULL;
    }

    pthread_mutex_lock(&g_buf_list_mutex);

    pNode = (BUF_NODE *)_pop_chain((CHAIN_HEAD *)&g_free_buf_list);
    if (pNode != NULL)
    {
        g_free_buf_list.count --;
    }

    pthread_mutex_unlock(&g_buf_list_mutex);

    if (pNode == NULL)
    {
        return NULL;
    }
    return (char *)pNode->data;
}

void free_buf(char *data)
{
    int i       = 0;
    int iRet    = 0;
    if (data == NULL)
    {
        return;
    }

    pthread_mutex_lock(&g_buf_list_mutex);

    for(i=0; i<M_BUF_CNT; i++)
    {
        if(data == g_buf_node[i].data)
        {
            iRet = _insert_node_tail((CHAIN_HEAD *)&g_free_buf_list, (CHAIN_NODE *)&g_buf_node[i]);
            if (iRet == 0)
            {
                g_free_buf_list.count ++;
            }
            break;;
        }
    }
    pthread_mutex_unlock(&g_buf_list_mutex);
    return;
}
