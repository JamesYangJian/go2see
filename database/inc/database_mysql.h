#ifndef _DATABASE_MYSQL_h__
#define _DATABASE_MYSQL_h__

int database_mysql_module_init(char *pSvrIP, short sPort, char *pUserName, char *pUserPass, char *pDBName);
void database_mysql_module_finalize();

void *database_mysql_execute_cmd(char *pCmd);
int database_mysql_num_rows(void *results);
int database_mysql_num_fields(void *results);
char **database_mysql_fetch_record(void *results);
void database_mysql_release_results(void *results);

#endif
