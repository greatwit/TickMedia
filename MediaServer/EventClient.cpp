/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>

#include "TcpServer.hpp"
#include "TcpClient.hpp"
#include "ActorStation.hpp"

#define PORT 31000

void usage(char*exename) {
	printf("Usage: %s -v filepath\n -f remotefile savefile", exename);
}

#define IPADDR "192.168.1.107"		//"127.0.0.1"

int main( int argc, char * argv[] )
{
	int port = PORT, maxThreads = 10;
	//const char * serverType = "hahs";

	if(argc<2) {
		usage(argv[0]);
		return 0;
	}

	extern char *optarg ;
	int c ;

	while( ( c = getopt ( argc, argv, "p:t:s:v" )) != EOF ) {
		switch ( c ) {
			case 'p' :
				port = atoi( optarg );
				break;
			case 't':
				maxThreads = atoi( optarg );
				break;
			case 's':
				//serverType = optarg;
				break;
			case '?' :
			case 'v' :
				//printf( "Usage: %s [-p <port>] [-t <threads>] [-s <hahs|lf>]\n", argv[0] );
				//exit( 0 );
				break;
		}
	}


	ActorStation station;
	station.startup();

	TcpClient client;
	if(!strcmp(argv[1], "-v")&&argc==3)
		client.connect(IPADDR, port, (const char*)argv[2]);			//127.0.0.1 192.168.1.108
	else if(!strcmp(argv[1], "-f")&&argc==4)
		client.connect(IPADDR, port, (const char*)argv[2], (const char*)argv[3]);
	else
		usage(argv[0]);

	client.registerEvent(station.getEventArg());

	getchar();

	client.disConnect();
	station.shutdown();

	return 0;
}



