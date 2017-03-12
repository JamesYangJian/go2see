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
    E_PROGRAM_REQUEST_NOTIFY_CODE = 1006,              //1006
    E_PROGRAM_REQUEST_NOTIFY_ACK_CODE,                 //1007
    E_PROGRAM_REQUEST_CODE,
    E_PROGRAM_RESPOSE_CODE,
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
<cmd>%d</cmd>\n\
<seq>%d</seq>\n\
<param>\n"

#define M_MESSAGE_TAIL_FMT \
"</param>\n\
</message>"

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

#define M_PGM_REQ_MSG_FMT \
"<auth>%s</auth>\n"

#define M_PGM_REQ_MSG_ACK_FMT \
"<result>%d</result>\n"



#endif
