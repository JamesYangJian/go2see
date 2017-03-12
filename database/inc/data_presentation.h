#ifndef _DATA_PRESENTATION_H__
#define _DATA_PRESENTATION_H__

#define M_DEV_ID_LEN             24
#define M_DEV_PASS_LEN         24
#define M_DEV_DESC_LEN        128
#define M_IPADDR_LEN            16
#define M_GROUP_DESC_LEN    128
#define M_PG_NAME_LEN         128
#define M_PG_DESC_LEN          256
#define M_PG_PATH_LEN         128

#define M_DEV_INFO_TABLE_NAME "dev_list"
#define M_ONLINE_DEV_TABLE_NAME "online_list"
#define M_GROUP_LIST_TABLE_NAME "group_list"
#define M_PROGRAM_LIST_TABLE_NAME "program_list"
#define M_DEV_GROUP_LIST_TABLE_NAME "dev_group_list"
#define M_GROUP_PROGRAM_LIST_TABLE_NAME "group_program_list"

typedef struct
{
    char szDevID[M_DEV_ID_LEN + 1];
    char szDevPass[M_DEV_PASS_LEN + 1];
    char szDescription[M_DEV_DESC_LEN+1];
    int    iType;
}S_DEVICE_INFO;

typedef struct
{
    char szDevID[M_DEV_ID_LEN + 1];
    char szIPAddr[M_IPADDR_LEN];
    unsigned int valid_time;
}S_ONLINE_DEVICE;

typedef struct
{
    int program_id;
    char szProgramName[M_PG_NAME_LEN + 1];
    char szProgramDesc[M_PG_DESC_LEN + 1];
    char szProgramPath[M_PG_PATH_LEN +1];
}S_PROGRAM_INFO;

typedef struct
{
    char *pDBType;
    char *pDBSvrIP;
    short sDBPort;
    char *pUserName;
    char *pUserPass;
    char *pDBName;
}S_DB_CONFIG;

int database_module_init(S_DB_CONFIG *pDBConfig);

S_DEVICE_INFO *query_dev_info_by_id(char *pID, int *pRecordNum);
int *query_device_groups(char *pDevID, int *pRecordNum);
int *query_group_programs(int group_id, int *pRecordNum);
S_PROGRAM_INFO *query_program_info_by_id(int program_id, int*pRecordNum);
int insert_online_device_record(S_ONLINE_DEVICE *pOnLineDev);
int update_online_device_record(S_ONLINE_DEVICE *pOnlineDev);


#endif
