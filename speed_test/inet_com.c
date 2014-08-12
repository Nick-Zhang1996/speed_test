//
//  inet_com.c
//  speed_test
//
//  Created by Nick zhang on 14-7-26.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//


#include "inet_com.h"

double start_send_time,end_send_time,start_rec_time,end_rec_time;

int send_data(int socket_fd,char* data,int data_count){
    
    int real_data_count=data_count+1;
    write(socket_fd, &real_data_count, sizeof(int));
    write(socket_fd, data, real_data_count);
    return 0;
}

#if defined(__MACH__) && defined(__APPLE__)

double my_get_time(){
    static double timeConvert = 0.0;
    if ( timeConvert == 0.0 )
    {
        mach_timebase_info_data_t timeBase;
        (void)mach_timebase_info( &timeBase );
        timeConvert = (double)timeBase.numer /
        (double)timeBase.denom /
        1000000000.0;
    }
    return (double)mach_absolute_time( ) * timeConvert;
}
#endif

int asc2dec(const char* const str,int* dec){
    int num_count=0;
    *dec=0;
    while (1) {
        char val=*(str+num_count);
        if (val=='\0') {
            break;
        }
        
        if (!isdigit(val)) {
            return -1;
        }
        
        *dec=*dec*10+C2I(val);
        num_count++;
    }
    
    return 0;
}

void client(int port){

    struct hostent* hostinfo;
    struct sockaddr_in name;
    int socket_fd;
    int input_fd;
    int errcd;
 
//the real code
//    hostinfo=gethostbyname("nickzhang1996.xicp.net");
//    assert(hostinfo!=NULL);
//   name.sin_addr=*((struct in_addr*) hostinfo->h_addr_list);
//local test code
    errcd=inet_aton("127.0.0.1", &name.sin_addr);
    assert(errcd!=0);

    name.sin_port=htons(port);
    name.sin_family=AF_INET;
    socket_fd=socket(PF_INET, SOCK_STREAM, 0);
    assert(socket_fd!=-1);
    fprintf(stdout, "client: client ready(pid=%d),waiting for server initialization\n",(int)getpid());
   
    sleep(2);
    
    
//connect to server
    
    if (connect(socket_fd, &name, sizeof(struct sockaddr_in))==-1){
        perror("client");
        fprintf(stderr, "%d",errno);
        close(socket_fd);
        return;
    }
    
    input_fd=open("/dev/urandom",O_RDONLY);
    
    fprintf(stdout, "client: starting to send data, des-port:%d blocksize=%d totaldata=%d\n",port,buffersize,test_pkg_size);
    
    {int errorcode=write_with_blk(socket_fd, input_fd, test_pkg_size);
        assert(errorcode==0);}
    
    close(socket_fd);
    close(input_fd);
    
    fprintf(stdout, "client: all data sent successfully, quit\n");
    return;
    
    
}


int write_with_blk(int output_fd,int input_fd,unsigned int byte_count){
    int remain_data=byte_count;
    bool is_first=1;
    while (remain_data!=0) {
        pkt_info this_pkt;
        char buffer[buffersize];
        struct timespec timebuffer;
        
        this_pkt.length=min(buffersize, remain_data);
#ifdef __linux__
        {int errorcode=clock_gettime(CLOCK_MONOTONIC,&timebuffer);
            assert(errorcode=0);
        }
        this_pkt.send_time=timebuffer.tv_nsec;
        
#endif
        
#ifdef __MACH__
        //comment out to save executive time,only the first and last pkt is marked with time
       // this_pkt.send_time=my_get_time();
        if (is_first) {
            this_pkt.send_time=my_get_time();
            is_first=0;
        }   else{
        this_pkt.send_time=0.0;
        }
#endif
        this_pkt.end=0;
        read(input_fd, buffer, this_pkt.length);
        write(output_fd,&this_pkt, sizeof(pkt_info));
        write(output_fd, buffer, this_pkt.length);
        remain_data-=this_pkt.length;
    }
    
    pkt_info end_pkt;
    end_pkt.length=0;
    end_pkt.end=1;
#ifdef __linux__
    {int errorcode=clock_gettime(CLOCK_MONOTONIC,&timebuffer);
        assert(errorcode=0);
        end_pkt.sendtime=timebuffer.tv_nsec;
    }
#endif

#ifdef __MACH__
    end_pkt.send_time=my_get_time();
    
#endif
    write(output_fd, &end_pkt, sizeof(pkt_info));
    

    
    return 0;
}




int read_with_blk(int input_fd,void* buffer,unsigned int buffer_size){
    unsigned int errcd;
    char* pointer=(char*)buffer;
    pkt_info this_pkt;

    bool discard_data=(buffer==NULL)?1:0;
    bool is_start_pkt=1;
    
   
    
    while (1) {
        errcd=read(input_fd, &this_pkt, sizeof(pkt_info));
        assert(errcd=sizeof(pkt_info));
        
        
        // save the START_SEND_TIME in a global variable,only once in each call
        if (is_start_pkt) {
            start_send_time=this_pkt.send_time;
             start_rec_time=my_get_time();
            is_start_pkt=0;
        }
        
        //all data received, break
        // save the END_SEND_TIME in a global variable,only once in each call
        if (this_pkt.end) {
            end_send_time=this_pkt.send_time;
            break;
        }
        
        if (!discard_data){
            assert((pointer+this_pkt.length)<((char*)buffer+buffer_size));
            errcd=read(input_fd, pointer, this_pkt.length);
            assert(errcd=this_pkt.length);
            pointer+=errcd;
        } else{
            char temp[buffersize];
            errcd=read(input_fd, temp, this_pkt.length);
            assert(errcd=this_pkt.length);
        }
    }
    
    end_rec_time=my_get_time();
    
    return 0;
        
}


void server(int port){
    
    int rval;
    struct sockaddr_in name;
    int service_socket_fd;
    int com_socket_fd;
    struct sockaddr_in client_addr;
    size_t client_addr_length;
    
    memset(&name, 0, sizeof(name));
    name.sin_family=AF_INET;
    name.sin_port=htons(port);
    
    int errcd=inet_aton("127.0.0.1", &name.sin_addr);
    assert(errcd!=0);
    
    service_socket_fd=socket(PF_INET, SOCK_STREAM, 0);
    if (service_socket_fd==-1) {
        perror("cannot create socket");
        exit(1);
    }
    
    rval=bind(service_socket_fd, &name, sizeof(struct sockaddr_in));
    if (rval!=0) {
        perror("cannot bind socket");
        exit(1);
    }
    
    rval=listen(service_socket_fd, 5);
    if (rval!=0) {
        perror("cannot bind socket");
        exit(1);
    }
    
    
    fprintf(stdout, "server; server started,listening on port %d pid %d\n",port,(int)getpid());

    com_socket_fd=accept(service_socket_fd, &client_addr, &client_addr_length);
    if (com_socket_fd==-1) {
        perror("fail to accpet conection");
        exit(1);
    }
    

    
    int client_port=ntohs(client_addr.sin_port);
    
    fprintf(stdout, "server: accepted connect request from port %d\n",client_port);
    
    int ecd=read_with_blk(com_socket_fd, NULL, 0);
    assert(ecd==0);
    
    close(com_socket_fd);
    close(service_socket_fd);
    
    double send_interval=end_send_time-start_send_time;
    double rec_interval=end_rec_time-start_rec_time;
    
    fprintf(stdout, "server: data reveived, send in %fs,rec in %fs\n ",send_interval,rec_interval);
    fprintf(stdout, "server:%fs between send and receive,total=%f\n",start_rec_time-start_send_time,end_rec_time-start_send_time);
    fprintf(stdout, "server: speed=%fMB/s\n",test_pkg_size/(end_rec_time-start_send_time)/1000000);
    fprintf(stdout, "server:job done,quitting\n");
    
    
    
    return;}






