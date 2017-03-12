#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_utils.h"
#include "program_manage.h"


int find_program_list_via_dev_id(S_COMMON_LIST *list_head, char *pDevID)
{
    int *pGroupIDList = NULL;
    int *pPGIDList = NULL;
    int iGroupIDRecordNum = 0;
    int iProgramIDRecordNum = 0;
    int iProgramRecordNum = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int group_id = 0;
    int program_id = 0;
    int program_count = 0;
    S_PROGRAM_INFO_NODE *pPGInfoNode = NULL;
    S_PROGRAM_INFO *pPGInfo = NULL;
    
    if (list_head == NULL || pDevID == NULL)
    {
        return 0;
    }

    //1. find group via device id
    pGroupIDList = query_device_groups(pDevID, &iGroupIDRecordNum);
    if (pGroupIDList == NULL)
    {
        return 0;
    }

    //2. find program via groups
    for (i=0; i<iGroupIDRecordNum; i++)
    {
        group_id = pGroupIDList[i];
        pPGIDList = query_group_programs(group_id, &iProgramIDRecordNum);
        if (pPGIDList != NULL)
        {
            for(j=0; j<iProgramIDRecordNum; j++)
            {
                program_id = pPGIDList[j];
                pPGInfo = query_program_info_by_id(program_id, &iProgramRecordNum);
                if (pPGInfo != NULL)
                {
                    pPGInfoNode = (S_PROGRAM_INFO_NODE *)malloc(iProgramRecordNum * sizeof(S_PROGRAM_INFO_NODE));
                    memset(pPGInfoNode, 0, iProgramRecordNum * sizeof(S_PROGRAM_INFO_NODE));
                    for (k=0; k<iProgramRecordNum; k++)
                    {
                        memcpy(&pPGInfoNode[k].stProgramInfo, &pPGInfo[k], sizeof(S_PROGRAM_INFO));
                        _insert_node_tail((CHAIN_HEAD *)list_head, (CHAIN_NODE *)(&pPGInfoNode[k]));
                        list_head->count ++;
                    }
                }
            }
        }
    }

    if (pGroupIDList != NULL)
    {
        free(pGroupIDList);
        pGroupIDList = NULL;
    }

    if (pPGIDList != NULL)
    {
        free(pPGIDList);
        pPGIDList = NULL;
    }

    if (pPGInfo != NULL)
    {
        free(pPGInfo);
        pPGInfo = NULL;
    }

    return list_head->count;
}

int program_exist(int program_id)
{
    int record_num = 0;
    S_PROGRAM_INFO *pProgram = NULL;
    
    pProgram = query_program_info_by_id(program_id, &record_num);

    if (pProgram != NULL)
    {
        free(pProgram);
        pProgram = NULL;
        return 1;
    }

    return 0;
}
