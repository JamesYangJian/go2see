#ifndef _BUF_MANAGE_H__
#define _BUF_MANAGE_H__

#include "chain.h"

#define M_BUF_LEN   10240
#define M_BUF_CNT   20


typedef struct
{
    CHAIN_NODE node;
    char       data[M_BUF_LEN];
}BUF_NODE;


void module_buf_manage_init();
char* alloc_buf(int len);
void  free_buf(char *data);



#endif //_BUF_MANAGE_H__
