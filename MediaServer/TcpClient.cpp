

#include "TcpClient.hpp"
#include "IOUtils.hpp"

#include "Session.hpp"

#include "event.h"
#include "basedef.h"
#include "h264.h"

TcpClient :: TcpClient( )
		:mSockId(0)
		,mSession(NULL)
{

}

TcpClient :: ~TcpClient()
{
	if(mSockId > 0) {
		close(mSockId);
		mSockId = 0;
	}

	if(mSession != NULL) {
		delete mSession;
		mSession = NULL;
	}
}

int TcpClient :: connect(const char* destIp, unsigned short destPort) {
	int ret = IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGW("connect ret:%d sockid:%d.\n",ret, mSockId);
	if(ret>=0) {
		mSid.mKey = mSockId;
		IOUtils::setNonblock( mSockId );
		mSession = new Session( mSid, VIDEO_RECV_MSG );
	}
	return ret;
}

int TcpClient :: connect(const char* destIp, unsigned short destPort, const char*filepath) {
	int ret = IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGW("connect ret:%d sockid:%d.\n",ret, mSockId);
	if(ret>=0) {
		mSid.mKey = mSockId;
		IOUtils::setNonblock( mSockId );
		mSession = new Session( mSid, VIDEO_RECV_MSG, (char*)filepath );
	}
	return ret;
}

int TcpClient :: connect(const char* destIp, unsigned short destPort, void *surface) {
	int ret = IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGW("connect ret:%d sockid:%d.\n",ret, mSockId);
	if(ret>=0) {
		mSid.mKey = mSockId;
		IOUtils::setNonblock( mSockId );
		mSession = new Session( mSid, VIDEO_RECV_MSG, surface );
	}
	return ret;
}

int TcpClient :: connect(const char* destIp, unsigned short destPort, const char*filepath, void *surface) {
	int ret = IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGW("connect ret:%d sockid:%d.\n",ret, mSockId);
	if(ret>=0) {
		mSid.mKey = mSockId;
		IOUtils::setNonblock( mSockId );
		mSession = new Session( mSid, VIDEO_RECV_MSG, (char*)filepath, surface );
	}
	return ret;
}

int TcpClient :: connect(const char* destIp, unsigned short destPort, const char*remoteFile, const char*saveFile) {
	int ret = IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGW("connect ret:%d sockid:%d.\n",ret, mSockId);
	if(ret>=0) {
		mSid.mKey = mSockId;
		IOUtils::setNonblock( mSockId );
		mSession = new Session( mSid, FILE_RECV_MSG, (char*)remoteFile, (char*)saveFile );
	}
	return ret;
}

int TcpClient :: disConnect() {
	if(mSockId > 0) {
		EventArg * eventArg = (EventArg*)mSession->getArg();
		eventArg->getSessionManager()->remove(mSockId);
		event_del(mSession->getReadEvent());
		event_del(mSession->getReadEvent());

		close(mSockId);
		mSockId = 0;
		GLOGW("disConnect sockid:%d.\n", mSockId);
	}
	return 0;
}

int TcpClient :: registerEvent(const EventArg& evarg) {

	if(mSession!=NULL) {

		evarg.getSessionManager()->get( mSid.mKey, &mSid.mSeq );
		evarg.getSessionManager()->put( mSid.mKey, mSession, &mSid.mSeq );
		mSession->setArg( (void*)&evarg );

		event_set( mSession->getReadEvent(), mSockId, EV_READ|EV_PERSIST, EventCall::onRead, mSession );
		EventCall::addEvent( mSession, EV_READ, mSockId );

		//event_set( mSession->getWriteEvent(), mSockId, EV_WRITE, EventCall::onWrite, mSession );
		//EventCall::addEvent( mSession, EV_WRITE, mSockId );
		GLOGW("tcpclient registerEvent mSession done.\n");
	}
	else
		GLOGE("tcpclient registerEvent mSession is NULL.\n");

	return 0;
}


