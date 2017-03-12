#ifndef _TRANSACTION_H__
#define _TRANSACTION_H__

#include "ezxml.h"
#include "tlv.h"
#include "channel.h"
#include "msg_define.h"



#define M_TRANS_ID_LEN   48

#define M_MAX_TRANSACTION_ID  0xfffffffe

#define M_TRANSACTION_MAX_RETRY_TIME 3
#define M_TRANSACTION_RETRY_INTERVAL  5

typedef enum
{
    E_TRANSACTION_TYPE_REQ = 0,
    E_TRANSACTION_TYPE_REP
}E_TRANSACTION_TYPE;

typedef enum
{
    E_TRANSACTION_UNCONFIRMED = 0,
    E_TRANSACTION_CONFIRMED
}E_TRANSACTION_STATE;

typedef struct
{
    CHAIN_NODE node;
    E_TRANSACTION_TYPE transaction_type;    //indication request/response transaction
    int transaction_cmd;                                   //message command for this transaction
    char transaction_id[M_TRANS_ID_LEN];       //compose by device_id and trans_id to identify the very transaction
    unsigned int cmd_seq;                                 //command sequence, to identify the very command
    char szCmdBuf[M_TLV_BUF_LEN];                 //contain message to be send
    COMM_CHANNEL *pChannel;                        //combined channel to send/recv data
    E_TRANSACTION_STATE state;                     //state confirmed/unconfirmed
    E_PROT_TYPE prot_type;                             //the protocol this transaction handled
    unsigned int expire_time;                            //expire time to retry
    int retry;                                                     //retry time
}S_TRANSACTION;

typedef int (* input_rep_msg_handler)(ezxml_t pxml, S_TRANSACTION *pTrans);
typedef int (* input_req_msg_handler)(ezxml_t pxml, char *pBuf);
typedef int (* output_req_msg_handler)(void *pParam, char *pBuf);

typedef struct
{
    CHAIN_NODE node;
    int cmd_code;
}S_COMM_HANDLER_NODE;

typedef struct
{
    S_COMM_HANDLER_NODE node;
    input_req_msg_handler handler;
}INPUT_REQ_HANDLER_NODE;

typedef struct
{
    S_COMM_HANDLER_NODE node;
    input_rep_msg_handler handler;
}INPUT_REP_HANDLER_NODE;

typedef struct
{
    S_COMM_HANDLER_NODE node;
    output_req_msg_handler handler;
}OUTPUT_REQ_HANDLER_NODE;

int module_transaction_init();
int alloc_cmd_seq();
void alloc_transaction_id(char *pTransID);
S_TRANSACTION *search_transaction_via_id(char *pID);
S_TRANSACTION *alloc_request_transaction(E_MESSAGE_CODE cmd);
S_TRANSACTION *alloc_response_transaction(E_MESSAGE_CODE cmd, char *pTransID, int seq);
void release_transaction(S_TRANSACTION *pTransaction);
void loop_transaction();

int transaction_send_output_request(S_TRANSACTION *pTrans, void *pParam);
int transaction_handle_input_request(S_TRANSACTION *pTrans, ezxml_t pxml);
int transaction_handle_input_response(S_TRANSACTION *pTrans, ezxml_t pxml);

void register_input_req_msg_handler(int cmd_code, input_req_msg_handler handler);
void register_input_rep_msg_handler(int cmd_code, input_rep_msg_handler handler);
void register_output_msg_handler(int cmd_code, output_req_msg_handler handler);
void *serach_handler_node_via_code(S_COMMON_LIST *pHead, int cmd_code);


#endif
