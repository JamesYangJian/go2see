#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include "sys_utils.h"
#include "transport_layer.h"
#include "sys_conf.h"

#include "transport.h"


static int g_video_msg_sock = -1;
static int g_max_fd = 0;

int module_transport_init()
{
    int msg_port = get_video_server_port();
    int iRet = 0;
    unsigned int on = 1;

    
    g_video_msg_sock = tl_create(TYPE_STREAM);
    if (g_video_msg_sock <= 0)
    {
        SYS_LOG_PRINTF("Create system message socket failed!\n");
        return -1;
    }

    setsockopt(g_video_msg_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    iRet = tl_open_listen(g_video_msg_sock, (unsigned short)msg_port);
    if (iRet < 0)
    {
        tl_destroy(g_video_msg_sock);
        return -1;
    }

    init_channel_list();

    return 0; 
}


void run()
{
    fd_set rd_set;
    fd_set except_set;
    int    iRet = 0;
    int    new_sock = 0;
    COMM_CHANNEL *pChn = NULL;
    struct timeval tv;

    tv.tv_sec = 10;
    tv.tv_usec = 0;
    
    g_max_fd = g_video_msg_sock;
    //accept message port and read channel data
    while (1)
    {
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        update_select_sets(&rd_set, &except_set);
        

        iRet = select(g_max_fd+1, &rd_set, NULL, &except_set, &tv);
        printf("%s\n", strerror(errno));
        if (iRet > 0)
        {
            if (FD_ISSET(g_video_msg_sock, &rd_set))
            {
                new_sock = accept(g_video_msg_sock, NULL, NULL);
                if (new_sock > 0)
                {
                    SYS_LOG_PRINTF("Accept successfully, create new channel!\n");
                    FD_SET(new_sock, &rd_set);
                    g_max_fd = M_MAX(g_max_fd, new_sock);
                    
                    pChn = apply_channel();
                    if (pChn != NULL)
                    {
                        pChn->sock = new_sock;
                        pChn->proto_type = E_PROTO_TYPE_TCP;
                        pChn->expire_time = 0xFFFFFFFF;
                    }
                    else
                    {
                        close(new_sock);
                    }
                }
                continue;
            }

            loop_channel_data(&rd_set, &except_set);
        }
        else
        {
            continue;
        }
    }
}

void loop_channel_data(fd_set *rd_set, fd_set *except_set)
{
    COMM_CHANNEL *pChn = get_used_channel_head();
    COMM_CHANNEL *pTmpChn = NULL;
    int iRet = 0;
    time_t t;

    if (rd_set == NULL)
    {
        return;
    }        

    while (pChn != NULL)
    {
        time(&t);
        if (pChn->expire_time > t)
        {
            SYS_LOG_PRINTF("Channel expires!\n");
            pTmpChn = pChn;        
        }
        else if (FD_ISSET(pChn->sock, except_set))
        {
            SYS_LOG_PRINTF("Channel exception, sock:%d!\n", pChn->sock);
            pTmpChn = pChn;
        }
        else if (FD_ISSET(pChn->sock, rd_set))
        {
            SYS_LOG_PRINTF("Begin to receive channel data!\n");
            iRet = channel_recv_tlv_msg(pChn);
            if (iRet < 0)
            {
                pTmpChn = pChn;
            }
        }

        pChn = get_next_channel(pChn);
        if (pTmpChn != NULL)
        {
            SYS_LOG_PRINTF("dequeue unused channel!\n");
            release_channel(pTmpChn);
            tl_destroy(pTmpChn->sock);
            pTmpChn = NULL;
        }

    }
}

void update_select_sets(fd_set *rd_set, fd_set *except_set)
{
    COMM_CHANNEL *pChn = get_used_channel_head();

    if (rd_set == NULL)
    {
        return;
    }

    FD_ZERO(rd_set);
    FD_ZERO(except_set);
    FD_SET(g_video_msg_sock, rd_set);
    FD_SET(g_video_msg_sock, except_set);

    while(pChn != NULL)
    {
        FD_SET(pChn->sock, rd_set);
        FD_SET(pChn->sock, except_set);
        pChn = get_next_channel(pChn);
    }
}

