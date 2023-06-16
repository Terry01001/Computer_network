#include"TCP.h"
#include"Header.h"
void initial(int port);
void run_client();

int main(int argc, char** argv){
    srand(time(NULL));
    if(argc==3){
        send_ip=argv[1];
		send_port = atoi(argv[2]);
	    initial(1024+rand()%64512);
    }
    else{
        cout << "Please input server's ip and port" << endl;
		exit(1);
    }

    thread client_thread(run_client);
	client_thread.join();
	

}


void initial(int port){
    client_send_byte=1;
    server_send_byte=1025;
    from_ip = LOCAL_IP;
    from_port = port;
}
void run_client(){
    create_client();
    client_connect();
    cout<<"finish"<<endl;

}
