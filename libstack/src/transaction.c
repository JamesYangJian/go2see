#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "chain.h"
#include "sys_utils.h"

#include "transaction.h"

static S_COMMON_LIST g_transaction_list;
static S_COMMON_LIST g_input_req_handler_list;
static S_COMMON_LIST g_input_rep_handler_list;
static S_COMMON_LIST g_output_req_handler_list;
static int g_transaction_id = 0;
static int g_req_seq = 0;

int module_transaction_init()
{
    memset(&g_transaction_list, 0, sizeof(g_transaction_list));
    _init_chain((CHAIN_HEAD *)&g_transaction_list);

    memset(&g_input_req_handler_list, 0, sizeof(g_input_req_handler_list));
    _init_chain((CHAIN_HEAD *)&g_input_req_handler_list);

    memset(&g_input_rep_handler_list, 0, sizeof(g_input_rep_handler_list));
    _init_chain((CHAIN_HEAD *)&g_input_rep_handler_list);    

    memset(&g_output_req_handler_list, 0, sizeof(g_output_req_handler_list));
    _init_chain((CHAIN_HEAD *)&g_output_req_handler_list);

    g_transaction_id = 0;
}

int alloc_cmd_seq()
{
    if (g_req_seq == M_MAX_SEQ)
    {
        return 0;
    }
    return g_req_seq++;
}

void alloc_transaction_id(char *pTransID)
{
    if (pTransID == NULL)
    {
        return;
    }

    if (g_transaction_id == M_MAX_TRANSACTION_ID)
    {
        g_transaction_id = 0;
    }
    else
    {
        g_transaction_id++;
    }

    sprintf(pTransID, "%s-%d", get_device_id(), g_transaction_id);
}

S_TRANSACTION *search_transaction_via_id(char *pID)
{
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    S_TRANSACTION *pTransaction = NULL;

    if (pID == NULL)
    {
        return NULL;
    }

    pHead = (CHAIN_HEAD *)&g_transaction_list;
    pNode = CHAIN_FIRST(pHead);

    while(pNode != NULL)
    {
        pTransaction = (S_TRANSACTION *)pNode;
        if (strcmp(pTransaction->transaction_id, pID) == 0)
        {
            break;
        }

        pNode = CHAIN_NEXT(pNode);
    }

    return pTransaction;
}

S_TRANSACTION *alloc_request_transaction(E_MESSAGE_CODE cmd)
{
    S_TRANSACTION *pTransaction = NULL;
    time_t t;

    pTransaction = (S_TRANSACTION *)malloc(sizeof(S_TRANSACTION));
    if (pTransaction == NULL)
    {
        SYS_LOG_PRINTF("Fatal Error! malloc transaction failed!\n");
        exit(0);
    }
    memset(pTransaction, 0, sizeof(S_TRANSACTION));

    alloc_transaction_id(pTransaction->transaction_id);
    time(&t);
    pTransaction->expire_time = t + M_TRANSACTION_RETRY_INTERVAL;
    pTransaction->state = E_TRANSACTION_UNCONFIRMED;
    pTransaction->transaction_type = E_TRANSACTION_TYPE_REQ;
    pTransaction->transaction_cmd = cmd;
    pTransaction->cmd_seq = alloc_cmd_seq();

    _insert_node_tail((CHAIN_HEAD *)&g_transaction_list, (CHAIN_NODE *)pTransaction);

    return pTransaction;
}

S_TRANSACTION *alloc_response_transaction(E_MESSAGE_CODE cmd, char *pTransID, int seq)
{
    S_TRANSACTION *pTransaction = NULL;
    time_t t;

    if (pTransID == NULL)
    {
        return NULL;
    }

    pTransaction = (S_TRANSACTION *)malloc(sizeof(S_TRANSACTION));
    if (pTransaction == NULL)
    {
        SYS_LOG_PRINTF("Fatal Error! malloc transaction failed!\n");
        exit(0);
    }
    memset(pTransaction, 0, sizeof(S_TRANSACTION));

    strcpy(pTransaction->transaction_id, pTransID);
    time(&t);
    pTransaction->expire_time = t + M_TRANSACTION_RETRY_INTERVAL;
    pTransaction->state = E_TRANSACTION_UNCONFIRMED;
    pTransaction->transaction_type = E_TRANSACTION_TYPE_REP;
    pTransaction->transaction_cmd = cmd;
    pTransaction->cmd_seq = seq;

    _insert_node_tail((CHAIN_HEAD *)&g_transaction_list, (CHAIN_NODE *)pTransaction);

    return pTransaction;
}

void release_transaction(S_TRANSACTION *pTransaction)
{
    if (pTransaction == NULL)
    {
        return;
    }

    _dequeue_node((CHAIN_HEAD *)&g_transaction_list, (CHAIN_NODE *)pTransaction);
    free(pTransaction);
    
    return;
}

void loop_transaction()
{
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    CHAIN_NODE *pDel = NULL;
    S_TRANSACTION *pTransaction = NULL;
    time_t t;

    pHead = (CHAIN_HEAD *)&g_transaction_list;
    pNode = CHAIN_FIRST(pHead);

    time(&t);

    while (pNode != NULL)
    {
        pTransaction = (S_TRANSACTION *)pNode;
        if (pTransaction->state = E_TRANSACTION_CONFIRMED 
            || pTransaction->retry > M_TRANSACTION_MAX_RETRY_TIME
            || pTransaction->pChannel == NULL)
        {
            pDel = pNode;
        }
        else
        {
            if (t > pTransaction->expire_time)
            {
                pTransaction->retry ++;
                channel_send_tlv_msg(pTransaction->pChannel, pTransaction->prot_type, 
                    strlen(pTransaction->szCmdBuf), (char *) pTransaction->szCmdBuf);
            }
        }

        pNode = CHAIN_NEXT(pNode);

        if (pDel != NULL)
        {
            release_transaction((S_TRANSACTION *)pDel);
        }
    }
}

void *serach_handler_node_via_code(S_COMMON_LIST *pList, int cmd_code)
{
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    S_COMM_HANDLER_NODE *pHandlerNode = NULL;
    if (pHead == NULL)
    {
        return NULL;
    }

    pHead = (CHAIN_HEAD *)pList;
    pNode = CHAIN_FIRST(pHead);

    while (pNode != NULL)
    {
        pHandlerNode = (S_COMM_HANDLER_NODE *)pNode;
        if (pHandlerNode->cmd_code == cmd_code)
        {
            break;
        }
    }

    return (void *)pHandlerNode;
}

void register_input_req_msg_handler(int cmd_code, input_req_msg_handler handler)
{
    INPUT_REQ_HANDLER_NODE *pInputReqNode = NULL;
    if (handler == NULL)
    {
        return;
    }

    pInputReqNode = (INPUT_REQ_HANDLER_NODE *)malloc(sizeof(INPUT_REQ_HANDLER_NODE));
    if (pInputReqNode == NULL)
    {
        SYS_LOG_PRINTF("Fatal Error! Malloc input req msg handler node failed!\n");
        exit(0);
    }
    pInputReqNode->node.cmd_code = cmd_code;
    pInputReqNode->handler = handler;

    _insert_node_tail((CHAIN_HEAD *)&g_input_req_handler_list, (CHAIN_NODE *)pInputReqNode);

    return;
}

void register_input_rep_msg_handler(int cmd_code, input_rep_msg_handler handler)
{
    INPUT_REP_HANDLER_NODE *pInputRepNode = NULL;
    if (handler == NULL)
    {
        return;
    }

    pInputRepNode = (INPUT_REP_HANDLER_NODE *)malloc(sizeof(INPUT_REP_HANDLER_NODE));
    if (pInputRepNode == NULL)
    {
        SYS_LOG_PRINTF("Fatal Error! Malloc input rep msg handler node failed!\n");
        exit(0);
    }
    pInputRepNode->node.cmd_code = cmd_code;
    pInputRepNode->handler = handler;

    _insert_node_tail((CHAIN_HEAD *)&g_input_rep_handler_list, (CHAIN_NODE *)pInputRepNode);

    return;
}

void register_output_msg_handler(int cmd_code, output_req_msg_handler handler)
{
    OUTPUT_REQ_HANDLER_NODE *pOutputNode = NULL;
    if (handler == NULL)
    {
        return;
    }

    pOutputNode = (OUTPUT_REQ_HANDLER_NODE *)malloc(sizeof(OUTPUT_REQ_HANDLER_NODE));
    if (pOutputNode == NULL)
    {
        SYS_LOG_PRINTF("Fatal Error! Malloc input msg handler node failed!\n");
        exit(0);
    }
    pOutputNode->node.cmd_code = cmd_code;
    pOutputNode->handler = handler;

    _insert_node_tail((CHAIN_HEAD *)&g_output_req_handler_list, (CHAIN_NODE *)pOutputNode);

    return;
}

int transaction_send_output_request(S_TRANSACTION *pTrans, void *pParam)
{
    char *pBuf = NULL;
    OUTPUT_REQ_HANDLER_NODE *pOutputNode = NULL;

    if (pTrans == NULL)
    {
        SYS_LOG_PRINTF("Can't process a NULL transaction!\n");
        return -1;
    }

    pOutputNode = (OUTPUT_REQ_HANDLER_NODE *)serach_handler_node_via_code(&g_output_req_handler_list, pTrans->transaction_cmd);
    if (pOutputNode == NULL)
    {
        SYS_LOG_PRINTF("Transaction can't process a unkonwn output request command :%d\n", pTrans->transaction_cmd);
        return -1;
    }

    pBuf = (char *)pTrans->szCmdBuf;

    sprintf(pBuf, M_MESSAGE_HEAD_FMT, E_TRANSACTION_TYPE_REQ, E_DEVICE_REGISTER_CODE, pTrans->cmd_seq, pTrans->transaction_id);
    pBuf += strlen(pBuf);
    pOutputNode->handler(pParam, pBuf);
    sprintf(pBuf+strlen(pBuf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pTrans->pChannel, E_PROT_TYPE_XML, strlen(pTrans->szCmdBuf), (char *)pTrans->szCmdBuf);
    
    return 0;
}

int transaction_handle_input_request(S_TRANSACTION *pTrans, ezxml_t pxml)
{
    char *pBuf = NULL;
    INPUT_REQ_HANDLER_NODE *pInputNode = NULL;

    if (pTrans == NULL)
    {
        SYS_LOG_PRINTF("Can't process a NULL transaction!\n");
        return -1;
    }    

    pInputNode = (INPUT_REQ_HANDLER_NODE *)serach_handler_node_via_code(&g_input_req_handler_list, pTrans->transaction_cmd);
    if (pInputNode == NULL)
    {
        SYS_LOG_PRINTF("Transaction can't process a unkonwn input request command :%d\n", pTrans->transaction_cmd);
        return -1;
    }
    
    pBuf = (char *)pTrans->szCmdBuf;
    sprintf(pBuf, M_MESSAGE_HEAD_FMT, E_TRANSACTION_TYPE_REP, pTrans->transaction_cmd+1,  pTrans->cmd_seq, pTrans->transaction_id);
    pBuf += strlen(pBuf);
    pInputNode->handler(pxml, pBuf);
    sprintf(pBuf+strlen(pBuf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pTrans->pChannel, E_PROT_TYPE_XML, strlen(pTrans->szCmdBuf), (char *)pTrans->szCmdBuf);
    
    return 0;
}

int transaction_handle_input_response(S_TRANSACTION *pTrans, ezxml_t pxml)
{
    INPUT_REP_HANDLER_NODE *pInputRepNode = NULL;

    if (pTrans == NULL)
    {
        SYS_LOG_PRINTF("Can't process a NULL transaction!\n");
        return -1;
    }

    pInputRepNode = (INPUT_REP_HANDLER_NODE *)serach_handler_node_via_code(&g_input_rep_handler_list, pTrans->transaction_cmd);
    if (pInputRepNode == NULL)
    {
        SYS_LOG_PRINTF("Transaction can't process a unkonwn input request command :%d\n", pTrans->transaction_cmd);
        return -1;
    }

    pInputRepNode->handler(pxml, pTrans);
    
    return 0;
}

