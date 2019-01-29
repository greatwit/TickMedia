

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "EventCall.hpp"
#include "Session.hpp"
#include "BufferCache.hpp"

#include "IOUtils.hpp"

#include "config.h"   // from libevent, for event.h
//#include "event_msgqueue.h"
#include "event.h"
#include "basedef.h"

#define TIMEOUT_US 1000000

EventArg :: EventArg()
		:mTimeout(0)
		,mEventBase(NULL)
		,mSessionManager(NULL) {
}

EventArg :: EventArg( int timeout )
			:mTimeout(timeout)
			,mEventBase(NULL)
			,mSessionManager(NULL) {
}

EventArg :: ~EventArg()
{
	//delete mInputResultQueue;
	//delete mOutputResultQueue;

	delete mSessionManager;
	mSessionManager = NULL;

	//msgqueue_destroy( (struct event_msgqueue*)mResponseQueue );
	//event_base_free( mEventBase );
}

int EventArg ::Create() {
	if(mEventBase==NULL)
		mEventBase = (struct event_base*)event_init();

	if(mSessionManager==NULL)
		mSessionManager = new SessionManager();

	return 0;
}

int EventArg ::Destroy() {
	event_destroy();
	if(mSessionManager!=NULL) {
		delete mSessionManager;
		mSessionManager = NULL;
	}
	return 0;
}

struct event_base * EventArg :: getEventBase() const {
	return mEventBase;
}

/*
void * SP_EventArg :: getResponseQueue() const
{
	return mResponseQueue;
}

SP_BlockingQueue * SP_EventArg :: getInputResultQueue() const
{
	return mInputResultQueue;
}

SP_BlockingQueue * SP_EventArg :: getOutputResultQueue() const
{
	return mOutputResultQueue;
}
*/

SessionManager * EventArg :: getSessionManager() const
{
	return mSessionManager;
}

void EventArg :: setTimeout( int timeout )
{
	mTimeout = timeout;
}

int EventArg :: getTimeout() const
{
	return mTimeout;
}

//-------------------------------Event callback------------------------------------

void EventCall :: onAccept( int fd, short events, void * arg )
{
	int clientFD;
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof( clientAddr );
	GLOGW( "accept id:%d\n",fd);
	AcceptArg_t * acceptArg = (AcceptArg_t*)arg;
	EventArg * eventArg = acceptArg->mEventArg;

	clientFD = accept( fd, (struct sockaddr *)&clientAddr, &clientLen );
	if( -1 == clientFD ) {
		syslog( LOG_WARNING, "accept failed" );
		return;
	}

	if( IOUtils::setNonblock( clientFD ) < 0 ) {
		GLOGE("failed to set client socket non-blocking\n" );
	}

	Sid_t sid;
	sid.mKey = clientFD;
	eventArg->getSessionManager()->get( sid.mKey, &sid.mSeq );

	char clientIP[ 32 ] = { 0 };
	IOUtils::inetNtoa( &( clientAddr.sin_addr ), clientIP, sizeof( clientIP ) );
	GLOGW( "clientIP: %s clientFD:%d\n",clientIP, clientFD);
	//session->getRequest()->setClientIP( clientIP );

	Session * session = new Session( sid );
	if( NULL != session ) {
		eventArg->getSessionManager()->put( sid.mKey, session, &sid.mSeq );
		session->setArg( eventArg );

		event_set( session->getReadEvent(),  clientFD, EV_READ,  onRead, session );
		event_set( session->getWriteEvent(), clientFD, EV_WRITE, onWrite, session );
		addEvent(  session, EV_READ, clientFD );

		//event_set( session->getTimeEvent(), clientFD, EV_TIMEOUT, onTimer, session );
		evtimer_set(session->getTimeEvent(), onTimer, session);//useful
		addEvent(  session, EV_TIMEOUT, clientFD );

		struct timeval timeout;
		memset( &timeout, 0, sizeof( timeout ) );
		timeout.tv_sec 	= 0;
		timeout.tv_usec = TIMEOUT_US;
		evtimer_add( session->getTimeEvent(), &timeout );
		//addEvent( session, EV_WRITE, clientFD );

		if( eventArg->getSessionManager()->getCount() > acceptArg->mMaxConnections
			/*|| eventArg->getInputResultQueue()->getLength() >= acceptArg->mReqQueueSize*/ ) {

			//syslog( LOG_WARNING, "System busy, session.count %d [%d], queue.length %d [%d]",
			//	eventArg->getSessionManager()->getCount(), acceptArg->mMaxConnections,
			//	eventArg->getInputResultQueue()->getLength(), acceptArg->mReqQueueSize );
/*
			SP_Message * msg = new SP_Message();
			msg->getMsg()->append( acceptArg->mRefusedMsg );
			msg->getMsg()->append( "\r\n" );
			session->getOutList()->append( msg );
*/
			session->setStatus( Session::eExit );

			addEvent( session, EV_WRITE, clientFD );
		} else {
			EventHelper::doStart( session );
		}
	} else {
		close( clientFD );
		GLOGE("Out of memory, cannot allocate session object!\n" );
	}

}

void EventCall :: onRead( int fd, short events, void * arg )
{
	Session * session = (Session*)arg;
	session->setReading( 0 );
	Sid_t sid = session->getSid();

	if( EV_READ & events ) {

		int ret = session->readBuffer();
		if(ret==0)
		{
				EventArg * eventArg = (EventArg*)session->getArg();
				eventArg->getSessionManager()->remove( fd );
				event_del( session->getReadEvent() );
				event_del( session->getWriteEvent() );
				event_del( session->getTimeEvent() );

				delete session;
				session = NULL;

				close( fd );

				GLOGE("read zero:%d\n",ret);

				return;
		}
	}//if

	addEvent( session, EV_READ, fd );
}

void EventCall :: onWrite( int fd, short events, void * arg )
{
	int ret = 0;
	Session * session = (Session*)arg;

	EventArg * eventArg = (EventArg*)session->getArg();

	session->setWriting( 0 );

	Sid_t sid = session->getSid();
	//GLOGW("onWrite fd:%d sid:%d",fd,session->getSid().mKey);

	if( EV_WRITE & events ) {
		ret = session->writeBuffer();
	}

	//>0 need listen,<=0 not do that
	if(ret==OWN_SOCK_EXIT) {
//		EventArg * eventArg = (EventArg*)session->getArg();
//		eventArg->getSessionManager()->remove( fd );
//		event_del( session->getReadEvent() );
//		event_del( session->getWriteEvent() );
//		event_del( session->getTimeEvent() );
//
//		delete session;
//		session = NULL;
//		close( fd );
	}
	else if(ret != 0)
		addEvent( session, EV_WRITE, fd );


/*
	if( EV_WRITE & events ) {
		int ret = 0;

		if( session->getOutList()->getCount() > 0 ) {
			int len = session->getIOChannel()->transmit( session );

			if( len > 0 ) {
				if( session->getOutList()->getCount() > 0 ) {
					// left for next write event
					addEvent( session, EV_WRITE, -1 );
				}
			} else {
				if( EINTR != errno && EAGAIN != errno ) {
					ret = -1;
					if( 0 == session->getRunning() ) {
						syslog( LOG_NOTICE, "session(%d.%d) write error", sid.mKey, sid.mSeq );
						EventHelper::doError( session );
					} else {
						addEvent( session, EV_WRITE, -1 );
						syslog( LOG_NOTICE, "session(%d.%d) busy, process session error later, errno [%d]",
								sid.mKey, sid.mSeq, errno );
					}
				}
			}
		}

		if( 0 == ret && session->getOutList()->getCount() <= 0 ) {
			if( Session::eExit == session->getStatus() ) {
				ret = -1;
				if( 0 == session->getRunning() ) {
					syslog( LOG_NOTICE, "session(%d.%d) close, exit", sid.mKey, sid.mSeq );

					eventArg->getSessionManager()->remove( fd );
					event_del( session->getReadEvent() );
					handler->close();
					close( fd );
					delete session;
				} else {
					addEvent( session, EV_WRITE, -1 );
					syslog( LOG_NOTICE, "session(%d.%d) busy, terminate session later",
							sid.mKey, sid.mSeq );
				}
			}
		}

		if( 0 == ret ) {
			if( 0 == session->getRunning() ) {
				SP_MsgDecoder * decoder = session->getRequest()->getMsgDecoder();
				if( SP_MsgDecoder::eOK == decoder->decode( session->getInBuffer() ) ) {
					EventHelper::doWork( session );
				}
			} else {
				// If this session is running, then onResponse will add write event for this session.
				// So no need to add write event here.
			}
		}
	} else {
		if( 0 == session->getRunning() ) {
			EventHelper::doTimeout( session );
		} else {
			addEvent( session, EV_WRITE, -1 );
			syslog( LOG_NOTICE, "session(%d.%d) busy, process session timeout later",
					sid.mKey, sid.mSeq );
		}
	}
*/
}

void EventCall :: onTimer( int fd, short events, void * arg )
{
	Session * session = (Session*)arg;
	int count = session->setHeartBeat();

	struct timeval timeout;
	memset( &timeout, 0, sizeof( timeout ) );
	timeout.tv_sec 	= 0;
	timeout.tv_usec = TIMEOUT_US;
	evtimer_add(session->getTimeEvent(), &timeout);
}

void EventCall :: onResponse( void * queueData, void * arg )
{
	//SP_Response * response = (SP_Response*)queueData;
	//EventArg * eventArg = (EventArg*)arg;
	//SessionManager * manager = eventArg->getSessionManager();

	//Sid_t fromSid = response->getFromSid();
	//u_int16_t seq = 0;
}

void EventCall :: addEvent( Session * session, short events, int fd )
{
	EventArg * eventArg = (EventArg*)session->getArg();

	if( ( events & EV_WRITE ) && (0 == session->getWriting()) ) {
		session->setWriting( 1 );

		struct event*pEvent=session->getWriteEvent();

		if( fd < 0 ) fd = EVENT_FD( pEvent );

		event_set( pEvent, fd, events, onWrite, session );
		event_base_set( eventArg->getEventBase(), pEvent );


		struct timeval timeout;
		memset( &timeout, 0, sizeof( timeout ) );
		timeout.tv_sec = eventArg->getTimeout();
		event_add( pEvent, &timeout );
	}

	if( (events & EV_READ) && (0 == session->getReading()) ) {
		session->setReading( 1 );

		if( fd < 0 ) fd = EVENT_FD( session->getWriteEvent() );

		struct event*pEvent=session->getReadEvent();
		event_set( pEvent, fd, events, onRead, session );
		event_base_set( eventArg->getEventBase(), pEvent );

		struct timeval timeout;
		memset( &timeout, 0, sizeof( timeout ) );
		timeout.tv_sec = eventArg->getTimeout();
		event_add( pEvent, &timeout );
	}
}



//----------------------------------EventHelper---------------------------------

int EventHelper :: isSystemSid( Sid_t * sid )
{
	return sid->mKey == Sid_t::eTimerKey && sid->mSeq == Sid_t::eTimerSeq;
}

void EventHelper :: doWork( Session * session )
{
/*
	if( Session::eNormal == session->getStatus() ) {
		session->setRunning( 1 );
		EventArg * eventArg = (EventArg*)session->getArg();
		eventArg->getInputResultQueue()->push( new SP_SimpleTask( worker, session, 1 ) );
	} else {
		Sid_t sid = session->getSid();

		char buffer[ 16 ] = { 0 };
		session->getInBuffer()->take( buffer, sizeof( buffer ) );
		syslog( LOG_WARNING, "session(%d.%d) status is %d, ignore [%s...] (%dB)",
			sid.mKey, sid.mSeq, session->getStatus(), buffer, session->getInBuffer()->getSize() );
		session->getInBuffer()->reset();
	}
*/
}

void EventHelper :: worker( void * arg )
{
/*
	Session * session = (Session*)arg;
	SP_Handler * handler = session->getHandler();
	EventArg * eventArg = (EventArg *)session->getArg();

	SP_Response * response = new SP_Response( session->getSid() );
	if( 0 != handler->handle( session->getRequest(), response ) ) {
		session->setStatus( Session::eWouldExit );
	}

	session->setRunning( 0 );

	msgqueue_push( (struct event_msgqueue*)eventArg->getResponseQueue(), response );
*/
}

void EventHelper :: doError( Session * session )
{
	EventArg * eventArg = (EventArg *)session->getArg();

	event_del( session->getWriteEvent() );
	event_del( session->getReadEvent() );

	Sid_t sid = session->getSid();

//	SP_ArrayList * outList = session->getOutList();
//	for( ; outList->getCount() > 0; ) {
//		SP_Message * msg = ( SP_Message * ) outList->takeItem( SP_ArrayList::LAST_INDEX );
//
//		int index = msg->getToList()->find( sid );
//		if( index >= 0 ) msg->getToList()->take( index );
//		msg->getFailure()->add( sid );
//
//		if( msg->getToList()->getCount() <= 0 ) {
//			doCompletion( eventArg, msg );
//		}
//	}

	// remove session from SessionManager, onResponse will ignore this session
	eventArg->getSessionManager()->remove( sid.mKey );

	//eventArg->getInputResultQueue()->push( new SP_SimpleTask( error, session, 1 ) );

}

void EventHelper :: error( void * arg )
{
	Session * session = ( Session * )arg;
	//EventArg * eventArg = (EventArg*)session->getArg();

	Sid_t sid = session->getSid();

//	SP_Response * response = new SP_Response( sid );
//	session->getHandler()->error( response );
//
//	msgqueue_push( (struct event_msgqueue*)eventArg->getResponseQueue(), response );
//
//	// onResponse will ignore this session, so it's safe to destroy session here
//	session->getHandler()->close();
	close( EVENT_FD( session->getWriteEvent() ) );
	delete session;
	syslog( LOG_WARNING, "session(%d.%d) error, exit\n", sid.mKey, sid.mSeq );

}

void EventHelper :: doTimeout( Session * session )
{
/*
	EventArg * eventArg = (EventArg*)session->getArg();

	event_del( session->getWriteEvent() );
	event_del( session->getReadEvent() );

	Sid_t sid = session->getSid();

	SP_ArrayList * outList = session->getOutList();
	for( ; outList->getCount() > 0; ) {
		SP_Message * msg = ( SP_Message * ) outList->takeItem( SP_ArrayList::LAST_INDEX );

		int index = msg->getToList()->find( sid );
		if( index >= 0 ) msg->getToList()->take( index );
		msg->getFailure()->add( sid );

		if( msg->getToList()->getCount() <= 0 ) {
			doCompletion( eventArg, msg );
		}
	}

	// remove session from SessionManager, onResponse will ignore this session
	eventArg->getSessionManager()->remove( sid.mKey );

	eventArg->getInputResultQueue()->push( new SP_SimpleTask( timeout, session, 1 ) );
*/
}

void EventHelper :: timeout( void * arg )
{
/*
	Session * session = ( Session * )arg;
	EventArg * eventArg = (EventArg*)session->getArg();

	Sid_t sid = session->getSid();

	SP_Response * response = new SP_Response( sid );
	session->getHandler()->timeout( response );
	msgqueue_push( (struct event_msgqueue*)eventArg->getResponseQueue(), response );

	// onResponse will ignore this session, so it's safe to destroy session here
	session->getHandler()->close();
	close( EVENT_FD( session->getWriteEvent() ) );
	delete session;
	syslog( LOG_WARNING, "session(%d.%d) timeout, exit", sid.mKey, sid.mSeq );
*/
}


void EventHelper :: doStart( Session * session )
{
	//session->setRunning( 1 );
	//EventArg * eventArg = (EventArg*)session->getArg();
	printf("doStart.\n");
	//eventArg->getInputResultQueue()->push( new SP_SimpleTask( start, session, 1 ) );
}

void EventHelper :: start( void * arg )
{
/*
	Session * session = ( Session * )arg;
	SP_EventArg * eventArg = (SP_EventArg*)session->getArg();

	SP_IOChannel * ioChannel = session->getIOChannel();

	int initRet = ioChannel->init( EVENT_FD( session->getWriteEvent() ) );

	// always call SP_Handler::start
	SP_Response * response = new SP_Response( session->getSid() );
	int startRet = session->getHandler()->start( session->getRequest(), response );

	int status = SP_Session::eWouldExit;

	if( 0 == initRet ) {
		if( 0 == startRet ) status = SP_Session::eNormal;
	} else {
		delete response;
		// make an empty response
		response = new SP_Response( session->getSid() );
	}

	session->setStatus( status );
	session->setRunning( 0 );
	msgqueue_push( (struct event_msgqueue*)eventArg->getResponseQueue(), response );
*/
}

/*
void SP_EventHelper :: doCompletion( SP_EventArg * eventArg, SP_Message * msg )
{
	eventArg->getOutputResultQueue()->push( msg );
}
*/
