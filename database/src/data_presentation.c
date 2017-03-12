#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_utils.h"
#include "database.h"
#include "data_presentation.h"

static S_DATABASE_INTERFACE *gp_database_interface = NULL;

int database_module_init(S_DB_CONFIG *pDBConfig)
{
    int iRet = 0;
    
    if (pDBConfig == NULL)
    {
        SYS_LOG_PRINTF("Error! Can't init a NULL database config!\b");
        return -1;
    }

    gp_database_interface = find_database(pDBConfig->pDBType);
    if (gp_database_interface == NULL)
    {
        SYS_LOG_PRINTF("Error! Database info not exist!\b");
        return -1;
    }

    iRet = gp_database_interface->database_module_init(pDBConfig->pDBSvrIP, pDBConfig->sDBPort, pDBConfig->pUserName,
        pDBConfig->pUserPass, pDBConfig->pDBName);

    return iRet;
}

S_DEVICE_INFO *query_dev_info_by_id(char *pID, int *pRecordNum)
{
    char szCmd[128];
    void *results = NULL;
    int    row_num = 0;
    int    field_num = 0;
    char **record = 0;
    int i = 0;
    S_DEVICE_INFO *dev_list = NULL;

    if (pID == NULL || pRecordNum == NULL)
    {
        return NULL;
    }

    sprintf(szCmd, "select * from %s where name=\"%s\";", M_DEV_INFO_TABLE_NAME, pID);

    results = gp_database_interface->database_execute_cmd((char *)szCmd);
    if (results == NULL)
    {
        return NULL;
    }

    row_num = gp_database_interface->database_num_rows(results);
    field_num = gp_database_interface->database_num_fields(results);

    if (row_num > 0)
    {
        dev_list = (S_DEVICE_INFO *)malloc(row_num * sizeof(S_DEVICE_INFO));
        memset(dev_list, 0, row_num * sizeof(S_DEVICE_INFO));
        *pRecordNum = row_num;

        for (i=0; i<row_num; i++)
        {
            record = gp_database_interface->database_fetch_record(results);
            strncpy(dev_list[i].szDevID, record[0], M_DEV_ID_LEN);
            strncpy(dev_list[i].szDevPass, record[1], M_DEV_PASS_LEN);
            strncpy(dev_list[i].szDescription, record[2], M_DEV_DESC_LEN);
            dev_list[i].iType = atoi(record[3]);
        }
    }

    gp_database_interface->database_release_results(results);

    return dev_list;
}

int *query_device_groups(char *pDevID, int *pRecordNum)
{
    char szCmd[128];
    void *results = NULL;
    int    row_num = 0;
    int    field_num = 0;
    char **record = 0;
    int i = 0;
    int *pGroupList = NULL;
    
    if (pDevID == NULL || pRecordNum == NULL)
    {
        return NULL;
    }
    
    sprintf(szCmd, "select * from %s where dev_id=\"%s\";", M_DEV_GROUP_LIST_TABLE_NAME, pDevID);

    results = gp_database_interface->database_execute_cmd((char *)szCmd);
    if (results == NULL)
    {
        return NULL;
    }

    row_num = gp_database_interface->database_num_rows(results);
    field_num = gp_database_interface->database_num_fields(results);

    if (row_num > 0)
    {
        pGroupList = (int *)malloc(sizeof(int) * row_num);
        memset(pGroupList, 0, sizeof(int) * row_num);
        *pRecordNum = row_num;

        for (i=0; i<row_num; i++)
        {
            record =  gp_database_interface->database_fetch_record(results);
            pGroupList[i] = atoi(record[1]);
        }
    }

    gp_database_interface->database_release_results(results);

    return pGroupList;
}

int *query_group_programs(int group_id, int *pRecordNum)
{
    char szCmd[128];
    void *results = NULL;
    int    row_num = 0;
    char **record = 0;
    int i = 0;
    int *pProgramList = NULL;
    
    if (pRecordNum == NULL)
    {
        return NULL;
    }
    
    sprintf(szCmd, "select * from %s where group_id=\"%d\";", M_GROUP_PROGRAM_LIST_TABLE_NAME, group_id);

    results = gp_database_interface->database_execute_cmd((char *)szCmd);
    if (results == NULL)
    {
        return NULL;
    }

    row_num = gp_database_interface->database_num_rows(results);

    if (row_num > 0)
    {
        pProgramList= (int *)malloc(sizeof(int) * row_num);
        memset(pProgramList, 0, sizeof(int) * row_num);
        *pRecordNum = row_num;

        for (i=0; i<row_num; i++)
        {
            record =  gp_database_interface->database_fetch_record(results);
            pProgramList[i] = atoi(record[1]);
        }
    }

    gp_database_interface->database_release_results(results);

    return pProgramList;
}

S_PROGRAM_INFO *query_program_info_by_id(int program_id, int*pRecordNum)
{
    char szCmd[128];
    void *results = NULL;
    int    row_num = 0;
    char **record = 0;
    int i = 0;
    S_PROGRAM_INFO *pPGInfo = NULL;
    
    if (pRecordNum == NULL)
    {
        return NULL;
    }
    
    sprintf(szCmd, "select * from %s where program_id=\"%d\";", M_PROGRAM_LIST_TABLE_NAME, program_id);

    results = gp_database_interface->database_execute_cmd((char *)szCmd);
    if (results == NULL)
    {
        return NULL;
    }

    row_num = gp_database_interface->database_num_rows(results);

    if (row_num > 0)
    {
        pPGInfo = (S_PROGRAM_INFO *)malloc(sizeof(S_PROGRAM_INFO) * row_num);
        memset(pPGInfo, 0, sizeof(S_PROGRAM_INFO) * row_num);
        *pRecordNum = row_num;

        for (i=0; i<row_num; i++)
        {
            record =  gp_database_interface->database_fetch_record(results);
            pPGInfo[i].program_id = atoi(record[0]);
            strncpy(pPGInfo[i].szProgramName, record[1], M_PG_NAME_LEN);
            strncpy(pPGInfo[i].szProgramDesc, record[2], M_PG_DESC_LEN);
            strncpy(pPGInfo[i].szProgramPath, record[3], M_PG_PATH_LEN);
        }
    }

    gp_database_interface->database_release_results(results);

    return pPGInfo;
}

int insert_online_device_record(S_ONLINE_DEVICE *pOnLineDev)
{
    char szCmd[128];
    void *results = NULL;
    
    if (pOnLineDev == NULL)
    {
        return -1;
    }

    sprintf(szCmd, "insert into %s values (\"%s\", \"%s\", \"%d\");",M_ONLINE_DEV_TABLE_NAME, 
        pOnLineDev->szDevID, pOnLineDev->szIPAddr, pOnLineDev->valid_time);

    results = gp_database_interface->database_execute_cmd(szCmd);

    if (results != NULL)
    {
        gp_database_interface->database_release_results(results);
    }

    return 0;
}

int update_online_device_record(S_ONLINE_DEVICE *pOnlineDev)
{
    char szCmd[128];
    void *results = NULL;

    if (pOnlineDev == NULL)
    {
        return -1;
    }

    sprintf(szCmd, "update %s set  ip_address=\"%s\", valid_time=\"%d\" where dev_name=\"%s\";", M_ONLINE_DEV_TABLE_NAME, 
        pOnlineDev->szIPAddr, pOnlineDev->valid_time, pOnlineDev->szDevID);

    results = gp_database_interface->database_execute_cmd(szCmd);

    if (results != NULL)
    {
        gp_database_interface->database_execute_cmd(szCmd);
    }

    return 0;
}
