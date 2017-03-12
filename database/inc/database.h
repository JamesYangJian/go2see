#ifndef _DATABASE_H__
#define _DATABASE_H__

typedef struct
{
    char *pDBType;
    int (*database_module_init)(char *pSvrIP, short sPort, char *pUserName, char *pUserPass, char *pDBName);
    void (*database_module_finalize)();
    void *(*database_execute_cmd)(char *pCmd);
    int (*database_num_rows)(void *results);
    int (*database_num_fields)(void *results);
    char **(*database_fetch_record)(void *results);
    void (*database_release_results)(void *results);
}S_DATABASE_INTERFACE;


S_DATABASE_INTERFACE *find_database(char *pDBType);

#endif
