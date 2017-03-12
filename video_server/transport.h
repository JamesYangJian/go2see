#ifndef _TRANSPORT_H__
#define _TRANSPORT_H__

#include "channel.h"

#define M_VIDEO_CHANNEL_ID 0xffff

int module_transport_init();

void run();
void loop_channel_data(fd_set *rd_set, fd_set *except_set);
void loop_expire_channel();
void update_select_sets(fd_set *rd_set, fd_set *except_set);
COMM_CHANNEL *get_video_server_channel();
void connect_to_video_server();
#endif
