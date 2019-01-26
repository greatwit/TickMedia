

#ifndef __ioutils_hpp__
#define __ioutils_hpp__

#include <netinet/in.h>

class IOUtils {
public:
	static void inetNtoa( in_addr * addr, char * ip, int size );

	static int setNonblock( int fd );

	static int setBlock( int fd );

	static int tcpListen( const char * ip, int port, int * fd, int tcpdelay = 1 );
	
   	static int tcpConnect(const char *destip, int destport, int * fd, int blocking = 1);
	//static int tcpSendPackage(int fd, char*data, int len);
	static int tcpSendData(int fd, char*data, int len);

private:
	IOUtils();
};

#endif

