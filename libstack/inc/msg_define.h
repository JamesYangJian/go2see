#ifndef M_MSG_MANAGE_H__
#define M_MSG_MANAGE_H__


#define M_MAX_SEQ 0xffffffe

#define M_REGISTER_WAIT_IN_SEC 10

typedef enum
{
    E_MSG_TYPE_START = 0,
    E_MSG_TYPE_REQUEST,
    E_MSG_TYPE_RESPONSE,
    E_MSG_TYPE_INVALID
}E_MSG_TYPE;


//Message code, any reponse is 1 bigger than its request
typedef enum
{
    E_DEVICE_REGISTER_CODE  = 1000,
    E_DEVICE_REGISTER_ACK_CODE,                                 //1001
    E_PG_INFO_REQUEST_CODE,                                        //1002  
    E_PG_INFO_REQUEST_ACK_CODE,                               //1003
    E_PG_REQUEST_CODE,                                                 //1004
    E_PG_REQUEST_ACK_CODE,                                         //1005
    E_PG_REQUEST_NOTIFY_CODE,                                   //1006
    E_PG_REQUEST_NOTIFY_ACK_CODE,                           //1007
    E_PG_DATA_REQUEST_CODE,                                      //1008
    E_PG_DATA_RESPOSE_CODE,                                      //1009
    E_INVALID_CODE
}E_MESSAGE_CODE;


//Return code for program request notify
#define M_PG_REQ_NOTIFY_RET_SUCCEED                          0
#define M_PG_REQ_NOTIFY_RET_FAIL_PG_NOT_EXIST       1
#define M_PG_REQ_NOTIFY_RET_FAIL_AUTH_CREAT          2
#define M_PG_REQ_NOTIFY_RET_FAIL_CHANNEL_FULL       3

//Return code for program request
#define M_PG_REQ_RET_SUCCEED               0
#define M_PG_REQ_RET_FAILE_EXPIRE       1
#define M_PG_REQ_RET_FAIL_AUTH            2
#define M_PG_REQ_RET_FAIL_FILE_OPEN   3


#define M_MESSAGE_HEAD_FMT \
"<message>\n\
<type>%d</type>\n\
<cmd>%d</cmd>\n\
<seq>%d</seq>\n\
<transaction_id>%s</transaction_id>\n\
<param>\n"

#define M_MESSAGE_TAIL_FMT \
"</param>\n\
</message>"

#define M_DEV_REGISTER_MSG_FMT \
"<dev_id>%s</dev_id>\n\
<dev_pass>%s</dev_pass>\n\
<ip_addr>%s</ip_addr>\n\
<video_port>%d</video_port>\n"

#define M_REG_ACK_MSG_FMT \
"<result>%d</result>\n\
<reason>%s</reason>\n\
<heartbeat>%d</heartbeat>\n\
<datetime>%s</datetime>\n"

#define M_PGM_INFO_REQ_MSG_FMT \
"<dev_id>%s</dev_id>\n"

#define M_PGM_INFO_HEAD_MSG_FMT \
"<result>%d</result>\n\
<program_num>%d</program_num>\n"

#define M_PGM_INFO_MSG_FMT \
"<program_info>\n\
<program_id>%d</program_id>\n\
<program_description>%s</program_description>\n\
</program_info>\n"

#define M_PGM_REQ_MSG_FMT \
"<dev_id>%s</dev_id>\n\
<program_id>%d</program_id>\n"

#define M_PGM_REQ_ACK_MSG_FMT \
"<result>%d</result>\n\
<auth>%s</ auth >\n\
<video_server_info>%s</video_server_info>\n\
<stream_info>%s</stream_info>\n"

#define M_PGM_DATA_REQ_MSG_FMT \
"<auth>%s</auth>\n"

#define M_PGM_DATA_REQ_MSG_ACK_FMT \
"<result>%d</result>\n"



#endif
