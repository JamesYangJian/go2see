
#ifndef _SYS_UTIL_H
#define _SYS_UTIL_H


#define LIST_OK 0
#define LIST_ERROR 1

#define SYS_OK 0
#define SYS_ERROR 1
#define SYS_RCV_ERROR -1
#define SYS_SEM_FAIL 0xf001

#define SYS_TRUE 1
#define SYS_FALSE 0

#define M_MAX(a,b) (a>=b)?a:b

void sys_log_printf(char *file, int line, char *fmt, ...);

#define _sys_log_
#ifdef _sys_log_
#define SYS_LOG_PRINTF(fmt, args...) sys_log_printf(__FILE__, __LINE__, fmt, ##args)
#else
#define SYS_LOG_PRINTF(fmt, args...)
#endif

#define M_FLOW_STR_FMT \
"in-bytes: %d\n\
in-pkts: %d\n\
out-bytes: %d\n\
out-pkts: %d\n"

//modifed here
#define SYS_HASH_TABLE_SIZE 	16229

#define SYS_HASH_TABLE_INDEX(ulIpAddr) \
 	((ulIpAddr & 0xff)^((ulIpAddr >> 8) & 0xff)^ \
	((ulIpAddr >> 16) & 0xff)^((ulIpAddr >> 24) & 0xff))

/* Include our own copy of struct sysinfo to avoid binary compatability
 * problems with Linux 2.4, which changed things.  Grumble, grumble. */
struct my_sysinfo {
	long uptime;			/* Seconds since boot */
	unsigned long loads[3];		/* 1, 5, and 15 minute load averages */
	unsigned long totalram;		/* Total usable main memory size */
	unsigned long freeram;		/* Available memory size */
	unsigned long sharedram;	/* Amount of shared memory */
	unsigned long bufferram;	/* Memory used by buffers */
	unsigned long totalswap;	/* Total swap space size */
	unsigned long freeswap;		/* swap space still available */
	unsigned short procs;		/* Number of current processes */
	unsigned short pad;			/* Padding needed for m68k */
	unsigned long totalhigh;	/* Total high memory size */
	unsigned long freehigh;		/* Available high memory size */
	unsigned int mem_unit;		/* Memory unit size in bytes */
	char _f[20-2*sizeof(long)-sizeof(int)];	/* Padding: libc5 uses this.. */
};

void get_net_flow_stat(int *pInBytes, int *pInPkts, int *pOutBytes, int *pOutPkts, int *pInKBPS, int *pOutKBPS);
int   ReadCpuInfo(int *iCpuUsage);
char *decode(const char *encd, char *decd );
int caculate_hour_index_per_week();
char *current_date_time();
#endif 

