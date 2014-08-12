//
//  inet_com.h
//  speed_test
//
//  Created by Nick zhang on 14-7-26.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#ifndef speed_test_inet_com_h

#define speed_test_inet_com_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef __MACH__

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>


#endif



#define min(a,b) (((a)<(b))? (a):(b))
#define C2I(val) val-48

extern const unsigned int buffersize;
extern const unsigned int test_pkg_size;
extern double start_send_time,end_send_time,start_rec_time,end_rec_time;

typedef char boolean;
typedef struct {
    double send_time;
    unsigned int length;
    bool end;
} pkt_info;

int asc2dec(const char* const str,int* dec);
void client(int port);
void server(int port);
int write_with_blk(int output_fd,int input_fd,unsigned int byte_count);

int read_with_blk(int input_fd,void* buffer,unsigned int buffer_size);

#if defined(__MACH__) && defined(__APPLE__)

double my_get_time();
#endif

#endif




