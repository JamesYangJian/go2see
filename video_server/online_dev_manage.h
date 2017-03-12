#ifndef _ONLINE_DEV_MANAGE_H__
#define _ONLINE_DEV_MANAGE_H__

#include "data_presentation.h"

typedef struct
{
    CHAIN_NODE node;
    S_ONLINE_DEVICE stOnlineDev;
}S_ONLINE_DEV_NODE;

void module_online_dev_mange_init();
int add_online_device(S_ONLINE_DEVICE *pOnlineDev); 
S_ONLINE_DEVICE *find_online_device(char *pDevID);

#endif
