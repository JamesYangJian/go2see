
#ifndef	__TRANSPORT_LAYER_H
#define	__TRANSPORT_LAYER_H

#include <netinet/in.h> 
#include <sys/socket.h> 

#define	TYPE_STREAM	1
#define	TYPE_DGRAM	2

#define M_RET_ERROR -1
#define M_RET_OK     0

#define M_TL_BLOCK_LEN	0x00000400

int tl_create(int type);

int tl_destroy(int socket);

int tl_open_listen(int lsocket,unsigned short listen_port);

int tl_connect(int socket,int dest_ip,unsigned short dest_port,struct timeval * tv);

int tl_snd(int socket,unsigned char * snd_buf,int len,struct timeval* tv);

int tl_rcv(int socket,unsigned char * rcv_buf,int len,struct timeval* tv);

int tl_getlocalip(char *eth, char *ipAddr);

int tl_udp_bind(int sock, short port);

int tl_getlocalmac(char *eth, char *pmac);

int tl_snd_stream(int socket,unsigned char * snd_buf,int len,struct timeval* tv);

int tl_rcv_udp(int socket,unsigned char * rcv_buf,int len,struct timeval* tv, struct sockaddr *addr);

int tl_snd_udp(int sock, unsigned char *snd_buf, int len, struct timeval *tv, struct sockaddr *addr);
#endif
