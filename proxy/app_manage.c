#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "chain.h"
#include "app_manage.h"

void module_app_manager_init()
{
    register_xml_msg_dispatcher_to_tlv(xml_msg_dispatcher);
}
