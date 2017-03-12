#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "sys_utils.h"
#include "chain.h"

#include "channel.h"
#include "xml_msg_handler.h"
#include "msg_define.h"
#include "online_dev_manage.h"
#include "tlv.h"
#include "program_manage.h"

int register_handler(void *pChannel, ezxml_t pxml)
{
    char buf[256];
    COMM_CHANNEL *pChn = (COMM_CHANNEL *)pChannel;
    char szDevID[M_DEV_ID_LEN+1];
    char szDevPass[M_DEV_PASS_LEN+1];
    char szIPAddr[M_IPADDR_LEN];
    int iPort = 0;
    char *pTmp = NULL;
    int iRegRet = M_REG_RET_SUCCEED;
    char szReason[24];
    S_DEVICE_INFO *pDevInfo = NULL;
    int dev_record_num = 0;
    int i = 0;
    char *pDateTime = NULL;
    time_t t;
    
    SYS_LOG_PRINTF("Handling register message!\n");

    if (pChn == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for register_handler!\n");
        return -1;
    }

    memset(szDevID, 0, sizeof(szDevID));
    memset(szDevPass, 0, sizeof(szDevPass));
    memset(szIPAddr, 0, sizeof(szIPAddr));
    strcpy(szReason, "succeed!");

    pTmp = ezxml_get_child_txt(pxml, "dev_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("register failed, dev_id is NULL\n");
        iRegRet = M_REG_RET_FAIL_NOT_EXIST;
        strcpy(szReason, "NULL device!");
    }
    else
    {
        strncpy(szDevID, pTmp, M_DEV_ID_LEN);
    }

    if(iRegRet == M_REG_RET_SUCCEED)
    {
        pTmp = ezxml_get_child_txt(pxml, "dev_pass");
        if (pTmp != NULL)
        {
            strncpy(szDevPass, pTmp, M_DEV_PASS_LEN);
        }

        pTmp = ezxml_get_child_txt(pxml, "ip_addr");
        if (pTmp != NULL)
        {
            strncpy(szIPAddr, pTmp, M_IPADDR_LEN-1);
        }

        pTmp = ezxml_get_child_txt(pxml, "video_port");
        if (pTmp != NULL)
        {
            iPort = atoi(pTmp);
        }

        pDevInfo = query_dev_info_by_id(szDevID, &dev_record_num);
        if (pDevInfo != NULL)
        {
            if (dev_record_num == 1)
            {
                SYS_LOG_PRINTF("-----------Query device information as follows:---------!\n");
                SYS_LOG_PRINTF("%s %s %s %d\n", pDevInfo->szDevID, pDevInfo->szDevPass,
                    pDevInfo->szDescription, pDevInfo->iType);

                if (strcmp(pDevInfo->szDevPass, szDevPass) != 0)
                {
                    SYS_LOG_PRINTF("register failed, device authentication failed\n");
                    iRegRet = M_REG_RET_FAIL_AUTH;
                    strcpy(szReason, "Authentication failed!");
                }
                else
                {
                    //Register OK
                    SYS_LOG_PRINTF("Register successfully!\n");
                    S_ONLINE_DEVICE stOnlineDev;
                    memset(&stOnlineDev, 0, sizeof(S_ONLINE_DEVICE));
                    strncpy(stOnlineDev.szDevID, szDevID, M_DEV_ID_LEN);
                    strcpy(stOnlineDev.szIPAddr, szIPAddr);
                    time(&t);
                    t += M_REGISTER_INTERVAL_IN_SEC;
                    stOnlineDev.valid_time= t;
                    
                    add_online_device(&stOnlineDev);
                }
            }
            else
            {
                    SYS_LOG_PRINTF("register failed, more device records found\n");
                    iRegRet = M_REG_RET_FAIL_AUTH;
                    strcpy(szReason, "Server internal error!");                
            }
            free((void *)pDevInfo);
        }
        else
        {
            SYS_LOG_PRINTF("register failed, device not found\n");
            iRegRet = M_REG_RET_FAIL_NOT_EXIST;
            strcpy(szReason, "Device not found!");
        }
    }
    

    pDateTime = current_date_time();
    sprintf(buf, M_MESSAGE_HEAD_FMT, E_REGISTER_ACK_CODE, pChn->required_seq);
    sprintf(buf+strlen(buf), M_REG_ACK_MSG_FMT, iRegRet, szReason, M_REGISTER_INTERVAL_IN_SEC, pDateTime);
    sprintf(buf+strlen(buf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pChn, E_PROT_TYPE_XML, strlen(buf), buf);

    return 0;
}

int program_info_request_handler(void *pChannel, ezxml_t pxml)
{
    char buf[256];
    COMM_CHANNEL *pChn = (COMM_CHANNEL *)pChannel;
    char *pDevID = NULL;
    S_ONLINE_DEVICE *pOnlineDev = NULL;
    int ret = M_PG_INFO_RET_SUCCEED;
    int program_count = 0;
    S_COMMON_LIST stProgramList;
    S_PROGRAM_INFO_NODE *pPGNode = NULL;
    CHAIN_HEAD *pHead = NULL;
    CHAIN_NODE *pNode = NULL;
    

    if (pChannel == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for program_info_request_handler!\n ");
        return -1;
    }

    pDevID = ezxml_get_child_txt(pxml, "dev_id");
    if (pDevID == NULL)
    {
        SYS_LOG_PRINTF("Error! Can't find a NULL device's program info!\n");
        return -1;
    }

    pOnlineDev = find_online_device(pDevID);
    if (pOnlineDev == NULL)
    {
        ret = M_PG_INFO_RET_FAIL_NOT_ONLINE;
    }

    if (ret == M_PG_INFO_RET_SUCCEED)
    {
        memset(&stProgramList, 0, sizeof(S_COMMON_LIST));
        _init_chain((CHAIN_HEAD *)&stProgramList);
        program_count = find_program_list_via_dev_id(&stProgramList, pDevID);
    }

    sprintf(buf, M_MESSAGE_HEAD_FMT, E_PROGRAM_INFO_ACK_CODE, pChn->required_seq);
    sprintf(buf+strlen(buf), M_PGM_INFO_HEAD_MSG_FMT, ret, program_count);
    if (program_count > 0)
    {
        pHead = (CHAIN_HEAD *)&stProgramList;
        pNode = CHAIN_FIRST(pHead);
        while(pNode != NULL)
        {
            pPGNode = (S_PROGRAM_INFO_NODE *)pNode;
            sprintf(buf+strlen(buf), M_PGM_INFO_MSG_FMT, 
                pPGNode->stProgramInfo.program_id, pPGNode->stProgramInfo.szProgramDesc);
            _dequeue_node(pHead, pNode);
            free(pNode);
            pNode = CHAIN_FIRST(pHead);
        }
    }
    sprintf(buf+strlen(buf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pChn, E_PROT_TYPE_XML, strlen(buf), buf);
    
    return 0;
}

int program_request_handler(void *pChannel, ezxml_t pxml)
{
    char buf[256];
    COMM_CHANNEL *pChn = (COMM_CHANNEL *)pChannel;
    COMM_CHANNEL *pVideoChannel = NULL;
    COMM_CHANNEL *pSendChannel = NULL;
    char *pDevID = NULL;
    char *pTmp = NULL;;
    S_ONLINE_DEVICE *pOnlineDev = NULL;
    int ret = M_PG_REQ_RET_SUCCEED;
    int program_id = 0;
    unsigned int seq = 0;
    

    if (pChannel == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for program_info_request_handler!\n ");
        return -1;
    }

    pDevID = ezxml_get_child_txt(pxml, "dev_id");
    if (pDevID == NULL)
    {
        SYS_LOG_PRINTF("Error! NULL device can't do any operation!\n");
        return -1;
    }
    pTmp = ezxml_get_child_txt(pxml, "program_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Error! You can't request a NULL program!\n");
        return -1;
    }
    program_id = atoi(pTmp);

    pOnlineDev = find_online_device(pDevID);
    if (pOnlineDev == NULL)
    {
        ret = M_PG_REQ_RET_FAIL_NOT_ONLINE;
    }

    if (ret == M_PG_REQ_RET_SUCCEED)
    {
        if (program_exist(program_id) == 0)
        {
            ret = M_PG_REQ_RET_FAIL_PG_NOT_EXIST;
        }
    }

    if (ret == M_PG_REQ_RET_SUCCEED)
    {
        pVideoChannel = get_video_server_channel();
        if (pVideoChannel == NULL || pVideoChannel->bUsed == 0)
        {
            ret = M_PG_REQ_RET_FAIL_SVR_ERROR;
        }
    }

    if (pChn->required_seq == 0)
    {
        seq = M_MAX_SEQ;
    }
    seq = pChn->required_seq - 1;

    if (ret == M_PG_REQ_RET_SUCCEED)
    {
        //Send request to video server
        sprintf(buf, M_MESSAGE_HEAD_FMT, E_PROGRAM_REQUEST_NOTIFY_CODE, seq);
        sprintf(buf+strlen(buf), M_PGM_REQ_NOITFY_MSG_FMT, pDevID, program_id, pChn->channel_id);
        pSendChannel = pVideoChannel;
    }
    else
    {
        //Send response to device
        sprintf(buf, M_MESSAGE_HEAD_FMT, E_PROGRAM_REQUEST_ACK_CODE, seq);
        sprintf(buf+strlen(buf), M_PGM_REQ_ACK_MSG_FMT, ret, "", "", "");
        pSendChannel = pChn;
    }
    sprintf(buf+strlen(buf), M_MESSAGE_TAIL_FMT);
    channel_send_tlv_msg(pSendChannel, E_PROT_TYPE_XML, strlen(buf), buf);

    return 0;
}

int program_request_notify_ack_handler(void *pChannel, ezxml_t pxml)
{
    char *pTmp = NULL;
    int result = 0;
    char *pAuth = NULL;
    char *pVideoSvrInfo = NULL;
    char *pStreamInfo = NULL;
    int iChannel_id = 0;
    char buf[256];
    COMM_CHANNEL *pSendChannel = NULL;
    COMM_CHANNEL *pChn = NULL;
    int seq = 0;

    if (pChannel == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for program_request_notify_ack_handler!\n ");
        return -1;
    }
    pChn = (COMM_CHANNEL *)pChannel;

    pTmp = ezxml_get_child_txt(pxml, "result");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Video server send a wrong format ack!\n");
        return -1;
    }
    result = atoi(pTmp);

    pTmp = ezxml_get_child_txt(pxml, "auth");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Video server send a wrong format ack!\n");
        return -1;
    }
    pAuth = pTmp;

    pTmp = ezxml_get_child_txt(pxml, "video_server_info");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Video server send a wrong format ack!\n");
        return -1;
    }
    pVideoSvrInfo = pTmp;

    pTmp = ezxml_get_child_txt(pxml, "stream_info");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Video server send a wrong format ack!\n");
        return -1;
    }
    pStreamInfo = pTmp;

    pTmp = ezxml_get_child_txt(pxml, "channel_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Video server send a wrong format ack!\n");
        return -1;
    }
    iChannel_id= atoi(pTmp);

    pSendChannel = search_channel_via_id(iChannel_id);
    if (pSendChannel == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Can't find the required channel!\n");
        return -1;    
    }
    
    if (pChn->required_seq == 0)
    {
        seq = M_MAX_SEQ;
    }
    seq = pChn->required_seq - 1;
    
    sprintf(buf, M_MESSAGE_HEAD_FMT, E_PROGRAM_REQUEST_ACK_CODE, seq);
    sprintf(buf+strlen(buf), M_PGM_REQ_ACK_MSG_FMT, result, pAuth, pVideoSvrInfo, pStreamInfo);
    sprintf(buf+strlen(buf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pSendChannel, E_PROT_TYPE_XML, strlen(buf), buf);

    return 0;
}

