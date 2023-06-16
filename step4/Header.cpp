#include"Header.h"


string server_ip;
string from_ip, send_ip;
int server_port;
int send_port,from_port;
int server_sockfd, client_sockfd;

struct sockaddr_in server_addr,client_addr;
struct sockaddr_in from_sock, send_sock;

int server_seq_num,client_seq_num;
int client_send_byte,server_send_byte;
int serveracknum_testpktloss;

//char file_name[10][50];//10 file,file name at most 50 characters
vector<unsigned int> lostacknum;

vector<string> file_name;
int file_num;

bool get_FIN_flag(TCP_header header){return header.offset_flag & 1;}
bool get_SYN_flag(TCP_header header){return (header.offset_flag >> 1) & 1;}
bool get_RST_flag(TCP_header header){return (header.offset_flag >> 2) & 1;}
bool get_PAH_flag(TCP_header header){return (header.offset_flag >> 3) & 1;}
bool get_ACK_flag(TCP_header header){return (header.offset_flag >> 4) & 1;}
bool get_URG_flag(TCP_header header){return (header.offset_flag >> 5) & 1;}
