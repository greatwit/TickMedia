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

int main( int argc, char * argv[] )
{
	int port = PORT, maxThreads = 10;
	//const char * serverType = "hahs";

	if(argc<2) {
		printf("Usage: %s filepath\n", argv[0]);
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
				printf( "Usage: %s [-p <port>] [-t <threads>] [-s <hahs|lf>]\n", argv[0] );
				exit( 0 );
				break;
		}
	}


	ActorStation station;
	station.startup();

	TcpClient client;
	client.connect("127.0.0.1", port, (const char*)argv[1]);			//127.0.0.1 192.168.1.108
	client.registerEvent(station.getEventArg());

	getchar();

	client.disConnect();
	station.shutdown();

	return 0;
}



