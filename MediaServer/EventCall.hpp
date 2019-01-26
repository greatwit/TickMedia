

#ifndef __eventcall_hpp__
#define __eventcall_hpp__
/*
class SP_HandlerFactory;
class SP_BlockingQueue;
class SP_Message;
*/

#include "TaskBase.hpp"

class SP_IOChannelFactory;


class SessionManager;
class Session;

struct event_base;

class EventArg {
public:
	EventArg();
	EventArg( int timeout );
	~EventArg();
	int Create();
	int Destroy();

	struct event_base * getEventBase() const;
/*
	void * getResponseQueue() const;
	SP_BlockingQueue * getInputResultQueue() const;
	SP_BlockingQueue * getOutputResultQueue() const;
*/
	SessionManager * getSessionManager() const;

	void setTimeout( int timeout );
	int getTimeout() const;

private:
	struct event_base * mEventBase;
/*
	void * mResponseQueue;
	SP_BlockingQueue * mInputResultQueue;
	SP_BlockingQueue * mOutputResultQueue;
*/
	SessionManager * mSessionManager;

	int mTimeout;
};

typedef struct tag_AcceptArg {
	EventArg * mEventArg;

	//SP_HandlerFactory * mHandlerFactory;
	//SP_IOChannelFactory * mIOChannelFactory;
	int mReqQueueSize;
	int mMaxConnections;
	char * mRefusedMsg;
} AcceptArg_t;


class EventCall {

	public:
		static void onAccept( int fd, short events, void * arg );
		static void onRead( int fd, short events, void * arg );
		static void onWrite( int fd, short events, void * arg );
		static void onTimer( int fd, short events, void * arg );
		static void addEvent( Session * session, short events, int fd );

		static void onResponse( void * queueData, void * arg );

	private:
		EventCall();
		~EventCall();
};

class EventHelper {

	public:
		static void doStart( Session * session );
		static void start( void * arg );

		static void doWork( Session * session );
		static void worker( void * arg );

		static void doError( Session * session );
		static void error( void * arg );

		static void doTimeout( Session * session );
		static void timeout( void * arg );

		//static void doCompletion( EventArg * eventArg, SP_Message * msg );

		static int isSystemSid( Sid_t * sid );

	private:
		EventHelper();
		~EventHelper();
};

#endif

