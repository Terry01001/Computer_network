#ifndef HEADER_H
#define HEADER_H

#include<iostream>
#include<vector>
#include<cstring>
#include<cstdlib>//atoi
#include<cstdio>//error
#include<ctime>
#include<sys/types.h>
#include<sys/socket.h>//socket
#include<netdb.h>//struct addrinfo
#include<netinet/in.h>
#include<arpa/inet.h>
#include<functional>//bind
#include<fstream>
#include<errno.h>
#include<unistd.h>
#include<math.h>
#include<thread>
//#include<pthread.h>


#define RTT 20
#define THRESHOLD 65536
#define MSS 1024
#define BUFFER_SIZE 524288
#define LOCAL_IP "127.0.0.1"
//#define FILE_SIZE 10240

using namespace std;

extern string server_ip;
extern string from_ip, send_ip;
extern int server_port;
extern int send_port,from_port;
extern int server_sockfd, client_sockfd;

extern struct sockaddr_in server_addr,client_addr; //netinet/in.h
extern struct sockaddr_in from_sock, send_sock;

extern int server_seq_num,client_seq_num;
extern int client_send_byte,server_send_byte;
extern int serveracknum_testpktloss;

//extern char file_name[10][50];//10 file,file name at most 50 characters
extern vector<unsigned int> lostacknum;

extern vector<string> file_name;
extern int file_num;

typedef struct tcp_header
{
	unsigned short src_port;
	unsigned short des_port;
	unsigned int seq_num;
	unsigned int ACK;
	unsigned short offset_flag;	//data offset(4),no used(6),flag(6)
								//flag : URG ACK PSH RST SYN FIN
	unsigned short window;
	unsigned short checksum;
	unsigned short urg_ptr;
	int option;
} TCP_header;

typedef struct tcp_pkt
{
	TCP_header Header;
	char data[MSS];
} TCP_pkt;


bool get_URG_flag(TCP_header);
bool get_ACK_flag(TCP_header);
bool get_PAH_flag(TCP_header);
bool get_RST_flag(TCP_header);
bool get_SYN_flag(TCP_header);
bool get_FIN_flag(TCP_header);


/*
netinet/in.h
IPv4 socket

struct sockaddr_in {
	short            sin_family;   // AF_INET,becase this is IPv4;
	unsigned short   sin_port;     // save port No
	struct in_addr   sin_addr;
	char             sin_zero[8];  // Not used
};

struct in_addr {
	unsigned long s_addr;          // load with inet_pton()
};

*/



//int sendto(int s, const void *buf, int len, unsigned int flags, const struct sockaddr *to, int tolen);
//int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr* from , socklen_t *fromlen)
//char* inet_ntoa(struct in_addr);
//u_short PASCAL FAR htons( u_short hostshort);    //host to net short int
#endif
