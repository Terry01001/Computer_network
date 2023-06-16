#include"command_function.h"
#include"Header.h"


void client_send_command(TCP_pkt p){//send command / receive result /send ack
    TCP_pkt send_pkt=p, rcv_pkt, lastrcv_pkt;
    socklen_t len = sizeof(server_addr);
    int dup_flag=0;


    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 600000;
    if(connect(client_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect error");
        exit(1);
    }
    if(setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0){
        perror("setsockopt error");
        exit(1);
    }


    cout<<"Send a command"<<endl;
    cout<< "\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    client_send_byte+=1024;
    int firstgetcommand=0;

	sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, sizeof(send_sock));
    

    if(strncmp(send_pkt.data,"get",3)==0){
        firstgetcommand=1;
        char *filename;
        filename=strtok(send_pkt.data," ");
        filename=strtok(NULL," ");

        cout << "Receive video: "<< filename <<" from " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
    }

    while(1){
        if(recvfrom(client_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&send_sock, (socklen_t*)&len) != -1){
            if(strncmp(send_pkt.data,"cal",3)==0){
                receive_window-=MSS;
                cout << send_pkt.data << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << rcv_pkt.data << endl;
                serveracknum_testpktloss=rcv_pkt.Header.ACK;
                client_math_ack(); // send ack
                return;
            }
            else if(strncmp(send_pkt.data,"get",3)==0){//send command / receive video packet / send ack(file size)
                receive_window-=MSS;
                if(rcv_pkt.Header.ACK-1 != client_seq_num && firstgetcommand != 1){//packet loss then send duplicate ack
                    cout<<"Packet loss: ack => "<<rcv_pkt.Header.ACK - 1 <<endl;
                    for(unsigned int i=rcv_pkt.Header.ACK-1;i>client_seq_num;i--){
                        lostacknum.push_back(i);
                        // client_seq_num=rcv_pkt.Header.ACK;
                        // server_seq_num=rcv_pkt.Header.seq_num;
                        //server_send_byte+=1024;
                    }
                    client_seq_num+=1;
                    //send duplicate ack
                    for(int i=1;i<=3;i++){
                        if(i==1)
                            cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                        //if(i>=2){
                        //    receive_window-=MSS;
                        //}

                        if(i==3){
                        //    server_send_byte-=3072;
                            receive_window=BUFFER_SIZE;
                        }
                        lastrcv_pkt=client_video_dupack(rcv_pkt);
                        
                        
                        dup_flag=1;
                    }
                //continue;
                }
                /*
                //send duplicate ack
                if((rcv_pkt.Header.ACK) != client_seq_num && firstgetcommand != 1){
                    //cout<<"Packet loss: ack => "<<rcv_pkt.Header.ACK - 1 <<endl;
                    for(int i=1;i<=3;i++){
                        if(i>=2){
                            receive_window-=MSS;
                        }
                        if(i==3){
                            server_send_byte-=4096;
                            receive_window=BUFFER_SIZE;
                        }
                        client_video_dupack(rcv_pkt);
                    }
                    
                    continue;
                }
                */
                firstgetcommand=0;

                if(dup_flag!=1)
                    cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;

                if(dup_flag==1){
                    rcv_pkt=lastrcv_pkt;
                    dup_flag=0;
                }

                client_video_ack(rcv_pkt);//send ack

                if(rcv_pkt.Header.option == -1){
                    cout<<"lost ack num: ";
                    for(int i=0;i<lostacknum.size();i++){
                        cout<< lostacknum[i] <<" ";
                    }
                    cout<<endl;
                    lostacknum.clear();
                    serveracknum_testpktloss=rcv_pkt.Header.ACK;
                    cout << "Finish transimission"<<endl;
                    return;
                }
            }
            else if(strncmp(send_pkt.data,"dns",3)==0){//send command / receive result / send ack
                receive_window-=MSS;//record rwnd put it into 
                char *domain_name;
                domain_name = strtok(send_pkt.data," ");
                domain_name = strtok(NULL," ");
                cout << "Finding IP address of "<< domain_name << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << rcv_pkt.data << endl;
                serveracknum_testpktloss=rcv_pkt.Header.ACK;
                client_dns_ack();//send ack
                cout<<"Finish finding"<<endl;
                cout<<domain_name<<endl;
                return;
            }
        }
        else{
            client_video_loss(rcv_pkt);
        }
    }

}

void server_rcv_command(TCP_pkt p){//receive command /send result / recieve ack
    TCP_pkt send_pkt, rcv_pkt = p;

    cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
    cout << rcv_pkt.data << endl;
    if(strncmp(rcv_pkt.data,"cal",3)==0){
        server_math(rcv_pkt);
    }
    else if(strncmp(rcv_pkt.data,"get",3)==0){//receive command /send video packet / recieve ack(file size)
        server_video(rcv_pkt);
    }
    else if(strncmp(rcv_pkt.data,"dns",3)==0){//receive command /send result / recieve ack
        server_dns(rcv_pkt);
    }
}

void server_video(TCP_pkt p){//send video packet (twice) and  receive ack(file size)
    TCP_pkt send_pkt, rcv_pkt = p;
    
    server_send_byte=rcv_pkt.Header.option;
    server_seq_num=rcv_pkt.Header.ACK;
    client_seq_num=rcv_pkt.Header.seq_num + 1;//for ack

    int loss_p=1000000;   //i can set 1000000  but for test easily
    int loss_rate;
    int lost=0;
    int ret;

    char *filename;
    filename = strtok(rcv_pkt.data," ");
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

    int firstloss=1;
    int maxsend_packetnum=congestion_window/MSS;
    int CA=0;//1 if is the congestion state
    int duplicateack_num=1;
    int check_duplicate_acknum=0;
    cout <<"*****slow start*****"<<endl;
    while(1){//send video
        if(duplicateack_num==3){
            cout<<"Receive duplicate ACKs."<<endl;
            client_seq_num=rcv_pkt.Header.seq_num+1; // for send ack
            server_seq_num=rcv_pkt.Header.ACK;


            ssthresh=congestion_window/2;
            congestion_window=ssthresh+3*MSS;//fast recovery
            
            maxsend_packetnum=congestion_window/MSS;
            duplicateack_num=1;
            //cout<<"*****Fast retransmit*****"<<endl;
            cout<<"*****Fast recovery*****"<<endl;

        
            server_send_byte-=3*MSS;

            //cout<<"*****slow start*****"<<endl;
        }
        if(congestion_window>=ssthresh && maxsend_packetnum == congestion_window/MSS){
            cout <<"*****Congestion avoidance*****"<<endl;
            CA=1;
        }
        if(need_send_byte!=0){

            if(server_send_byte>=4096 && server_send_byte <5120){
                loss_p=1;
                firstloss=0;
            }
            else
                loss_p=1000000;

            maxsend_packetnum-=1;

            loss_rate=rand()%loss_p;
            if(loss_rate!=0){
                
                if(need_send_byte<send_byte){send_byte=need_send_byte;}
                memset(send_pkt.data,'\0',sizeof(send_pkt.data));
                send_pkt.Header.src_port = server_port;
                send_pkt.Header.des_port = rcv_pkt.Header.src_port;

                send_pkt.Header.seq_num = server_seq_num;

                send_pkt.Header.ACK = client_seq_num;
                client_seq_num+=1;
                server_seq_num+=1;
            
                //read file
                fread((char*)send_pkt.data,1,send_byte,file);

                need_send_byte-=send_byte;
                file_size-=send_byte;
            
                if(file_size==0)
                    send_pkt.Header.option = -1;
                else 
                    send_pkt.Header.option = 1;     

                sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len);      
                
                cout<<"cwnd = "<<congestion_window<<", rwnd = "<<rcv_pkt.Header.window<<", threshold = "<<ssthresh<<endl;
                if(lost==0)//not lost
                    cout<<"\tSend a packet at : "<< server_send_byte <<" byte"<<endl;
                else
                    cout<<"\tSend a packet at : "<< server_send_byte <<" byte"<<endl;
                

                lost=0;
            //server_send_byte+=1024;
            }
            else{
                server_seq_num+=1;
                client_seq_num+=1;
                cout<<"cwnd = "<<congestion_window<<", rwnd = "<<rcv_pkt.Header.window<<", threshold = "<<ssthresh<<endl;
                cout<<"\tSend a packet at : "<< server_send_byte <<" byte"<<endl;
                server_send_byte+=1024;
                lost = 1;
            }
            

            while(1){//wait for ack
                if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
                    if(rcv_pkt.Header.option==-2){
                        lost=1;
                        break;
                    }
                    if(get_ACK_flag(rcv_pkt.Header)){
                        if(rcv_pkt.Header.ACK == check_duplicate_acknum){
                            duplicateack_num++;
                        }
                        else
                            duplicateack_num=1;

                        cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                        check_duplicate_acknum=rcv_pkt.Header.ACK;
                        server_send_byte+=MSS;
                        break;
                    }
                    //if(recv(server_sockfd,&rcv_pkt,sizeof(rcv_pkt),0)<0 ) {//whether change to recv(client_sockfd,&rcv_pkt,sizeof(rcv_pkt),0)
                    //cout<<"lost:"<<lost<<endl;
                    
                    //break;
                    //}
                }
                else{
			        perror("server_no ack error");
			        exit(1);
		        }
            }
            
        }
        else{break;}

        if(duplicateack_num==3)
            continue;
        if(CA==0 && maxsend_packetnum==0){//slow state
            congestion_window*=2;
            maxsend_packetnum=congestion_window/MSS;
        }
        else if(CA==1&&maxsend_packetnum==0){//congestion state
            congestion_window = congestion_window + (MSS * MSS /congestion_window);
            maxsend_packetnum=congestion_window/MSS;
        }

    }
    fclose(file);
    cout<<"==Video transmission complete=="<<endl;
    return;

}

TCP_pkt client_video_dupack(TCP_pkt p){
    TCP_pkt send_pkt,rcv_pkt=p;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = client_seq_num;
    //client_seq_num=send_pkt.Header.seq_num;

    send_pkt.Header.ACK = server_seq_num;//previous packet was lost
//server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)
    server_send_byte+=1024;
    send_pkt.Header.option = server_send_byte;

    send_pkt.Header.window = receive_window;//congestion control
    receive_window-=MSS;

    serveracknum_testpktloss=send_pkt.Header.seq_num;


    socklen_t len = sizeof(server_addr);
    
    cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    
    client_send_byte+=1024;
    
    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);


    while(1){
        if(recvfrom(client_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&send_sock, (socklen_t*)&len) != -1){
            
            cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", " << " ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
            return rcv_pkt;
            break;
        }
    }
}


void client_video_ack(TCP_pkt p){
    TCP_pkt send_pkt,rcv_pkt=p;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = rcv_pkt.Header.ACK;
    client_seq_num=send_pkt.Header.seq_num;
    //client_seq_num=send_pkt.Header.seq_num;

    send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)
    server_send_byte+=1024;
    send_pkt.Header.option = server_send_byte;

    send_pkt.Header.window = receive_window;//congestion control

    serveracknum_testpktloss=send_pkt.Header.seq_num;


    socklen_t len = sizeof(server_addr);
    
    cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    
    client_send_byte+=1024;

    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);
}

void client_video_loss(TCP_pkt p){
    TCP_pkt send_pkt,rcv_pkt=p;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = rcv_pkt.Header.ACK + 1;
//client_seq_num=send_pkt.Header.seq_num;//ready to send duplicate ack
    send_pkt.Header.ACK = rcv_pkt.Header.seq_num + 2;
    server_seq_num=send_pkt.Header.ACK;
    //send_pkt.Header.offset_flag = 16;//ACK(16)
//server_send_byte+=1024;
    send_pkt.Header.option = -2;

    receive_window-=MSS;
    send_pkt.Header.window = receive_window;

    //serveracknum_testpktloss=send_pkt.Header.seq_num;


    socklen_t len = sizeof(server_addr);
    
    //cout<<"send a packet(ACK) to " << LOCAL_IP <<" : " << send_port <<endl;
    //cout<<"\tSend a packet at : " << client_send_byte<<" byte"<<endl;
    
    //client_send_byte+=1024;

    sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, len);
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

