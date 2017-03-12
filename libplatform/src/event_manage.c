#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "chain.h"
#include "event_manage.h"
#include "buf_manage.h"
#include "sys_utils.h"

static EVENT_DISPATCHER event_dispatcher = NULL;
static EVENT_LIST       g_event_list;
static pthread_mutex_t  g_event_list_mutex;
static sem_t            g_event_sem;
static pthread_t        g_thread_event_loop;

void module_event_manage_init()
{
    memset(&g_event_list, 0, sizeof(g_event_list));
    _init_chain((CHAIN_HEAD *)&g_event_list);

    pthread_mutex_init(&g_event_list_mutex, NULL);
    sem_init(&g_event_sem, 0, 0);

    pthread_create(&g_thread_event_loop, NULL, event_loop, NULL);
}

EVENT_NODE *pop_event()
{
    EVENT_NODE *pNode = NULL;

    pthread_mutex_lock(&g_event_list_mutex);

    pNode = (EVENT_NODE *)_pop_chain((CHAIN_HEAD *)&g_event_list);
    if (pNode != NULL)
    {
        g_event_list.count -- ;
    }

    pthread_mutex_unlock(&g_event_list_mutex);

    return pNode;
}

void add_event_tail(EVENT_NODE *pnode)
{
    int iRet = 0;
    if (pnode == NULL)
    {
        return;
    }

    pthread_mutex_lock(&g_event_list_mutex);
    iRet = _insert_node_tail((CHAIN_HEAD *)&g_event_list, (CHAIN_NODE *)pnode);
    if (iRet == 0)
    {
        g_event_list.count ++;
    }
    sem_post(&g_event_sem);
    pthread_mutex_unlock(&g_event_list_mutex);
}


void *event_loop(void *argv)
{
    EVENT_NODE *pNode = NULL;
    int        iRet   = 0;

    while(1)
    {
        sem_wait(&g_event_sem);

        while( (pNode = pop_event()) != NULL)
        {
           if (event_dispatcher != NULL)
           {
               iRet = event_dispatcher((void *)pNode->event_data);
               //event has been handled, now free memory
               free_buf((char *)pNode);
           }
           else
           {
                SYS_LOG_PRINTF("event dispatcher is not registered, please check!\n"); 
           }
        }
    }
}

void register_event_dispatcher(EVENT_DISPATCHER dispatcher)
{
    event_dispatcher = dispatcher;
}
