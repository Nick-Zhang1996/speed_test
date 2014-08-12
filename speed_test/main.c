//
//  main.c
//  speed_test
//
//  Created by Nick zhang on 14-7-26.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

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
#include "inet_com.h"

const unsigned int buffersize=10000;
const unsigned int test_pkg_size=100000000;


int main(int argc, const char * argv[])
{
    int port_in,port_to;
/*
    assert(argc==3);
    {int e=asc2dec(argv[1], &port_in);assert(e==0);
        e=asc2dec(argv[2], &port_to);assert(e==0); }
*/
    port_in=2000;
    port_to=2000;
    fprintf(stdout, "main: ready to fork,port_to=%d,port_in=%d\n",port_to,port_in);
    
    if(fork()!=0) {client(port_to);}
    else {server(port_in);}
    
    
    return 0;
}

