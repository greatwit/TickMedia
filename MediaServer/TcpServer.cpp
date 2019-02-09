

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>

#include "TcpServer.hpp"


#include "Session.hpp"

#include "IOUtils.hpp"

#include "config.h"

#include "basedef.h"

//#include "event_msgqueue.h"

TcpServer :: TcpServer( const char * bindIP, int port )
{
	snprintf( mBindIP, sizeof( mBindIP ), "%s", bindIP );
	mPort = port;
	mIsShutdown = 0;
	mIsRunning = 0;
	mListenFD = 0;


	mMaxThreads 	= 64;
	mReqQueueSize 	= 128;
	mMaxConnections = 256;
	mRefusedMsg 	= strdup( "System busy, try again later." );
}

TcpServer :: ~TcpServer()
{
	if( NULL != mRefusedMsg ) free( mRefusedMsg );
	mRefusedMsg = NULL;
}


void TcpServer :: setMaxThreads( int maxThreads )
{
	mMaxThreads = maxThreads > 0 ? maxThreads : mMaxThreads;
}

void TcpServer :: setMaxConnections( int maxConnections )
{
	mMaxConnections = maxConnections > 0 ? maxConnections : mMaxConnections;
}

void TcpServer :: setReqQueueSize( int reqQueueSize, const char * refusedMsg )
{
	mReqQueueSize = reqQueueSize > 0 ? reqQueueSize : mReqQueueSize;

	if( NULL != mRefusedMsg ) free( mRefusedMsg );
	mRefusedMsg = strdup( refusedMsg );
}

int TcpServer :: registerEvent(const EventArg& evarg) {
	int ret = 0;

	ret = IOUtils::tcpListen( mBindIP, mPort, &mListenFD, 0 );
	GLOGW("create listenid:%d ret:%d\n", mListenFD, ret);

	memset( &mAcceptArg, 0, sizeof( AcceptArg_t ) );
	mAcceptArg.mEventArg 		= (EventArg*)&evarg;
	mAcceptArg.mReqQueueSize 	= mReqQueueSize;
	mAcceptArg.mMaxConnections 	= mMaxConnections;
	mAcceptArg.mRefusedMsg 		= mRefusedMsg;

	event_set( &mEvAccept, mListenFD, EV_READ|EV_PERSIST, EventCall::onAccept, &mAcceptArg );
	event_base_set( evarg.getEventBase(), &mEvAccept );
	event_add( &mEvAccept, NULL );

	return ret;
}

void TcpServer :: shutdown() {
	event_del( &mEvAccept);
	if(mListenFD>0) {
		close(mListenFD);
		mListenFD = 0;
		GLOGW("close listenid:%d\n", mListenFD);
	}
}

void TcpServer :: setSurface(void*surface) {
	if(mAcceptArg.mEventArg) {
		EventArg *arg = mAcceptArg.mEventArg;
		SessionManager * manager = arg->getSessionManager();
		manager->setSurface(surface);
	}
}


