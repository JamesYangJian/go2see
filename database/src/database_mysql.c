#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mysql.h>

#include "sys_utils.h"
#include "database_mysql.h"
#include "database.h"

S_DATABASE_INTERFACE database_mysql = 
{
    "mysql",
    database_mysql_module_init,
    database_mysql_module_finalize,
    database_mysql_execute_cmd,
    database_mysql_num_rows,
    database_mysql_num_fields,
    database_mysql_fetch_record,
    database_mysql_release_results
};

static  MYSQL *mysql = NULL;

int database_mysql_module_init(char *pSvrIP, short sPort, char *pUserName, char *pUserPass, char *pDBName)
{
    mysql = mysql_init(NULL); 

    mysql = mysql_real_connect(mysql, pSvrIP, pUserName, pUserPass, pDBName, sPort, NULL, 0);

    if (mysql == NULL)
    {
        SYS_LOG_PRINTF("Connect to database failed, ip:%s port:%d user:%s!\n", pSvrIP, sPort, pUserName);
        return -1;
    }

    return 0;

}

void database_mysql_module_finalize()
{
    mysql_close(mysql);
    mysql_server_end();
}

void *database_mysql_execute_cmd(char *pCmd)
{
    int iRet = 0;
    MYSQL_RES *results = NULL;
    
    if (pCmd == NULL || mysql == NULL)
    {
        return NULL;
    }

    iRet = mysql_query(mysql, pCmd);
    if (iRet != 0)
    {
        return NULL;
    }
    
    results = mysql_store_result(mysql);

    return (void *)results;
}

int database_mysql_num_rows(void *results)
{
    int num = 0;
    if (results != NULL)
    {
        num = mysql_num_rows((MYSQL_RES *)results);
    }

    return num;
}

int database_mysql_num_fields(void *results)
{
    int num = 0;
    if (results != NULL)
    {
        num = mysql_num_fields((MYSQL_RES *)results);
    }

    return num;
}

char **database_mysql_fetch_record(void *results)
{
    MYSQL_ROW record;
    if (results != NULL)
    {
        record = mysql_fetch_row((MYSQL_RES *)results);
    }

    return (char **)record;
}

void database_mysql_release_results(void *results)
{
    if (results != NULL)
    {
        mysql_free_result((MYSQL_RES *)results);
    }
}

