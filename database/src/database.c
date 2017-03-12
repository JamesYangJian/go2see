#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"

extern S_DATABASE_INTERFACE database_mysql;

static S_DATABASE_INTERFACE *database_set[] = 
{
    &database_mysql,
};

S_DATABASE_INTERFACE *find_database(char *pDBType)
{
    int iDBConfigNum = 0;
    S_DATABASE_INTERFACE *pDBInterface = NULL;
    int i = 0;

    iDBConfigNum = sizeof(database_set)/sizeof(void *);

    for(i=0; i<iDBConfigNum; i++)
    {
        pDBInterface = database_set[i];
        if (strcmp(pDBInterface->pDBType, pDBType) == 0)
        {
            break;
        }
    }
    
    return pDBInterface;
}