#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "sys_utils.h"
#include "chain.h"
#include "transport_layer.h"

#include "channel.h"
#include "xml_msg_handler.h"
#include "msg_define.h"
#include "online_dev_manage.h"
#include "tlv.h"
#include "program_manage.h"

int program_request_notify_handler(void *pChannel, ezxml_t pxml)
{
    char *pTmp = NULL;
    char *pDevID = NULL;
    int iProgram_id = 0;
    int iChannel_id = 0;
    int result = M_PG_REQ_NOTIFY_RET_SUCCEED;
    int ret = 0;
    int seq = 0;
    COMM_CHANNEL *pChn = NULL;
    COMM_CHANNEL *pDataSendChn = NULL;
    int program_num = 0;
    S_PROGRAM_INFO *pProgramInfo = NULL;
    S_PROGRAM_TRANSFER_INFO *pPgTransferInfo = NULL;
    char buf[256];
    char szServerInfo[24];
    int sock = -1;
    time_t t;
    char *pTransProt = NULL;
    

    if (pChannel == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for program_request_notify_ack_handler!\n ");
        return -1;
    }
    pChn = (COMM_CHANNEL *)pChannel;

    pTmp = ezxml_get_child_txt(pxml, "dev_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Proxy send a wrong format request!\n");
        return -1;        
    }
    pDevID = pTmp;

    pTmp = ezxml_get_child_txt(pxml, "program_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Proxy send a wrong format request!\n");
        return -1;        
    }
    iProgram_id = atoi(pTmp);

    pTmp = ezxml_get_child_txt(pxml, "channel_id");
    if (pTmp == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Proxy send a wrong format request!\n");
        return -1;        
    }
    iChannel_id = atoi(pTmp);

    pProgramInfo = find_program_record(iProgram_id, &program_num);
    if (pProgramInfo == NULL)
    {
        result = M_PG_REQ_NOTIFY_RET_FAIL_PG_NOT_EXIST;
    }
    else
    {
        pPgTransferInfo = (S_PROGRAM_TRANSFER_INFO *)malloc(sizeof(S_PROGRAM_TRANSFER_INFO));
        if (pPgTransferInfo == NULL)
        {
             SYS_LOG_PRINTF("Fatal error! Malloc program transfer info sturct failed!\n");
             exit(0);
        }
        else
        {
            memset(pPgTransferInfo, 0, sizeof(S_PROGRAM_TRANSFER_INFO));
            ret = gene_video_req_auth(pDevID, iProgram_id, (char *)pPgTransferInfo->szAuth);
            if (ret != 0)
            {
                 SYS_LOG_PRINTF("Internal error! Generate a video authentication string failed!\n");
                 result = M_PG_REQ_NOTIFY_RET_FAIL_AUTH_CREAT;
                 free(pPgTransferInfo);
            }
            else
            {
                pPgTransferInfo->video_port = alloc_video_port();
                strncpy(pPgTransferInfo->szProtoType, (unsigned char *)get_video_protocol(), M_PROTOTYPE_STR_LEN);
                //Just copy the first program info
                memcpy(&pPgTransferInfo->stProgramInfo, pProgramInfo, sizeof(S_PROGRAM_INFO));
            }
        }
    }

    memset(buf, 0, sizeof(buf));
    if (pChn->required_seq == 0)
    {
        seq = M_MAX_SEQ;
    }
    seq = pChn->required_seq - 1;    
    sprintf(buf, M_MESSAGE_HEAD_FMT, E_PROGRAM_REQUEST_NOTIFY_ACK_CODE, seq);
    if (result == M_PG_REQ_NOTIFY_RET_SUCCEED)
    {
        // alloc a channel to receive request from device
        pDataSendChn = apply_channel();
        if (pDataSendChn == NULL)
        {
            result = M_PG_REQ_NOTIFY_RET_FAIL_CHANNEL_FULL;
        }
        else
        {
            sock = tl_create(TYPE_DGRAM);
            tl_udp_bind(sock, pPgTransferInfo->video_port);
            pDataSendChn->sock = sock;
            pDataSendChn->video_send_info = (void *)pPgTransferInfo;
            time(&t);
            pDataSendChn->expire_time = t + M_PG_REQ_EXPIRE_INTERVAL;
            pTransProt = pPgTransferInfo->szProtoType;
            pTransProt = pTransProt + strlen(pTransProt) -3;
            if (strcmp(pTransProt, "UDP") == 0)
            {
                pDataSendChn->proto_type = E_PROTO_TYPE_UDP;
            }
            else
            {
                pDataSendChn->proto_type = E_PROTO_TYPE_TCP;
            }
            pDataSendChn->channel_type = E_CLIENT_CHANNEL;
        }

        sprintf(szServerInfo, "%s:%d", get_video_server_ip(), pPgTransferInfo->video_port);
        sprintf(buf+strlen(buf), M_PGM_REQ_NOTIFY_ACK_MSG_FMT, result, pPgTransferInfo->szAuth, 
            szServerInfo, pPgTransferInfo->szProtoType, iChannel_id);
    }
    else
    {
        sprintf(buf+strlen(buf), M_PGM_REQ_NOTIFY_ACK_MSG_FMT, result, "", "", "", iChannel_id);
    }

    sprintf(buf+strlen(buf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pChn, E_PROT_TYPE_XML, strlen(buf), (char *)buf);

    if (pProgramInfo != NULL)
    {
        free(pProgramInfo);
        pProgramInfo = NULL;
    }

    return 0;
}

int program_request_handler(void *pChannel, ezxml_t pxml)
{
    char *pAuth = NULL;
    COMM_CHANNEL *pChn = NULL;
    int ret = M_PG_REQ_RET_SUCCEED;
    S_PROGRAM_TRANSFER_INFO *pPGTransInfo = NULL;
    char szBuf[256];
    int seq = 0;
    char szFullFileName[M_PG_PATH_LEN+M_PG_NAME_LEN+1];

    if (pChannel == NULL || pxml == NULL)
    {
        SYS_LOG_PRINTF("Internal error, param error for program_request_notify_ack_handler!\n ");
        return -1;
    }
    
    pChn = (COMM_CHANNEL *)pChannel;
    pPGTransInfo = (S_PROGRAM_TRANSFER_INFO *)pChn->video_send_info;
    if (pPGTransInfo == NULL)
    {
        SYS_LOG_PRINTF("Internal error, Channel without any program transfer information!\n ");
        return -1;    
    }

    pAuth = ezxml_get_child_txt(pxml, "auth");
    if (pAuth == NULL)
    {
        SYS_LOG_PRINTF("Internal error! Received a NULL authentication request!\n");
        expire_channel(pChn);
        return -1;      
    }

    if (strcmp(pAuth, pPGTransInfo->szAuth) != 0)
    {
        ret = M_PG_REQ_RET_FAIL_AUTH;
    }

    memset(szFullFileName, 0, sizeof(szFullFileName));
    sprintf(szFullFileName, "%s/%s", pPGTransInfo->stProgramInfo.szProgramPath, pPGTransInfo->stProgramInfo.szProgramName);
    pPGTransInfo->fp = fopen(szFullFileName, "rb");
    if (pPGTransInfo->fp == NULL)
    {
        ret = M_PG_REQ_RET_FAIL_FILE_OPEN;
    }

    memset(szBuf, 0, sizeof(szBuf));
    if (pChn->required_seq == 0)
    {
        seq = M_MAX_SEQ;
    }
    seq = pChn->required_seq - 1;
    
    sprintf(szBuf, M_MESSAGE_HEAD_FMT, E_PROGRAM_RESPOSE_CODE, seq);
    sprintf(szBuf+strlen(szBuf), M_PGM_REQ_MSG_ACK_FMT, ret);
    sprintf(szBuf+strlen(szBuf), M_MESSAGE_TAIL_FMT);

    channel_send_tlv_msg(pChn, E_PROT_TYPE_XML, strlen(szBuf), (char *)szBuf);

    if (ret ==M_PG_REQ_RET_SUCCEED)
    {
        //Add channel to data send list and begin to send data
        add_send_channel(pChn);
    }
    else
    {
        //The channel will be released
        expire_channel(pChn);
    }
    return 0;
}

