#include"command_function.h"
#include"Header.h"


void client_send_command(TCP_pkt p){//send command / receive result /send ack
    TCP_pkt send_pkt=p, rcv_pkt;
    socklen_t len = sizeof(server_addr);


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

    if(strncmp(send_pkt.data,"get",3)==0){
        firstgetcommand=1;
        cout << send_pkt.data << " from " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
    }
        


	sendto(client_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&send_sock, sizeof(send_sock));//send command


    int count_recvpacket=-1;
    while(1){
        if(recvfrom(client_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&send_sock, (socklen_t*)&len) != -1){
            if(strncmp(send_pkt.data,"cal",3)==0){
                cout << send_pkt.data << " by " << LOCAL_IP << " : " << send_pkt.Header.des_port << endl;
                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                cout << rcv_pkt.data << endl;
                serveracknum_testpktloss=rcv_pkt.Header.ACK;
                client_math_ack(); // send ack
                return;
            }
            else if(strncmp(send_pkt.data,"get",3)==0){//send command / receive video packet / send ack(file size)
                count_recvpacket++;
                
                if(firstgetcommand!=1 && count_recvpacket>0 && (count_recvpacket)%2==1 && rcv_pkt.Header.option!=-1){//delay ack
                    server_send_byte+=1024;
                    cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                    //serveracknum_testpktloss=rcv_pkt.Header.ACK;
                    continue;
                } 
                /*
                if((rcv_pkt.Header.ACK-1) != serveracknum_testpktloss && firstgetcommand != 1){//receive two packets
                    cout<<"Packet loss: ack => "<<rcv_pkt.Header.ACK - 1 <<endl;
                    for(unsigned int i=rcv_pkt.Header.ACK-1;i>serveracknum_testpktloss;i--){
                        lostacknum.push_back(i);
                        client_seq_num=rcv_pkt.Header.ACK;
                        server_seq_num=rcv_pkt.Header.seq_num;
                        //server_send_byte+=1024;
                    }
                
                }*/

                if(rcv_pkt.Header.option == -1){
                /*
                    cout<<"lost ack num: ";
                    for(int i=0;i<lostacknum.size();i++){
                        cout<< lostacknum[i] <<" ";
                    }
                    cout<<endl;
                    lostacknum.clear();
                    serveracknum_testpktloss=rcv_pkt.Header.ACK;
                */
                    client_video_ack(rcv_pkt);
                    cout << "Finish transimission"<<endl;
                    return;
                }
                
                firstgetcommand=0;
                count_recvpacket=0;

                cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                client_video_ack(rcv_pkt);//send delay ack
                
            }
            else if(strncmp(send_pkt.data,"dns",3)==0){//send command / receive result / send ack
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
            
            client_video_ack(rcv_pkt);//send delay ack
            
                
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
    
    int loss_p=1000000;   //i can set 1000000  but for test easily i set 40
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

    int count_sentpacket=-1;
    while(1){//send video
        if(need_send_byte!=0){
            count_sentpacket++;
            loss_rate=rand()%loss_p;
            if(count_sentpacket<=1 || loss_rate!=0){
                
                if(need_send_byte<send_byte){send_byte=need_send_byte;}
                memset(send_pkt.data,'\0',sizeof(send_pkt.data));
                

                send_pkt.Header.src_port = server_port;
                send_pkt.Header.des_port = rcv_pkt.Header.src_port;

                if(count_sentpacket==2)
                    send_pkt.Header.seq_num = rcv_pkt.Header.ACK+1;
                else
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
                    send_pkt.Header.option = 0;
                   

                sendto(server_sockfd, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&client_addr, len);      
                

                if(count_sentpacket<=1){
                    cout<<"\tSend a packet at : "<< rcv_pkt.Header.option <<" byte"<<endl;
                }
                else
                    cout<<"\tSend a packet at : "<< rcv_pkt.Header.option+1024 <<" byte"<<endl;
                
                lost=0;
            }
            else{
                cout<<"\tSend a packet at : "<< rcv_pkt.Header.option + 2048 <<" byte"<<endl;
                lost = 1;
            }
            

            while(1){//wait for ack
                if(send_pkt.Header.option==-1){
                    if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
                        if(get_ACK_flag(rcv_pkt.Header)){
                            count_sentpacket=0;
                            cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
                            break;
                        }
                    }
                //break;
                }
                if(count_sentpacket>0 && (count_sentpacket%2) ==1 ){//if count_sentpacke==1 then break(continue to send)
                    break;
                }
                if(recvfrom(server_sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*)&client_addr, (socklen_t*) &len) != -1){
                    //if(rcv_pkt.Header.option==-2){
                    //    lost=1;
                    //    count_sentpacket=0;
                    //    break;
                    //}
                    if(get_ACK_flag(rcv_pkt.Header)){
                        count_sentpacket=0;
                        cout << "\tReceive a packet (seq_num = " << rcv_pkt.Header.seq_num << ", ack_num = " << rcv_pkt.Header.ACK << ")" << endl;
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

    }
    fclose(file);
    cout<<"==Video transmission complete=="<<endl;
    return;

}

void client_video_ack(TCP_pkt p){
    TCP_pkt send_pkt,rcv_pkt=p;

    memset(send_pkt.data, '\0', sizeof(send_pkt.data));
    send_pkt.Header.src_port = from_port;
    send_pkt.Header.des_port = send_port;
    send_pkt.Header.seq_num = rcv_pkt.Header.ACK;
    client_seq_num=send_pkt.Header.seq_num;

    send_pkt.Header.ACK = server_seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    send_pkt.Header.offset_flag = 16;//ACK(16)
    server_send_byte+=1024;
    send_pkt.Header.option = server_send_byte;

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
    //client_seq_num=send_pkt.Header.seq_num;
    send_pkt.Header.ACK = server_seq_num + 1;
    server_seq_num=send_pkt.Header.ACK;
    //send_pkt.Header.offset_flag = 16;//ACK(16)
//server_send_byte+=1024;
    send_pkt.Header.option = -2;

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

