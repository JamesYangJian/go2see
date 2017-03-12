#ifndef _XML_MSG_DISPATCHER_H__
#define _XML_MSG_DISPATCHER_H__

typedef int (* msg_dispatcher)(void *pChn, ezxml_t pxml);

typedef struct
{
    int cmd;
    msg_dispatcher dispatcher;
}S_XML_MSG_DISPATCHER;

int xml_msg_dispatcher(char *pData, int len, void * pChannel);


#endif
