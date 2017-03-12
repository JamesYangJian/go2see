

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "sys_utils.h"
#include "sys_conf.h"

/*--------------------------ģ���Ա����---------------------------*/

SYS_CONFIG_DATA g_stConfigData;

SYS_CONF_ITEM clist[] =
{
    {"Database_Type",          c_set_str,  &g_stConfigData.szDBType,            M_DBTYPE_LEN},
    {"DBServer_IP",              c_set_str,  &g_stConfigData.szDBServer,         M_IP_STR_LEN-1},
    {"DBServer_Port",           c_set_uint, &g_stConfigData.iDBSvrPort,          sizeof(int)},
    {"DBServer_User",          c_set_str,  &g_stConfigData.szDBUserName,     M_USERNAME_LEN},
    {"DBServer_Pass",          c_set_str,  &g_stConfigData.szDBUserPass,       M_USERPASS_LEN},
    {"Database_Name",         c_set_str,  &g_stConfigData.szDBName  ,         M_DBNAME_LEN},
    {"Proxy_Server_Port",     c_set_uint, &g_stConfigData.iProxyPort  ,         sizeof(int)},
    {"Proxy_Chn_Num",        c_set_uint, &g_stConfigData.iProxyChnNum,     sizeof(int)},
    {"Video_Server_IP",        c_set_str,  &g_stConfigData.szVideoSvrIP,        M_IP_STR_LEN-1},
    {"Video_Server_Port",     c_set_uint, &g_stConfigData.iVideoSvrPt,          sizeof(int)},
};
/*--------------------------ģ���Ա�������-----------------------*/



/*--------------------------�ڲ��ӿ�-------------------------------*/


void PrintConfig()
{
}

void ParseConfFile()
{
    //���������ڽ��������ļ�����ȡ��������
    char buf[128];
    char *p                 = NULL;
    char *pTmp              = NULL;
    SYS_CONF_ITEM *pItem    = NULL;
    FILE          *fp       = NULL;

    fp = fopen(M_SYS_CONF_FILE, "r");
    if (fp == NULL)
    {
        //�����ļ������ڣ��˳�
        SYS_LOG_PRINTF("IPCamcfg file not exists, generat default!\n");
        exit(0);
    }

    while(fgets((char *)buf, sizeof(buf), fp) != NULL)
    {
       // trim(buf);

        if (buf[0] == '\n' || buf[0] == '/' || buf[0] == '#' || buf[0] == ' ')
        {
        	continue;
        }

        pTmp = strstr((const char*)buf, "=");
        if (pTmp == NULL)
        {
            continue;
        }

        p = pTmp;
        p--;
        while (*p == ' ') p--;
        *++p = 0;

        pItem = FindItem(buf);
        if (pItem == NULL)
        {
            continue;
        }

        pTmp ++;
        while( *pTmp == ' ')
        {
            pTmp++;
        }
        p = pTmp;
        while (*p != '\n' && *p != '\r' && *p != 0) p++;
        *p = 0;

        pItem->action(pTmp,  (char*)pItem->object, pItem->len);

    }

    fclose(fp);
}

void c_set_str(char *src, void *dest, int len)
{
    if ((src != NULL) && (dest != NULL))
    {
    	strncpy((char *)dest, src, len);
    }
}

void c_serialize_str(char *buf, const char *name, void *value)
{
    if ((buf != NULL ) && (name != NULL) && (value != NULL))
    {
        sprintf(buf, "%s = %s\n", name, (char*)value);
    }
}

void c_set_uint(char *src, void *dest, int len)
{
    if ((src != NULL) && (dest != NULL))
    {
        *(unsigned int *)dest = atoi(src);
    }
}

void c_serialize_uint(char *buf, const char *name, void *value)
{
    if ((buf != NULL ) && (name != NULL) && (value != NULL))
    {
        sprintf(buf, "%s = %d\n", name, *(unsigned int*)value);
    }
}

void c_set_uchar(char *src, void *dest, int len)
{
    if ((src != NULL) && (dest != NULL))
    {
        *(unsigned char*)dest = (unsigned char)atoi(src);
    }
}

void c_serialize_uchar(char *buf, const char *name, void *value)
{
    if ((buf != NULL ) && (name != NULL) && (value != NULL))
    {
        sprintf(buf, "%s = %d\n", name, *(unsigned char*)value);
    }
}

SYS_CONF_ITEM* FindItem(char *pSrc)
{
	SYS_CONF_ITEM *p;
	for (p=clist; p<clist+sizeof(clist)/sizeof(SYS_CONF_ITEM); p++)
	{
		if (strncmp(pSrc, p->name, strlen(p->name)) == 0)
		{
			return p;
		}
	}
	return NULL;
}



//��ȡ��ǰ�汾�ı������ں�ʱ��
void   GetBuildDateTime(char *buf)
{
    unsigned int iYear, iDay, iMon;
    char pMonth[4];
    unsigned char pRssMonth[][4] = {{"Jan"}, {"Feb"}, {"Mar"}, {"Apr"}, {"May"},  {"Jun"}, {"Jul"}, {"Aug"}, {"Sep"}, {"Oct"}, {"Nov"}, {"Dec"}};

    sscanf(__DATE__, "%s %d %d", pMonth, &iDay, &iYear);

    for (iMon=0; iMon<12; iMon++)
    {
        if (strcmp(pMonth, (char *)pRssMonth[iMon]) == 0)
        {
            break;
        }
    }

    iMon++;

    sprintf(buf, "%d-%d-%d %s", iYear, iMon, iDay, __TIME__);
}
/*--------------------------�ڲ��ӿڽ���---------------------------*/



/*--------------------------�ⲿ�ӿڶ���---------------------------*/

//ϵͳ��������,�������ļ��л�ȡ��������
void module_sys_conf_init()
{
    memset(&g_stConfigData, 0, sizeof(SYS_CONFIG_DATA));
    ParseConfFile();
}

//��ȡϵͳ��������
const SYS_CONFIG_DATA *GetSysConfig()
{
    return &g_stConfigData;
}

char *get_db_type()
{
    return (char *)g_stConfigData.szDBType;
}

char *get_db_server()
{
    return (char *)g_stConfigData.szDBServer;
}
int get_db_server_port()
{
    return g_stConfigData.iDBSvrPort;
}
char *get_db_user_name()
{
    return (char *)g_stConfigData.szDBUserName;
}
char *get_db_user_pass()
{
    return (char *)g_stConfigData.szDBUserPass;
}
char *get_database_name()
{
    return (char *)g_stConfigData.szDBName;
}

int get_server_msg_port()
{
    return g_stConfigData.iProxyPort;
}

int  get_channel_num()
{
    return g_stConfigData.iProxyChnNum;
}

char *get_video_server_ip()
{
    return g_stConfigData.szVideoSvrIP;
}

int     get_video_server_port()
{
    return g_stConfigData.iVideoSvrPt;
}
//��ȡЭ��ָ��ȫ��Ψһ�����к�
