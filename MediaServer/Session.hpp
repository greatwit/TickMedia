
#ifndef __session_hpp__
#define __session_hpp__


#include <unistd.h>
#include "TaskBase.hpp"

//class SP_Handler;
//class SP_ArrayList;


struct event;

class Session {

public:
	Session( Sid_t sid );
	Session( Sid_t sid, short type);
	Session( Sid_t sid, short type, char*filepath);
	Session( Sid_t sid, short type, char*filepath, void*surface);
	Session( Sid_t sid, short type, char*remoteFile, char*saveFile);
	virtual ~Session();

	struct event * getReadEvent();	
	struct event * getWriteEvent();
	struct event * getTimeEvent();

	//void setHandler( SP_Handler * handler );
	//SP_Handler * getHandler();

	void setSurface(void *surface);

	void setArg( void * arg );
	void * getArg();

	Sid_t getSid();

	enum { eNormal, eWouldExit, eExit };
	void setStatus( int status );
	int getStatus();

	int setHeartBeat();

	int readBuffer();
	int writeBuffer();

	int getRunning();
	void setRunning( int running );

	int getReading();
	void setReading( int reading );

	int getWriting();
	void setWriting( int writing );

private:
	Session( Session & );
	Session & operator=( Session & );
	int recvEx(char*pData, int len);
	int recvPackData();


	Sid_t mSid;
	TaskBase *mTaskBase;

	struct event * mReadEvent;
	struct event * mWriteEvent;
	struct event * mTimeEvent;
	//SP_Handler * mHandler;
	void * mArg;


	char mStatus;
	char mRunning;
	char mWriting;
	char mReading;

	int  mHeadLenConst;//const
	int  mTotalDataLen;
	int  mHasRecvLen;
	bool mbRecvHead;

	char mReadBuff[1500];
};

typedef struct tagSessionEntry SessionEntry_t;

class SessionManager {

public:
	SessionManager();
	~SessionManager();

	void setSurface(void*surface);

	int getCount();
	void put( uint16_t key, Session * session, uint16_t * seq );
	Session * get( uint16_t key, uint16_t * seq );
	Session * remove( uint16_t key, uint16_t * seq = NULL );

private:
	int mCount;
	SessionEntry_t * mArray[ 64 ];
};

#endif

