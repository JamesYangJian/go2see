#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"
#include "chain.h"
#include "online_dev_manage.h"

static S_COMMON_LIST online_device_list;

void module_online_dev_mange_init()
{
    memset(&online_device_list, 0, sizeof(S_COMMON_LIST));
    _init_chain((CHAIN_HEAD * )&online_device_list);
}

int add_online_device(S_ONLINE_DEVICE *pOnlineDev)
{
    S_ONLINE_DEVICE *pFoundonlineDev = NULL;
    S_ONLINE_DEV_NODE *pOnlineDevNode = NULL;
    if (pOnlineDev == NULL)
    {
        return 0;
    }

    pFoundonlineDev = find_online_device(pOnlineDev->szDevID);
    if (pFoundonlineDev != NULL)
    {
        //device record has already existed
        memcpy(pFoundonlineDev, pOnlineDev, sizeof(S_ONLINE_DEVICE));
        update_online_device_record(pOnlineDev);
    }
    else
    {
        //it is a new record
        pOnlineDevNode = (S_ONLINE_DEV_NODE *)malloc(sizeof(S_ONLINE_DEV_NODE));
        if (pOnlineDevNode == NULL)
        {
            SYS_LOG_PRINTF("Malloc online device node failed!\n");
            exit(0);
        }

        memset(pOnlineDevNode, 0, sizeof(S_ONLINE_DEV_NODE));
        memcpy(&pOnlineDevNode->stOnlineDev, pOnlineDev, sizeof(S_ONLINE_DEVICE));
        _insert_node_tail((CHAIN_HEAD * )&online_device_list, (CHAIN_NODE * )pOnlineDevNode);
        online_device_list.count ++;
        insert_online_device_record(pOnlineDev);
    }
    return 0;
}

S_ONLINE_DEVICE *find_online_device(char *pDevID)
{
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    S_ONLINE_DEV_NODE *pOnLineDevNode = NULL;
    
    if (pDevID == NULL)
    {
        return NULL;
    }

    pHead = (CHAIN_HEAD *)&online_device_list;
    pNode = CHAIN_FIRST(pHead);

    while(pNode != NULL)
    {
        pOnLineDevNode = (S_ONLINE_DEV_NODE*)pNode;
        if (strcmp(pOnLineDevNode->stOnlineDev.szDevID, pDevID) == 0)
        {
            break;
        }
        pNode = CHAIN_NEXT(pNode);
    }

    if (pNode == NULL)
    {
        return NULL;
    }
    else
    {
        return &pOnLineDevNode->stOnlineDev;
    }
        
}
