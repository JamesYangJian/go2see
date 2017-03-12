#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 

#include <sys/wait.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <unistd.h>

#include <pthread.h>
#include <string.h>
#include <sys/time.h>
//#include <sys/socket.h>
//#include <netinet/in.h>

#include "transport_layer.h"
#include "sys_utils.h"

//
int tl_create(int type)
{
	int ret = -1;

	if(type == TYPE_STREAM)
		ret = socket(AF_INET,SOCK_STREAM,0);
	else
		ret = socket(AF_INET,SOCK_DGRAM,0);

	return ret;
}

//
int tl_destroy(int socket)
{
	int ret = -1;

	ret = close(socket);
	if(ret == -1)
		return M_RET_ERROR;

	return M_RET_OK;
}

//监听
int tl_open_listen(int lsocket,unsigned short listen_port)
{
    struct sockaddr_in     server_addr; 

    bzero(&server_addr,sizeof(struct sockaddr_in)); 
    server_addr.sin_family=AF_INET; 
    server_addr.sin_port=htons(listen_port); 
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY); 


    if(bind(lsocket,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) 
    { 
        return M_RET_ERROR;  
    } 
    listen(lsocket,5); 

    return M_RET_OK; 
}

//建立连接
int tl_connect(int socket,int dest_ip,unsigned short dest_port,struct timeval * tv)
{
    struct sockaddr_in  dest_addr;
    socklen_t len;
    int	tmp_len,ret = -1;

    //flags = fcntl(socket, F_GETFL, 0);
    //flags &= ~O_NONBLOCK;
    //fcntl(socket, F_SETFL, flags);
    tmp_len = sizeof(struct timeval);
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, tv, tmp_len);
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, tv, tmp_len);

    bzero(&dest_addr,sizeof(struct sockaddr_in)); 
    dest_addr.sin_family=AF_INET; 
    dest_addr.sin_port=htons(dest_port); 
    dest_addr.sin_addr.s_addr=htonl(dest_ip);
    len = sizeof(dest_addr);
    ret = connect(socket,(const struct sockaddr*)&dest_addr,len);

    if(ret == -1)
    	return M_RET_ERROR;
    else
    	return M_RET_OK;

#if 0
    flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags|O_NONBLOCK);

    FD_ZERO(&wd_fds);
    FD_SET(socket,&wd_fds);

    select(0,NULL,NULL,NULL,tv);

    ret = select((socket +1),NULL,&wd_fds,NULL,tv);
    if(ret > 0)
    {
    	if(FD_ISET(socket,&wd_fds))
    	{
    		getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, sizeof(int));
    		if(!error)
    			return M_RET_OK;
    	}
    }


    return M_RET_ERROR;
#endif

}


int tl_snd_stream(int socket,unsigned char * snd_buf,int len,struct timeval* tv)
{
	unsigned char	*databuf = snd_buf;
	int	tmp_len,ret = -1;
    int   bytessent = 0;

	tmp_len = sizeof(struct timeval);
	setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, tv, tmp_len);

       while(bytessent < len)
        {
            ret = send(socket,databuf+bytessent,len-bytessent,0);
            if ((ret == -1) || (ret == 0))
            {
                //P_PDEBUG("%s %d tl_snd_stream failed ret:%d\n", __FILE__, __LINE__, ret);
				fprintf(stderr, "send data fails, errno = %d, error str: %s\n", errno, strerror(errno));
                return -1;
            }
            else
            {
                bytessent += ret;
            }   
        }
        return 0;
}



//数据发送
int tl_snd(int socket,unsigned char * snd_buf,int len,struct timeval* tv)
{
	unsigned char	*databuf = snd_buf;
	int	block_num = (len/M_TL_BLOCK_LEN);
	int	last_data_len = (len%M_TL_BLOCK_LEN);
	int	sent_len = 0, tmp_send_len = 0;
	int	tmp_len,ret = -1,i;

	tmp_len = sizeof(struct timeval);
	setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, tv, tmp_len);
	
	for(i = 0; i < block_num; i++)
	{
		ret = send(socket,databuf,M_TL_BLOCK_LEN,0);
		if((ret == -1)||(ret != M_TL_BLOCK_LEN))
		{
			sent_len = -1;
			goto snd_err;
		}
		else
		{
			databuf += ret;
			sent_len += ret;
		}
	}

	if(last_data_len > 0)
	{
	       //Todo: yangjian add: error here
		ret = send(socket,databuf ,last_data_len-tmp_send_len,0);
		if((ret == -1))//||(ret != last_data_len))
		{
			sent_len = -1;
			goto snd_err;
		}
		else
		{
			databuf += ret;
			tmp_send_len += ret;
                     sent_len += ret;
							
		}
	}


snd_err:

	return sent_len;

}

//数据接收
int tl_rcv(int socket,unsigned char * rcv_buf,int len,struct timeval* tv)
{
    unsigned char	*databuf = rcv_buf;
    int	rcvd_len = 0,tmp_len = 0;
    int	ret = -1;

    tmp_len = sizeof(struct timeval);
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, tv, tmp_len);

    tmp_len = M_TL_BLOCK_LEN < len ? M_TL_BLOCK_LEN:len;
    for(;rcvd_len != len; )
    {
        ret = recv(socket,databuf,tmp_len,0);
        if((ret == -1)||(ret == 0))
        {
            SYS_LOG_PRINTF("%s %d recv failed, ret:%d\n", __FILE__, __LINE__, ret);
            rcvd_len = -1;
            goto rcvd_err;				
        }
        else
        {
            databuf += ret;
            rcvd_len += ret;
        }
    }

    rcvd_err:
    return rcvd_len;
}


int tl_rcv_udp(int socket,unsigned char * rcv_buf,int len,struct timeval* tv, struct sockaddr *addr)
{
    int ret = 0;
    int tmp_len;
    unsigned char *databuf = rcv_buf;

    tmp_len = sizeof(struct timeval);
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, tv, tmp_len);

    tmp_len = sizeof(struct sockaddr);
    ret = recvfrom(socket,databuf,len,0, addr, &tmp_len);
    printf("rcv_udp:%d\n", ret);
    if (ret < 0)
    {
        return M_RET_ERROR;
    }

    return ret;
}

int tl_snd_udp(int sock, unsigned char *snd_buf, int len, struct timeval *tv, struct sockaddr *addr)
{
    int ret = 0;
    int tmp_len;
    unsigned char *databuf = snd_buf;

    tmp_len = sizeof(struct timeval);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, tv, tmp_len);

    tmp_len = sizeof(struct sockaddr);
    ret = sendto(sock,databuf,len,0, addr, tmp_len);
    
    printf("send_udp:%d\n", ret);
    if (ret < 0)
    {
        return M_RET_ERROR;
    }

    return ret;
}

int tl_udp_bind(int sock, short port)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
    
	return 0;
}

//获取本地的IP地址
int tl_getlocalip(char *eth, char *ipAddr)
{
	struct ifreq req;
	struct sockaddr_in *addr;
	unsigned long ip;
    	int sock;
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		//Log("socket create error\n");
		ipAddr[0] = 0;
		return M_RET_ERROR;
	}

	strcpy(req.ifr_name,eth);
	if(ioctl(sock,SIOCGIFADDR,&req) == -1)
		return M_RET_ERROR;
	addr = (struct sockaddr_in *)&req.ifr_addr;
	ip = addr->sin_addr.s_addr;
	sprintf(ipAddr,"%ld.%ld.%ld.%ld",ip&0xff,(ip>>8)&0xff,(ip>>16)&0xff,(ip>>24)&0xff);
	close(sock);
	return M_RET_OK;
}

//获取本地的mac地址
int tl_getlocalmac(char *eth, char *pmac)
{
    struct ifreq req;
    int sock;

    if (pmac == NULL)
    {
        return M_RET_ERROR;
    }

    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        //Log("socket create error\n");
        pmac[0] = 0;
        return M_RET_ERROR;
    }

    strcpy(req.ifr_name,eth);
    if(ioctl (sock, SIOCGIFHWADDR, (char *) &req) == -1)
        return M_RET_ERROR;

    sprintf(pmac, "%02x%02x%02x%02x%02x%02x",
                  req.ifr_hwaddr.sa_data[0],
                  req.ifr_hwaddr.sa_data[1],
                  req.ifr_hwaddr.sa_data[2],
                  req.ifr_hwaddr.sa_data[3],
                  req.ifr_hwaddr.sa_data[4],
                  req.ifr_hwaddr.sa_data[5]);
    printf("Mac addr: %x %x %x %x %x %x\n", 
                  req.ifr_hwaddr.sa_data[0],
                  req.ifr_hwaddr.sa_data[1],
                  req.ifr_hwaddr.sa_data[2],
                  req.ifr_hwaddr.sa_data[3],
                  req.ifr_hwaddr.sa_data[4],
                  req.ifr_hwaddr.sa_data[5]);
    
    close(sock);
    return M_RET_OK;
}

