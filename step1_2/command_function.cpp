#include"command_function.h"
#include"Header.h"


void client_send_command(TCP_pkt p){//send command / receive result /send ack
    TCP_pkt send_pkt=p, rcv_pkt;
    socklen_t len = sizeof(server_addr);

    cout<<"Send a command"<<endl;
    cout<< "\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    client_send_byte+=1024;
    int firstsend=1;
    
        


	sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, sizeof(send_sock));

    while(1){
        if(recvfrom(client_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&send_sock, (socklen_t*)&len) != -1){
            if(strncmp(send_pkt.data,"cal",3)==0){
                char *firstop;
                char *op;
                char *secondop;
                firstop = strtok(send_pkt.data," ");
                firstop = strtok(NULL," ");
                op = strtok(NULL," ");
                if(strcmp(op,"sqrt")!=0)
                	secondop = strtok(NULL," ");
                if(strcmp(op,"sqrt")==0){
			        cout << "Calculating "<<firstop<<" "<<op << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
		        }
                else
                	cout << "Calculating "<<firstop<<" "<<op<<" "<<secondop << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << rcv_pkt.data << endl;
                client_math_ack(); // send ack
                return;
            }
            else if(strncmp(send_pkt.data,"get",3)==0){//send command / receive video packet / send ack(file size)
                if(firstsend==1){
                    char *filename;
                    filename=strtok(send_pkt.data," ");
                    filename=strtok(NULL," "); 
                    cout << "Receive video: "<<filename << " from " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                    firstsend=0;
                }
                
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                client_video_ack();//send ack
                if(rcv_pkt.Header.option == -1){
                    cout << "Finish transimission"<<endl;
                    return;
                }
            }
            else if(strncmp(send_pkt.data,"dns",3)==0){//send command / receive result / send ack
                char *domain_name;
                domain_name = strtok(send_pkt.data," ");
                domain_name = strtok(NULL," ");
                cout << "Finding IP address of "<< domain_name << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << rcv_pkt.data << endl;
                client_dns_ack();//send ack
                cout<<"Finish finding"<<endl;
                cout<<domain_name<<endl;
                return;
            }
        }
    }

}

void server_rcv_command(TCP_pkt p){//receive command /send result / recieve ack
    congestion_window=1;
    TCP_pkt send_pkt, rcv_pkt = p;

    cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
    cout << rcv_pkt.data << endl;
    if(strncmp(rcv_pkt.data,"cal",3)==0){
        congestion_window*=2;
        server_math(rcv_pkt);
    }
    else if(strncmp(rcv_pkt.data,"get",3)==0){//receive command /send video packet / recieve ack(file size)
        server_video(rcv_pkt);
    }
    else if(strncmp(rcv_pkt.data,"dns",3)==0){//receive command /send result / recieve ack
        congestion_window*=2;
        server_dns(rcv_pkt);
    }
}
void server_dns(TCP_pkt p){//send result / recieve ack
    TCP_pkt send_pkt, rcv_pkt = p;

    char *domain_name;
    domain_name = strtok(rcv_pkt.data," ");
    domain_name = strtok(NULL," ");
    cout << "==Start finding: " << domain_name << " to "<<LOCAL_IP<<" : " << rcv_pkt.Header.src_port <<endl;


    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = rcv_pkt.Header.des_port;
    send_pkt.Header.des_port = rcv_pkt.Header.src_port;
    
    send_pkt.Header.seq_num = rcv_pkt.Header.ACK;

    send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
    client_seq_num=send_pkt.Header.ACK;
    server_seq_num=send_pkt.Header.seq_num;
    
    strcat(send_pkt.data,"Result: ");

    //gethostbyname start
    struct hostent *host;

    if((host = gethostbyname(domain_name))==NULL){
        cout << "gethostbyname error"<<endl;
        exit(1);
    }
    strcat(send_pkt.data,inet_ntoa(*(struct in_addr*)host->h_addr));//network addr translate into "."

    //gethostbyname over
    socklen_t len = sizeof(client_addr);
    //send packet
    cout << "cwnd = " << congestion_window << ", rwnd = "<<rcv_pkt.Header.window << ", threshold = " << ssthresh <<endl;
    cout<<"\tSend a packet at : " << rcv_pkt.Header.option <<" byte"<<endl;
    sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len); 
    memset(rcv_pkt.data, '\0', sizeof(rcv_pkt.data));

    while(1){//wait ack
        if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
            if(get_ACK_flag(rcv_pkt.Header)){
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << "==Finish finding=="<<endl;
                break;
            }
        }
    }
        
}

void client_dns_ack(){//send ack
    TCP_pkt send_pkt;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = client_seq_num + 1;
    client_seq_num=send_pkt.Header.seq_num;
    send_pkt.Header.ACK = server_seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)

    socklen_t len = sizeof(server_addr);
    
    cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    client_send_byte+=1024;

    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);
}



void server_video(TCP_pkt p){//send video packet / receive ack(file size)
    TCP_pkt send_pkt, rcv_pkt = p;
    char *token;
    char *filename;
    token = strtok(rcv_pkt.data," ");
    filename = strtok(NULL," ");

    
    cout<<"==Sending video: " << filename <<" to " << LOCAL_IP <<" : " << rcv_pkt.Header.src_port <<endl;

    ifstream f;
    //count bytes
    f.open(filename,ios::binary|ios::in|ios::ate);
    if(!f){
        cout<<"file open error"<<endl;
        exit(1);
    }
    int file_size = f.tellg();
    f.close();

    FILE* file;
    file = fopen(filename,"rb");
    //initial sen_pkt
    
    socklen_t len = sizeof(client_addr);
    int need_send_byte=file_size;
    int send_byte=1024;

    while(1){
        if(need_send_byte!=0){
            if(need_send_byte<send_byte){send_byte=need_send_byte;}
            memset(send_pkt.data,'\0',sizeof(send_pkt.data));
            send_pkt.Header.src_port = server_port;
            send_pkt.Header.des_port = rcv_pkt.Header.src_port;

            send_pkt.Header.seq_num = rcv_pkt.Header.ACK;

            send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
            client_seq_num=send_pkt.Header.ACK;
            server_seq_num=send_pkt.Header.seq_num;

            //read file
            fread((char*)send_pkt.data,1,send_byte,file);

            need_send_byte-=send_byte;
            file_size-=send_byte;
            
            if(file_size==0)
                send_pkt.Header.option = -1;
            else 
                send_pkt.Header.option = 1;           

            cout<<"\tSend a packet at : "<< rcv_pkt.Header.option <<" byte"<<endl;
            sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len);

            while(1){//wait for ack
                if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
                    if(get_ACK_flag(rcv_pkt.Header)){
                        cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                        break;
                    }
                }
            }
            
        }
        else{break;}

    }
    fclose(file);
    cout<<"==Video transmission complete=="<<endl;
    return;

}

void client_video_ack(){
    TCP_pkt send_pkt;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = client_seq_num + 1;
    client_seq_num=send_pkt.Header.seq_num;
    send_pkt.Header.ACK = server_seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)
    server_send_byte+=1024;
    send_pkt.Header.option = server_send_byte;


    socklen_t len = sizeof(server_addr);
    
    cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    
    client_send_byte+=1024;

    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);
}

void server_math(TCP_pkt p){//send result / receive ack
    TCP_pkt send_pkt, rcv_pkt = p;

    cout << "==Start calculation to " << LOCAL_IP <<" : " << rcv_pkt.Header.src_port <<endl;
    char *token;
    token = strtok(rcv_pkt.data," ");
    char *firstop;
    char *op;
    char *secondop;
    firstop = strtok(NULL," ");
    op = strtok(NULL," ");
    secondop = strtok(NULL," ");

    //send result back to client
    
    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = rcv_pkt.Header.des_port;
    send_pkt.Header.des_port = rcv_pkt.Header.src_port;
    
    send_pkt.Header.seq_num = rcv_pkt.Header.ACK;

    send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
    client_seq_num=send_pkt.Header.ACK;
    server_seq_num=send_pkt.Header.seq_num;
    
    strcat(send_pkt.data,"Result: ");

    cout << "cwnd = " << congestion_window << ", rwnd = "<<rcv_pkt.Header.window << ", threshold = " << ssthresh <<endl;
    if(strcmp(op,"add")==0){
        strcat(send_pkt.data,to_string(atoi(firstop)+atoi(secondop)).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    else if(strcmp(op,"sub")==0){
        strcat(send_pkt.data,to_string(atoi(firstop)-atoi(secondop)).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    else if(strcmp(op,"mul")==0){
        strcat(send_pkt.data,to_string(atoi(firstop)*atoi(secondop)).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    else if(strcmp(op,"div")==0){
        strcat(send_pkt.data,to_string(atoi(firstop)/atoi(secondop)).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    else if(strcmp(op,"pow")==0){
        strcat(send_pkt.data,to_string(pow(atoi(firstop),atoi(secondop))).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    else if(strcmp(op,"sqrt")==0){
        strcat(send_pkt.data,to_string(sqrt(atoi(firstop))).c_str());
        cout<<"\tsend a packet at : "<<rcv_pkt.Header.option<<" byte"<<endl;
    }
    //server_send_byte+=1024;
    socklen_t len = sizeof(client_addr);
    //send packet
    sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len); 
    memset(rcv_pkt.data, '\0', sizeof(rcv_pkt.data));

    while(1){//wait ack
        if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
            if(get_ACK_flag(rcv_pkt.Header)){
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << "==Finish calculation=="<<endl;
                break;
            }
        }
    }

}

void client_math_ack(){
    TCP_pkt send_pkt;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = client_seq_num + 1;
    client_seq_num=send_pkt.Header.seq_num;
    send_pkt.Header.ACK = server_seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)

    socklen_t len = sizeof(server_addr);
    
    cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    cout<<"Finish calculation" << endl;
    client_send_byte+=1024;

    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);
}

