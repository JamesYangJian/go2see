#ifndef M_MSG_MANAGE_H__
#define M_MSG_MANAGE_H__

#include "data_presentation.h"

#define M_MAX_SEQ 0xffffffe

#define M_REGISTER_INTERVAL_IN_SEC 60

typedef enum
{
    E_MSG_TYPE_START = 0,
    E_MSG_TYPE_REQUEST,
    E_MSG_TYPE_RESPONSE,
    E_MSG_TYPE_INVALID
}E_MSG_TYPE;

typedef enum
{
    E_REGISTER_CODE = 1000,                                 //1000
    E_REGISTER_ACK_CODE,                                    //1001
    E_PROGROM_INFO_CODE,                                   //1002
    E_PROGRAM_INFO_ACK_CODE,                           //1003
    E_PROGRAM_REQUEST_CODE,                             //1004
    E_PROGRAM_REQUEST_ACK_CODE,                    //1005
    E_PROGRAM_REQUEST_NOTIFY_CODE,              //1006
    E_PROGRAM_REQUEST_NOTIFY_ACK_CODE,      //1007
    E_INVALID_CODE
}E_MESSAGE_CODE;


//Return code for device register
#define M_REG_RET_SUCCEED                      0
#define M_REG_RET_FAIL_NOT_EXIST         1
#define M_REG_RET_FAIL_AUTH                  2
#define M_REG_RET_FAIL_INTERNAL           3

//Return code for program info request
#define M_PG_INFO_RET_SUCCEED                       0
#define M_PG_INFO_RET_FAIL_NOT_EXIST          1
#define M_PG_INFO_RET_FAIL_NOT_ONLINE        2

//Return code for program request
#define M_PG_REQ_RET_SUCCEED                               0
#define M_PG_REQ_RET_DEV_NOT_EXIST                   1
#define M_PG_REQ_RET_FAIL_NOT_ONLINE               2
#define M_PG_REQ_RET_FAIL_PG_NOT_EXIST           3
#define M_PG_REQ_RET_FAIL_SVR_ERROR                 4

#define M_MESSAGE_HEAD_FMT \
"<message>\n\
<cmd>%d</cmd>\n\
<seq>%d</seq>\n\
<param>\n"

#define M_MESSAGE_TAIL_FMT \
"</param>\n\
</message>"

#define M_REGISTER_MSG_FMT \
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
<program_id>%s</program_id>\n"

#define M_PGM_REQ_ACK_MSG_FMT \
"<result>%d</result>\n\
<auth>%s</ auth >\n\
<video_server_info>%s</video_server_info>\n\
<stream_info>%s</stream_info>\n"

#define M_PGM_REQ_NOITFY_MSG_FMT \
"<dev_id>%s</dev_id>\n\
<program_id>%s</program_id>\n\
<channel_id>%d</channel_id>\n"

#define M_PGM_REQ_NOTIFY_ACK_MSG_FMT \
"<result>%d</result>\n\
<auth>%s</auth>\n\
<video_server_info>%s</video_server_info>\n\
<stream_info>%s</stream_info>\n\
<channel_id>%d</channel_id>\n"

#endif
