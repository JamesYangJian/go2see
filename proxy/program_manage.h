#ifndef _PROGRAM_MANAGE_H__
#define _PROGRAM_MANAGE_H__

#include "chain.h"
#include "data_presentation.h"

typedef struct
{
    CHAIN_NODE node;
    S_PROGRAM_INFO stProgramInfo;
}S_PROGRAM_INFO_NODE;

int find_program_list_via_dev_id(S_COMMON_LIST *list_head, char *pDevID);
int program_exist(int program_id);

#endif
