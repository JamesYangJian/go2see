#ifndef _XML_MSG_HANDLER_H__
#define _XML_MSG_HANDLER_H__


int register_handler(void *pChannel, ezxml_t pxml);
int program_info_request_handler(void *pChannel, ezxml_t pxml);
int program_request_handler(void *pChannel, ezxml_t pxml);
int program_request_notify_ack_handler(void *pChannle, ezxml_t pxml);




#endif

