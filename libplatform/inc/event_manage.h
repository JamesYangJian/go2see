#ifndef _EVENT_MANAGE_H__
#define _EVENT_MANAGE_H__

#include "chain.h"

typedef int (*EVENT_DISPATCHER)(void *data);

typedef struct
{
    CHAIN_HEAD head;
    int count;
}EVENT_LIST;

typedef struct
{
    CHAIN_NODE node;
    char       event_data[0];
}EVENT_NODE;

void module_event_manage_init();
EVENT_NODE *pop_event();
void add_event_tail(EVENT_NODE *pnode);
void *event_loop(void *argv);
void register_event_dispatcher(EVENT_DISPATCHER dispatcher);

#endif //_EVENT_MANAGE_H__
