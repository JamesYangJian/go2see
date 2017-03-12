#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include "sys_utils.h"
#include "transport_layer.h"
#include "sys_conf.h"

#include "transport.h"


static int g_sys_msg_sock = -1;
static int g_max_fd = 0;
static COMM_CHANNEL *gp_video_server_channel = NULL;

int module_transport_init()
{
    int msg_port = get_server_msg_port();
    int iRet = 0;
    unsigned int on = 1;

    
    g_sys_msg_sock = tl_create(TYPE_STREAM);
    if (g_sys_msg_sock <= 0)
    {
        SYS_LOG_PRINTF("Create system message socket failed!\n");
        return -1;
    }

    setsockopt(g_sys_msg_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    iRet = tl_open_listen(g_sys_msg_sock, (unsigned short)msg_port);
    if (iRet < 0)
    {
        tl_destroy(g_sys_msg_sock);
        return -1;
    }

    init_channel_list();

    connect_to_video_server();
    
    return 0; 
}

void connect_to_video_server()
{
    int video_server_sock = -1;
    char *pVideoSvrIP = NULL;
    int iVideoSvrPT = 0;
    struct timeval tv;
    unsigned int on = 1;
    int iRet = 0;

    //connect to video_server
    video_server_sock = tl_create(TYPE_STREAM);
    if (video_server_sock <= 0)
    {
        SYS_LOG_PRINTF("Create video server socket failed!\n");
        return;
    }
    setsockopt(video_server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
    pVideoSvrIP = get_video_server_ip();
    iVideoSvrPT  = get_video_server_port();
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    iRet = tl_connect(video_server_sock, inet_addr(pVideoSvrIP), (unsigned short)iVideoSvrPT, &tv);
    if (iRet < 0)
    {
        SYS_LOG_PRINTF("Connect video server failed!\n");
        return;
    }

    if (gp_video_server_channel == NULL)
    {
        gp_video_server_channel = (COMM_CHANNEL *)malloc(sizeof(COMM_CHANNEL));
        if (gp_video_server_channel == NULL)
        {
            SYS_LOG_PRINTF("Fatal!Malloc video_server_channel failed!\n");
            exit(0);
        }
    }
    gp_video_server_channel->channel_id = M_VIDEO_CHANNEL_ID;
    gp_video_server_channel->bUsed = 1;
    gp_video_server_channel->sock = video_server_sock;
    g_max_fd = M_MAX(g_max_fd, video_server_sock);
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
    
    g_max_fd = g_sys_msg_sock;
    //accept message port and read channel data
    while (1)
    {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if (gp_video_server_channel == NULL || gp_video_server_channel->bUsed == 0)
        {
            connect_to_video_server();
        }
        update_select_sets(&rd_set, &except_set);
        

        iRet = select(g_max_fd+1, &rd_set, NULL, &except_set, &tv);
        printf("%s\n", strerror(errno));
        if (iRet > 0)
        {
            if (FD_ISSET(g_sys_msg_sock, &rd_set))
            {
                new_sock = accept(g_sys_msg_sock, NULL, NULL);
                if (new_sock > 0)
                {
                    SYS_LOG_PRINTF("Accept successfully, create new channel!\n");
                    FD_SET(new_sock, &rd_set);
                    g_max_fd = M_MAX(g_max_fd, new_sock);
                    
                    pChn = apply_channel();
                    if (pChn != NULL)
                    {
                        pChn->sock = new_sock;
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

    if (rd_set == NULL)
    {
        return;
    }

    //receive messages from video server
    if (gp_video_server_channel  != NULL && gp_video_server_channel->bUsed == 1)
    {
        if  (FD_ISSET(gp_video_server_channel->sock, except_set))
        {
            tl_destroy(gp_video_server_channel->sock);
            gp_video_server_channel->bUsed = 0;
        }
        else if (FD_ISSET(gp_video_server_channel->sock, rd_set))
        {
            iRet = channel_recv_tlv_msg(gp_video_server_channel);
            if (iRet <0)
            {
                tl_destroy(gp_video_server_channel->sock);
                gp_video_server_channel->bUsed = 0;
            }
        }
    }
        

    while (pChn != NULL)
    {
        if (FD_ISSET(pChn->sock, except_set))
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
    FD_SET(g_sys_msg_sock, rd_set);
    FD_SET(g_sys_msg_sock, except_set);
    if (gp_video_server_channel != NULL && gp_video_server_channel->bUsed == 1)
    {
        FD_SET(gp_video_server_channel->sock, rd_set);
        FD_SET(gp_video_server_channel->sock, except_set);
    }

    while(pChn != NULL)
    {
        FD_SET(pChn->sock, rd_set);
        FD_SET(pChn->sock, except_set);
        pChn = get_next_channel(pChn);
    }
}

COMM_CHANNEL *get_video_server_channel()
{
    return gp_video_server_channel;
}
