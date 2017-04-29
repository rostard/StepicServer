#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include <fstream>

using std::endl;
using std::cout;

char* server_ip=(char*)"0.0.0.0";
uint16_t server_port=12345;
char* server_directory;

std::ofstream log;
void getParam(int argc, char** argv){
    int opt;
    while((opt=getopt(argc,argv,"h:p:d:"))!=-1){
        switch(opt) {
            case 'h':
                server_ip = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'd':
                server_directory = optarg;
                break;
            default:
                break;
        }
    }
}

int prepareSocket(char* ip,uint16_t port){
    log<<server_ip<<endl;
    log<<server_port<<endl;
    log<<server_directory<<endl;

    int fd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_aton(ip,&addr.sin_addr);

    int err = bind(fd,(sockaddr*)(&addr),sizeof(addr));

    if(err==-1){
        log<<"err in bind"<<endl;
        cout<<"err in bind"<<endl;
        exit(EXIT_FAILURE);
    }
    err=listen(fd,SOMAXCONN);
    if(err==-1){
        log<<"err in listen"<<endl;
        cout<<"err in listen"<<endl;
        exit(EXIT_FAILURE);
    }


    return fd;
}

int daemonize(){
    pid_t pid=fork();
    switch (pid){
        case 0:
            log<<"Child alive"<<endl;
            setsid();
            break;
        case -1:
            log<<"Error in daemon"<<endl;
            exit(EXIT_FAILURE);
        default:
            log<<"Main process ends"<<endl;
            log<<"pid = "<<pid<<endl;
            exit(EXIT_SUCCESS);
    }
    return 1;
}

int main(int argc, char** argv) {
    getParam(argc,argv);
    log.open("log.txt");

    int fd=prepareSocket(server_ip,server_port);

    daemonize();

    int conn_fd=accept(fd,NULL,NULL);
    log<<"accepted"<<endl;

    char buf[1024];
    while(recv(conn_fd,buf,1024,0)>0){
        log<<buf<<endl;
    }

    log.close();
    return 0;
}
