#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <stdarg.h>

#include <assert.h>

#include "sys_utils.h"

//Buffer to save date time, YYYYMMDD-HH:MM:SS
static char szDateTime[128];


void SYS_Free(void * pPointer)
{
	free(pPointer);
}

void *SYS_Malloc(unsigned int ulSize)
{
	return malloc(ulSize);
}

char *decode(const char *encd, char *decd )
{
    if( (encd == NULL ) || (decd == NULL))
        return (char* )0;

    int i, j=0;
    char *cd = (char *)encd;
    char p[2];
    unsigned int num;

    for( i = 0; i < strlen( cd ); i++ )
    {
        printf("%x ", cd[i]);
        memset( p, '\0', 2 );
        if( cd[i] != '%' )
        {
            decd[j++] = cd[i];
            continue;
        }
        p[0] = cd[++i];
        p[1] = cd[++i];

        sscanf( p, "%x", &num );
        sprintf(p, "%c", num );
        decd[j++] = p[0];
    }
    decd[j] = '\0';
    printf("\n");

    return decd;
}

void get_net_flow_stat(int *pInBytes, int *pInPkts, int *pOutBytes, int *pOutPkts, int *pInKBPS, int *pOutKBPS)
{
    FILE *fp = NULL;
    char buf[100];
    static int last_inbytes = 0, last_outbytes = 0;
    int in_bytes = 0, out_bytes = 0;
    static time_t last_sec = 0;
    time_t t = 0;

    if (pInBytes == NULL || pInPkts == NULL || pOutBytes == NULL || pOutPkts == NULL ||
        pInKBPS == NULL || pOutKBPS == NULL)
    {
        return;
    }
    *pInKBPS = 0;
    *pOutKBPS = 0;

    fp = fopen("/tmp/flow.txt", "r");
    if (fp == NULL)
    {
        return;
    }
    fread(buf, 1, 100, fp);
    fclose(fp);
    sscanf(buf, M_FLOW_STR_FMT, pInBytes, pInPkts, pOutBytes, pOutPkts);
    in_bytes = *pInBytes;
    out_bytes = *pOutBytes;
    time(&t);
    if (last_sec == 0)
    {
        last_sec = t;
        last_inbytes = in_bytes;
        last_outbytes = out_bytes;
    }
    else
    {
        *pInKBPS  = ( (in_bytes - last_inbytes) >> 10 ) / (t - last_sec);
        *pOutKBPS = ( (out_bytes - last_outbytes) >> 10 ) / (t - last_sec);
        last_sec = t;
        last_inbytes = in_bytes;
        last_outbytes = out_bytes;
    }
    return;

}


#define   SET_IF_DESIRED(x,y)     if(x)   *(x)   =   (y)
#define   _ULL   unsigned   long   long

int   FiveCpuNumbers(int *uret,int *nret, int *sret, int *iret, int *iowait)
{
    //static   int   u   =   0,   m   =   0,   s   =   0,   i   =   0,   iw   =   0;
    int user_j, nice_j, sys_j, idle_j, iowait_j = 0;
    FILE   *fp;
    size_t   byte_read;
    char   buffer[100];

    fp = fopen("/proc/stat", "r");
    byte_read =   fread(buffer,   1,   sizeof(buffer)-1,   fp);
    fclose(fp);
    //printf("%s %d running here, byte_read:%d\nbuffer:%s!\n", __FILE__, __LINE__,
    //        byte_read, buffer); 

    if (byte_read==0 || byte_read==sizeof(buffer))
        return   -1;
    buffer[byte_read]   =   '\0';

    sscanf(buffer,   "cpu   %d   %d   %d   %d   %d",   &user_j,   &nice_j,   &sys_j,   &idle_j,   &iowait_j);
    SET_IF_DESIRED(uret,   user_j);
    SET_IF_DESIRED(nret,   nice_j);
    SET_IF_DESIRED(sret,   sys_j);
    SET_IF_DESIRED(iret,   idle_j);
    SET_IF_DESIRED(iowait,   iowait_j);
#if 0
    printf("cpu   %d   %d   %d   %d   %d\n",   user_j,   nice_j,   sys_j,   idle_j,   iowait_j);
    SET_IF_DESIRED(uret,   user_j   -   u);
    SET_IF_DESIRED(nret,   nice_j   -   m);
    SET_IF_DESIRED(sret,   sys_j   -   s);
    /*   Idle   can   go   backwards   one   tick   due   to   kernel   calculation   issues   */
    SET_IF_DESIRED(iret,   (idle_j   >   i)   ?   (idle_j   -   i)   :   0);
    SET_IF_DESIRED(iowait,   iowait_j   -   iw);
    u   =   user_j;
    m   =   nice_j;
    s   =   sys_j;
    i   =   idle_j;
    iw   =   iowait_j;
    //printf("cpu   %Lu   %Lu   %Lu   %Lu   %Lu   \n",user_j,nice_j,sys_j,idle_j,iowait_j);
#endif
    return   0;
}

  /*
    *   get   the   cpu   usage
    */
int   ReadCpuInfo(int *iCpuUsage)
{
    int cpu_usage = 0;
    int user_j, nice_j, sys_j, idle_j,iowait_j = 0;

    if(FiveCpuNumbers(&user_j, &nice_j, &sys_j, &idle_j, &iowait_j)!=0)
        return -1;

    //printf("user_j:%d nice_j:%d, sys_j:%d, idle_j:%d, iowait_j:%d\n",
    //        user_j, nice_j, sys_j, idle_j, iowait_j);

    cpu_usage = (idle_j * 100) / (user_j   +   nice_j   +   sys_j   +   idle_j   +   iowait_j);
    //printf("cpu usage:%d\n", cpu_usage);

    if(cpu_usage>100)
        cpu_usage   =   100;

    /*   change   to   useage   */
    cpu_usage = 100 - (cpu_usage);
    *iCpuUsage = cpu_usage;
    //printf("###   CPU   Usage   :   %d   %%\n",   cpu_usage);
    return 0;
}
#undef   _ULL

int caculate_hour_index_per_week()
{
    struct tm *t    = NULL;
    unsigned int iHourIdx = 0;
    unsigned int iWeekDay = 0;
    unsigned int iHour    = 0;
    time_t thetm;

    time(&thetm);
    t = localtime(&thetm);
    iWeekDay = t->tm_wday;
    iHour = t->tm_hour;

    iHourIdx = 24 * iWeekDay + iHour;
    return iHourIdx;
}

void sys_log_printf(char *file, int line, char *fmt, ...)
{
    char buf[512];
    va_list ap;
    time_t t;
    struct tm *dc;

    time(&t);
    dc = localtime(&t);

    strftime(buf, 512, "%Y-%m-%d %H:%M:%S", dc);
    printf("-----------%s------------\n", buf);

    va_start(ap, fmt);

    vsprintf(buf, fmt, ap);

    printf("%s %d %s\n", file, line, buf);
}

char *current_date_time()
{
    struct tm *t    = NULL;
    time_t thetm;
    

    time(&thetm);
    t = localtime(&thetm);

    strftime(szDateTime, sizeof(szDateTime), "%Y%m%d-%H:%M:%S", t);

    return szDateTime;
}