

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


#include "ActorStation.hpp"
#include "basedef.h"

	ActorStation :: ActorStation( )
		:mIsShutdown(0)
		,mIsRunning(0)
		,mTimeout(60) {
	}

	ActorStation :: ~ActorStation() {
		//mEventArg.Destroy();
		GLOGV("ActorStation Destroy.\n");
	}

	void ActorStation :: setTimeout( int timeout )
	{
		mTimeout = timeout > 0 ? timeout : mTimeout;
	}

	const EventArg&  ActorStation :: getEventArg() {
		return mEventArg;
	}

	int ActorStation :: startup() {
		if(isRunning()==0) {
			mIsShutdown = 0;
			mEventArg.setTimeout(mTimeout);
			mEventArg.Create();
			run();
			return 0;
		}
		return -1;
	}

	void ActorStation :: shutdown() {
		if(isRunning()==1) {
			mIsShutdown = 1;
			struct timeval tv;
			tv.tv_sec=0;
			tv.tv_usec=10;
			event_loopexit(&tv);

			GLOGV("ActorStation shutdown function.\n");
		}
	}

	int ActorStation :: isRunning() {
		return mIsRunning;
	}

	int ActorStation :: run() {
		int ret = -1;

		pthread_attr_t attr;
		pthread_attr_init( &attr );
		assert( pthread_attr_setstacksize( &attr, 1024 * 1024 ) == 0 );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

		pthread_t thread = 0;
		ret = pthread_create( &thread, &attr, reinterpret_cast<void*(*)(void*)>(eventLoop), this );
		pthread_attr_destroy( &attr );
		if( 0 == ret ) {
			GLOGE( "Thread #%ld has been created to listen.", thread );
		} else {
			mIsRunning = 0;
			GLOGE( "Unable to create a thread for TCP server, %s", strerror( errno ) ) ;
		}
		GLOGI("ActorStation run ret:%d\n", ret);
		return ret;
	}

	void * ActorStation :: eventLoop( void * arg ) {
		ActorStation * station = (ActorStation*)arg;

		station->mIsRunning = 1;

		station->start();

		station->mIsRunning = 0;

		return NULL;
	}

	int ActorStation :: start() {
		/* Don't die with SIGPIPE on remote read shutdown. That's dumb. */
		signal( SIGPIPE, SIG_IGN );

		int ret = 0;

		if( 0 == ret ) {
			// Clean close on SIGINT or SIGTERM.
			struct event evSigInt, evSigTerm;
			signal_set( &evSigInt, SIGINT,  sigHandler, this );
			event_base_set( mEventArg.getEventBase(), &evSigInt );
			signal_add( &evSigInt, NULL);

			signal_set( &evSigTerm, SIGTERM, sigHandler, this );
			event_base_set( mEventArg.getEventBase(), &evSigTerm );
			signal_add( &evSigTerm, NULL);


			/* Start the event loop. */
			while( 0 == mIsShutdown ) {
				event_base_loop( mEventArg.getEventBase(), EVLOOP_ONCE );
			}

			mEventArg.Destroy();
			GLOGV("ActorStation is shutdown.\n");

			signal_del( &evSigTerm );
			signal_del( &evSigInt );
		}
		return ret;
	}

	void ActorStation :: sigHandler( int, short, void * arg ) {
		ActorStation * station = (ActorStation*)arg;
		station->shutdown();
	}

