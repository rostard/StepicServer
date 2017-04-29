#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include <sys/epoll.h>
#include <strings.h>
#include <thread>

#define MAX_EVENTS 20

using std::endl;
using std::cout;

std::ofstream log;



char* server_ip=(char*)"0.0.0.0";
uint16_t server_port=12345;
char* server_directory;

std::vector<std::string> split(std::string str, char delim){
    std::stringstream ss(str);
    std::string element;
    std::vector<std::string> ret;

    while(getline(ss,element,delim)){
        ret.push_back(element);
    }
    ret.resize(ret.size()-1);
    return ret;
}
bool me=false;
void getParam(int argc, char** argv){
    int opt;
    while((opt=getopt(argc,argv,"h:p:d:m"))!=-1){
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
            case 'm':
                me=true;
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
            umask(0);
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

void processHTTP(int socket, std::string data) {
    std::vector<std::string> lines = split(data, '\n');
    std::vector<std::string> head = split(lines[0], ' ');
    for (int i = 0; i < lines.size(); i++) {
        log << i + 1 << ": " << lines[i] << endl;
    }

    if (head[0] == "GET"){
        std::string resource=head[1];
        log<<resource<<endl;
        if(resource[0]=='/')resource=&resource[1];
        resource=resource.substr(0,resource.find('?'));
        log<<resource<<endl;

        std::string response;

        std::string header;
        std::string data;
        int size=1024*10;
        char buf[size];
        bzero(buf,size);
        FILE *f=fopen(resource.c_str(),"r");
        if(f){
            //fread((void*)data.data(),1024*20,1,f);
            fread(buf,size,1,f);
            data=buf;
            fclose(f);
            header= "HTTP/1.0 200 OK\x0D\x0A"
                            "Content-Type: text/html\x0D\x0A"
                            "Connection: close\x0D\x0A"
                            "Content-Length: "+std::to_string(data.size())+"\x0D\x0A\x0D\x0A";
            response=header+data;
            log<<response;
            send(socket,response.c_str(),response.size(),MSG_NOSIGNAL);
        }
        else{
            header= "HTTP/1.0 404 NOT FOUND\x0D\x0A"
                    "Content-Length: 0\x0D\x0A"
                    "Connection: close\x0D\x0A"
                    "Content-Type: text/html\x0D\x0A\x0D\x0A";
            send(socket,header.c_str(),header.size(),MSG_NOSIGNAL);

        }

    }
    close(socket);
}

int main(int argc, char** argv) {
    getParam(argc,argv);
    if(me)log.open("log.txt");
    else log.open("/home/box/log.txt");
    int fd=prepareSocket(server_ip,server_port);

    daemonize();
    chdir(server_directory);

    int Epoll = epoll_create1(0);

    epoll_event Event;
    Event.data.fd=fd;
    Event.events=EPOLLIN;
    epoll_ctl(Epoll,EPOLL_CTL_ADD,fd,&Event);

    for(int cnt=0;cnt<1000;cnt++){
        epoll_event Events[MAX_EVENTS];
        int N = epoll_wait(Epoll,Events,MAX_EVENTS,-1);
        #pragma omp parallel for num_threads(15)
        for(unsigned int i=0;i<N;i++){
            if(Events[i].data.fd==fd){
                int client = accept(fd,0,0);
                Event.data.fd=client;
                Event.events=EPOLLIN;
                epoll_ctl(Epoll,EPOLL_CTL_ADD,client,&Event);
            }
            else{
                char buf[1024];
                recv(Events[i].data.fd,buf,1024,0);
                //std::thread thr(processHTTP,Events[i].data.fd);
                processHTTP(Events[i].data.fd,buf);
            }
        }
    }

    while(true){
        int client=accept(fd,NULL,NULL);
        log<<"accepted"<<endl;

        char buf[1024];
        recv(client,buf,1024,0);
        processHTTP(client,buf);

    }
    close(fd);
    log.close();
    return 0;
}
