

#ifndef _SYS_CONF_H__
#define _SYS_CONF_H__

#define M_SYS_CONF_FILE     "/mnt/cfg/config.cfg"

#define M_IP_STR_LEN        16
#define M_PORT_LEN           8
#define M_USERNAME_LEN  32
#define M_USERPASS_LEN   32
#define M_DBNAME_LEN      16
#define M_DBTYPE_LEN       16

#define bool int

//系统配置信息结构
typedef struct
{
    char    szDBType[M_DBTYPE_LEN+1];
    char    szDBServer[M_IP_STR_LEN];
    int       iDBSvrPort;
    char    szDBUserName[M_USERNAME_LEN+1];
    char    szDBUserPass[M_USERPASS_LEN+1];
    char    szDBName[M_DBNAME_LEN+1];
    int       iProxyPort;
    int       iProxyChnNum;
    char    szVideoSvrIP[M_IP_STR_LEN];
    int       iVideoSvrPt;
}SYS_CONFIG_DATA;


//系统记录对应信息结构，用于读写配置文件
typedef struct
{
	const char *name;
	void (*action)(char*, void *, int);
	void *object;
	int len;
}SYS_CONF_ITEM;

/*--------------------------内部接口定义---------------------------*/
void c_set_str(char *src, void *dest, int len);
void c_serialize_str(char *buf, const char *name, void *value);
void c_set_uint(char *src, void *dest, int len);
void c_serialize_uint(char *buf, const char *name, void *value);
void c_set_uchar(char *src, void *dest, int len);
void c_serialize_uchar(char *buf, const char *name, void *value);
void ParseConfFile();
void UpdateConfFile();
SYS_CONF_ITEM* FindItem(char *pSrc);
void GetBuildDateTime(char *buf);
/*--------------------------内部接口定义结束-----------------------*/

/*--------------------------外部接口定义---------------------------*/

//系统启动函数
void module_sys_conf_init();

//获取系统配置数据
const SYS_CONFIG_DATA *GetSysConfig();

char *get_db_type();
char *get_db_server();
int     get_db_server_port();
char *get_db_user_name();
char *get_db_user_pass();
char *get_database_name();
int     get_server_msg_port();
int     get_channel_num();
char *get_video_server_ip();
int     get_video_server_port();

//以下为各个系统配置项的获取接口
/*--------------------------外部接口定义结束-----------------------*/

#endif
