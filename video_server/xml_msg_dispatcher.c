#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "ezxml.h"
#include "sys_utils.h"

#include "xml_msg_dispatcher.h"
#include "channel.h"
#include "xml_msg_handler.h"
#include "msg_define.h"

static S_XML_MSG_DISPATCHER dispatcher_list[] = 
{
    {E_PROGRAM_REQUEST_NOTIFY_CODE, program_request_notify_handler},
    {E_PROGRAM_REQUEST_CODE, program_request_handler},
};

int xml_msg_dispatcher(char *pData, int len, void *pChannel)
{
    ezxml_t pxml      = NULL;
    ezxml_t pChildXml = NULL;
    char *pTmp        = NULL;
    int cmd = 0;
    int seq = 0;
    int i   = 0;
    int num = sizeof(dispatcher_list)/sizeof(S_XML_MSG_DISPATCHER);
    COMM_CHANNEL *pChn = (COMM_CHANNEL *)pChannel;

    if (pData == NULL || pChn == NULL)
    {
        return -1;
    }

    pxml = ezxml_parse_str(pData, len);
    if (pxml == NULL)
    {
        return -1;
    }

    pTmp = ezxml_get_child_txt(pxml, "cmd");
    if (pTmp == NULL)
    {
        ezxml_free(pxml);
        return -1;
    }
    cmd = atoi(pTmp);

    pTmp = ezxml_get_child_txt(pxml, "seq");
    if (pTmp == NULL)
    {
        ezxml_free(pxml);
        return -1;
    }
    seq = atoi(pTmp);

    if (seq < pChn->required_seq)
    {
        SYS_LOG_PRINTF("This message has already been processed!\n");
        //do default handler
        ezxml_free(pxml);
        return 0;
    }

    if (seq == M_MAX_SEQ)
    {
        pChn->required_seq = 0;
    }
    else
    {
        pChn->required_seq = seq + 1;
    }
    pChildXml = ezxml_child(pxml, "param");
    if (pChildXml == NULL)
    {
        ezxml_free(pxml);
        return -1;
    }

    for (i=0; i<num; i++)
    {
        if (cmd == dispatcher_list[i].cmd)
        {
            dispatcher_list[i].dispatcher(pChn, pChildXml);
        }
    }

    if (i > num)
    {
        SYS_LOG_PRINTF("Unkown command:%d\n", cmd);
        // do default handler for the command
        ezxml_free(pxml);
        return -1;
    }

    if (pxml != NULL)
    {
        ezxml_free(pxml);
        pxml = NULL;
    }
    
    return 0;
}
