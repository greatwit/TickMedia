#ifndef __actorstation_hpp__
#define __actorstation_hpp__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "EventCall.hpp"
#include "TaskBase.hpp"
#include "event.h"


class ActorStation {
	public:
		ActorStation(  );
		~ActorStation();

		const EventArg& getEventArg();

		int startup();
		int isRunning();
		void shutdown();
		void releaseSessions();

		void setTimeout( int timeout );

	private:
		static void * eventLoop( void * arg );
		int start();
		int run();
		static void sigHandler( int, short, void * arg );

		EventArg 	mEventArg;
		int mIsShutdown;
		int mIsRunning;
		int mTimeout;
};

#endif

