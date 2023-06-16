#include"threeway_handshake.h"
#include"Header.h"

bool client_threeway_handshake()
{
	TCP_pkt send_pkt, rcv_pkt;
	
	memset(send_pkt.data, '\0', sizeof(send_pkt.data));

	memset(&send_pkt, '\0', sizeof(send_pkt));
	send_pkt.Header.src_port = from_port;
	send_pkt.Header.seq_num = 1 + rand() % 10000;
	send_pkt.Header.ACK = 0;
	send_pkt.Header.offset_flag = 2;//SYN(2)

	cout << "=====Start the three-way handshake=====" << endl;

	socklen_t len = sizeof(server_addr);	
	sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock,len);

	cout << "Send a packet(SYN) to " << inet_ntoa(send_sock.sin_addr) << " : " << htons(send_sock.sin_port) << endl;
	cout << "\tSend a packet at : " << client_send_byte << " byte"<<endl;
	client_send_byte+=1024;

	while (1)
	{
		if (recvfrom(client_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&send_sock, (socklen_t*)&len) != -1)
		{
			if (get_SYN_flag(rcv_pkt.Header) && get_ACK_flag(rcv_pkt.Header))
			{
				//receive_window-=MSS;
				cout << "Receive a packet(SYN/ACK) from " << inet_ntoa(send_sock.sin_addr) << " : " << htons(send_sock.sin_port) << endl;
		    	cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK <<")"<< endl;
				break;
			}
		}
		else
		{
			perror("client_threeway_handshake error: ");
			return false;
		}
	}
	serveracknum_testpktloss=rcv_pkt.Header.ACK;

	send_pkt.Header.seq_num += 1;
	client_seq_num = send_pkt.Header.seq_num;
	send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
	server_seq_num = send_pkt.Header.ACK;
	send_pkt.Header.offset_flag = 16;//ACK(16)

	//strcpy((char*)send_pkt.data, file_name[0].c_str());

	sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);

	cout << "Send a packet(ACK) to " << LOCAL_IP << " : " << htons(send_sock.sin_port) << endl;
	cout << "\tSend a packet at : " << client_send_byte << " byte"<<endl;
	client_send_byte += 1024;
	
	cout << "=====Complete the three-way handshake=====" << endl;

	return true;
}

bool server_threeway_handshake(TCP_pkt p)
{
	TCP_pkt send_pkt, rcv_pkt = p;

	memset(send_pkt.data, '\0', sizeof(send_pkt.data));
	

	cout << "=====Start the three-way handshake=====" << endl;
	cout << "Receive a packet(SYN) from " << inet_ntoa(client_addr.sin_addr) << " : " << rcv_pkt.Header.src_port << endl;
	cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;

	send_pkt.Header.src_port = server_port;
	send_pkt.Header.seq_num = 1 + rand() % 10000;
	send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
	send_pkt.Header.offset_flag = 18;//ACK(16)+SYN(2)

	socklen_t len = sizeof(client_addr);
	sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len);
	congestion_window*=2;

	cout << "Send a packet(SYN/ACK) to " << inet_ntoa(client_addr.sin_addr) << " : " << rcv_pkt.Header.src_port << endl;
	cout << "\tSend a packet at : " << 1 << " byte"<<endl;
	while (1)
	{
		if (recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1)
		{
			if (get_ACK_flag(rcv_pkt.Header))
			{
				//file_name[0] = (char*)rcv_pkt.data;
				cout << "Receive a packet(ACK) from " << inet_ntoa(client_addr.sin_addr) << " : " << rcv_pkt.Header.src_port << endl;
				cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
				//server_seq_num = rcv_pkt.Header.seq_num + 1;
				server_seq_num = rcv_pkt.Header.ACK + 1;
				
				cout << "=====Complete the three-way handshake=====" << endl;
				return true;
			}
		}
		else
		{
			perror("server_threeway_handshake error: ");
			return false;
		}
	}
}

