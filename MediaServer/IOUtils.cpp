

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include "IOUtils.hpp"
#include "basedef.h"

#define  MAX_MTU 1400

void IOUtils :: inetNtoa( in_addr * addr, char * ip, int size )
{
	#if defined (linux) || defined (__ANDROID__) || defined (__sgi) || defined (__hpux) || defined (__FreeBSD__)
		const unsigned char *p = ( const unsigned char *) addr;
		snprintf( ip, size, "%i.%i.%i.%i", p[0], p[1], p[2], p[3] );
	#else
		snprintf( ip, size, "%i.%i.%i.%i", addr->s_net, addr->s_host, addr->s_lh, addr->s_impno );
	#endif
}

int IOUtils :: setNonblock( int fd )
{
	int flags;

	flags = fcntl( fd, F_GETFL );
	if( flags < 0 ) return flags;

	flags |= O_NONBLOCK;
	if( fcntl( fd, F_SETFL, flags ) < 0 ) return -1;

	return 0;
}

int IOUtils :: setBlock( int fd )
{
	int flags;

	flags = fcntl( fd, F_GETFL );
	if( flags < 0 ) return flags;

	flags &= ~O_NONBLOCK;
	if( fcntl( fd, F_SETFL, flags ) < 0 ) return -1;

	return 0;
}

int IOUtils :: tcpConnect(const char *destip, int destport, int * fd, int tcpdelay)
{
	int ret = 0;
    	int sockid = socket(AF_INET, SOCK_STREAM, 0);
    	if(sockid < 0) {
    		GLOGE("socket() failure val:%d", sockid);
    		return -1;
    	}



	if( 0 == tcpdelay ) {
		int flags = 1;
		if( setsockopt( sockid, IPPROTO_TCP, TCP_NODELAY, (char*)&flags, sizeof(flags) ) < 0 ) {
			GLOGE("failed to set socket to nodelay" );
			ret = -1;
		}
	}

    	struct sockaddr_in addr;
    
    	addr.sin_family = AF_INET;
    	addr.sin_port   = htons(destport);
    	addr.sin_addr.s_addr = inet_addr(destip);

    	if((ret = connect(sockid, (struct sockaddr*)&addr, sizeof(addr)) ) < 0) {
    		GLOGE("connect() failure ret:%d\n", ret);
    	}
	if( 0 != ret && sockid >= 0 ) close( sockid );

	if( 0 == ret ) {
		* fd = sockid;
	}

	return ret;
}

int IOUtils :: tcpSendData(int fd, char*data, int len){
	int sendLen = len, iRet = 0;
	if(len<= MAX_MTU)
		iRet += send(fd, data, len,0);
	else {
		while(sendLen>0){
			if(sendLen<=MAX_MTU)
		     		iRet += send(fd, data+len-sendLen, sendLen,0);
			else  	
				iRet += send(fd, data+len-sendLen, MAX_MTU,0);
			sendLen -= MAX_MTU;
		}
	}
	return iRet;
}

int IOUtils :: tcpListen( const char * ip, int port, int * fd, int blocking )
{
	int ret = 0;

	int listenFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( listenFd < 0 ) {
		syslog( LOG_WARNING, "listen failed, errno %d, %s", errno, strerror( errno ) );
		ret = -1;
	}

	if( 0 == ret && 0 == blocking ) {
		if( setNonblock( listenFd ) < 0 ) {
			syslog( LOG_WARNING, "failed to set socket to non-blocking" );
			ret = -1;
		}
	}

	if( 0 == ret ) {
		int flags = 1;
		if( setsockopt( listenFd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof( flags ) ) < 0 ) {
			syslog( LOG_WARNING, "failed to set setsock to reuseaddr" );
			ret = -1;
		}
		if( setsockopt( listenFd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags) ) < 0 ) {
			syslog( LOG_WARNING, "failed to set socket to nodelay" );
			ret = -1;
		}
	}

	struct sockaddr_in addr;

	if( 0 == ret ) {
		memset( &addr, 0, sizeof( addr ) );
		addr.sin_family = AF_INET;
		addr.sin_port = htons( port );

		addr.sin_addr.s_addr = INADDR_ANY;
		if( '\0' != *ip ) {
			if( 0 != inet_aton( ip, &addr.sin_addr ) ) {
				syslog( LOG_WARNING, "failed to convert %s to inet_addr", ip );
				ret = -1;
			}
		}
	}

	if( 0 == ret ) {
		if( bind( listenFd, (struct sockaddr*)&addr, sizeof( addr ) ) < 0 ) {
			syslog( LOG_WARNING, "bind failed, errno %d, %s", errno, strerror( errno ) );
			ret = -1;
		}
	}

	if( 0 == ret ) {
		if( ::listen( listenFd, 5 ) < 0 ) {
			syslog( LOG_WARNING, "listen failed, errno %d, %s", errno, strerror( errno ) );
			ret = -1;
		}
	}

	if( 0 != ret && listenFd >= 0 ) close( listenFd );

	if( 0 == ret ) {
		* fd = listenFd;
		syslog( LOG_NOTICE, "Listen on port [%d]", port );
	}

	return ret;
}

