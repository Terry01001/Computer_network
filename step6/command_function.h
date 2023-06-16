#ifndef COMMAND_FUNCTION_H
#define COMMAND_FUNCTION_H
#include"Header.h"

void client_send_command(TCP_pkt);
void server_rcv_command(TCP_pkt);
void server_dns(TCP_pkt);
void client_dns_ack();
void server_video(TCP_pkt);
void client_video_ack(TCP_pkt);
void client_video_loss(TCP_pkt);
TCP_pkt client_video_dupack(TCP_pkt);
void server_math(TCP_pkt);
void client_math_ack();


#endif
