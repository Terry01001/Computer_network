#include"TCP.h"
#include"Header.h"
#include"command_function.h"

bool create_server(){
	congestion_window=MSS;

    server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);//socket(IPv4,UDP,protocol)
    if(server_sockfd<0){
        perror("server_sockfd build error");
        return false;
    }

    //initialize address 
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);//host to network short
	server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());//c_str to integer address

	if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("server_sockfd bind failed ");
		return false;
	}

	return true;

}

bool create_client(){
	receive_window=BUFFER_SIZE;

	client_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_sockfd < 0)
	{
		perror("client_sockfd failed: ");
		return false;
	}

	//initialize address 
	memset(&from_sock, '\0', sizeof(from_sock));
	from_sock.sin_family = AF_INET;
	from_sock.sin_port = htons(0);//host to network short
	from_sock.sin_addr.s_addr = inet_addr(from_ip.c_str());//c_str to integer address

	memset(&send_sock, '\0', sizeof(send_sock));
	send_sock.sin_family = AF_INET;
	send_sock.sin_port = htons(send_port);
	send_sock.sin_addr.s_addr = inet_addr(send_ip.c_str());

	if (bind(client_sockfd, (struct sockaddr*)&from_sock, sizeof(from_sock))==-1)
	{
		perror("client_sockfd bind failed ");
		return false;
	}
	return true;
}

bool server_listen(){
    TCP_pkt send_pkt, rcv_pkt;
	
	ssthresh = THRESHOLD;

	//initialize TCP packet
	memset(send_pkt.data, '\0', sizeof(send_pkt.data));
	send_pkt.Header.src_port = server_port;

	//"Listening for clients"
	socklen_t len = sizeof(client_addr);
	while (1)
	{
		//rcv_pkt 存儲資料的地方
		if (recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1)
		{
			//thread server_thread2(server_listen);
			//cout<<"hiinrecv"<<endl;
			if (get_SYN_flag(rcv_pkt.Header))//新的client進來都需要做一次handshake
			{
				if (server_threeway_handshake(rcv_pkt))
				{
					//server_port=server_port + 1 + rand()%1000;
					//break;
					continue;
				}
			}
			//deal with the command
			//cout<<"hiinrecv"<<endl;
			server_rcv_command(rcv_pkt);





		}
		else
		{
			perror("server_listen error: ");
		}
	}
	//server_send_data();


    //thread server_thread2(server_listen);
	/*while (1)
	{
		if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_addr, (socklen_t *)&len) != -1)
			if (get_FIN_flag(rcv_pkt.Header))
			{
				return true;
			}
	}*/
	return true;
}

bool client_connect()
{
    cout<<"Server's IP address: "<<send_ip<<endl;
    cout<<"Server's port: "<<send_port<<endl;

	TCP_pkt send_pkt;
	if (client_threeway_handshake())//finish handshake then type command
	{
		while(1){//type command: cal a add b | get 1.mp4 | dns google.com | exit
			cout<<"Enter command (type \'exit\' to close connection): ";
			memset(send_pkt.data, '\0', sizeof(send_pkt.data));
			string command;
			getline(cin,command);
			if(command=="exit"){return true;}

			//send command to server
			strcpy(send_pkt.data,command.c_str());
			send_pkt.Header.src_port = from_port;
			send_pkt.Header.des_port = send_port;

			client_seq_num+=1;
			send_pkt.Header.seq_num = client_seq_num;
			
			server_seq_num+=1;
			send_pkt.Header.ACK = server_seq_num;
			send_pkt.Header.option = server_send_byte;
			server_send_byte += 1024;
			send_pkt.Header.window = receive_window;

			client_send_command(send_pkt);
		}
        return true;
	}
	return false;
}