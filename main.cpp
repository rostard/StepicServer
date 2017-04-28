#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

char* serv_ip="127.0.0.1";
int serv_port=11180;
char* serv_directory;

void getParam(int argc, char** argv){
    char opt;
    while((opt=getopt(argc,argv,"h:p:d:"))!=-1){
        switch(opt) {
            case 'h':
                serv_ip = optarg;
                break;
            case 'p':
                serv_port = atoi(optarg);
                break;
            case 'd':
                serv_directory = optarg;
                break;
        }
    }
}

int main(int argc, char** argv) {
    //getParam(argc,argv);
    int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(serv_port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //inet_aton(serv_ip,&addr.sin_addr);
    int err= bind(fd,(sockaddr*)(&addr),sizeof(addr));
    std::cout<<err;
    err=listen(fd,10);
    std::cout<<err;
    int conn_fd=accept(fd,NULL,NULL);
    char buf[1024];
    while(true){
        int len=recv(conn_fd,buf,1024,0);
        std::cout<<buf<<std::endl;
    }

    std::cout << "Hello, World!" << std::endl;

    return 0;
}