#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>


#include "buf_manage.h"
#include "event_manage.h"
#include "sys_utils.h"
#include "transport.h"
#include "sys_conf.h"
#include "data_presentation.h"
#include "tlv.h"




int main()
{
    int iRet = 0;

    //init system configuration module
    module_sys_conf_init();

    //init buffer manage module
    module_buf_manage_init();

    //init event manage module
    module_event_manage_init();

    //init database module
    S_DB_CONFIG stDBConfig;
    stDBConfig.pDBType      = get_db_type();
    stDBConfig.pDBSvrIP     = get_db_server();
    stDBConfig.sDBPort        = (short)get_db_server_port();
    stDBConfig.pUserName   = get_db_user_name();
    stDBConfig.pUserPass     = get_db_user_pass();
    stDBConfig.pDBName     = get_database_name();
    
    iRet = database_module_init(&stDBConfig);
    if (iRet != 0)
    {
        SYS_LOG_PRINTF("Fatal Error! init database failed!\n");
        exit(0);
    }
    
    //init transport module
    iRet = module_transport_init();
    if (iRet != 0)
    {
        exit(0);
    }

    //init tlv message handler module
    init_tlv_module();

    //start main loop
    run();
}
