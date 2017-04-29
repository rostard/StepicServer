#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include <fstream>
char* serv_ip=(char*)"0.0.0.0";
int serv_port=12345;
char* serv_directory;

void getParam(int argc, char** argv){
    int opt;
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
            default:
                break;
        }
    }
}

int main(int argc, char** argv) {
    getParam(argc,argv);
    std::ofstream out("/home/box/log.txt");
    out<<serv_ip<<std::endl;
    out<<serv_port<<std::endl;
    out<<serv_directory<<std::endl;
    int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(serv_port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //inet_aton(serv_ip,&addr.sin_addr);
    int err= bind(fd,(sockaddr*)(&addr),sizeof(addr));
    out<<err<<std::endl;
    std::cout<<err;
    err=listen(fd,10);
    std::cout<<err;
    out<<err<<std::endl;
    if(fork()) {
        std::cout<<"if";
        out << "Kill main" << std::endl;
        return 0;
    }
    out<<"Second proc"<<std::endl;
    int conn_fd=accept(fd,NULL,NULL);
    out<<"accepted"<<std::endl;
    char buf[1024];
    ssize_t len;
    while((len=recv(conn_fd,buf,1024,0))>0){
        out<<buf<<std::endl;
    }

    std::cout << "Hello, World!" << std::endl;
    out.close();
    return 0;
}
