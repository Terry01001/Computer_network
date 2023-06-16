#include"TCP.h"
#include"Header.h"

void initial(int port);
void run_server();

int main(int argc, char* argv[]){
    srand(time(NULL));

    if(argc==2){
        initial(atoi(argv[argc-1]));
    }
    else{
        cout << "incorrect format" << endl;
		exit(1);
    }

    thread server_thread(run_server);
	server_thread.join();

}

void initial(int port){
    server_ip = LOCAL_IP;
    server_port = port;
}

void run_server(){
    create_server();
	server_listen();
}