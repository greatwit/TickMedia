


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TaskVideoRecv.hpp"
#include "TaskVideoSend.hpp"
#include "TaskFileSend.hpp"
#include "TaskFileRecv.hpp"
#include "Session.hpp"


#include "config.h"
#include "event.h"

#include "h264.h"
#include "basedef.h"
#include "protocol.h"
#include "net_protocol.h"


//---------------------------sessionManager----------------------------------------

typedef struct tagSessionEntry {
	uint16_t mSeq;
	Session  *mSession;
} SessionEntry;

SessionManager :: SessionManager()
{
	mCount = 0;
	memset( mArray, 0, sizeof( mArray ) );
}

SessionManager :: ~SessionManager()
{
	for( int i = 0; i < (int)( sizeof( mArray ) / sizeof( mArray[0] ) ); i++ ) {
		SessionEntry_t * list = mArray[ i ];
		if( NULL != list ) {
			SessionEntry_t * iter = list;
			for( int i = 0; i < 1024; i++, iter++ ) {
				if( NULL != iter->mSession ) {
					delete iter->mSession;
					iter->mSession = NULL;
				}
			}
			free( list );
		}
	}

	memset( mArray, 0, sizeof( mArray ) );
}

int SessionManager :: getCount()
{
	return mCount;
}

void SessionManager :: put( uint16_t key, Session * session, uint16_t * seq )
{
	int row = key / 1024, col = key % 1024;

	if( NULL == mArray[ row ] ) {
		mArray[ row ] = ( SessionEntry_t * )calloc(
			1024, sizeof( SessionEntry_t ) );
	}

	SessionEntry_t * list = mArray[ row ];
	list[ col ].mSession = session;
	*seq = list[ col ].mSeq;

	mCount++;
}

Session * SessionManager :: get( uint16_t key, uint16_t * seq )
{
	int row = key / 1024, col = key % 1024;

	Session * ret = NULL;

	SessionEntry_t * list = mArray[ row ];
	if( NULL != list ) {
		ret = list[ col ].mSession;
		* seq = list[ col ].mSeq;
	} else {
		* seq = 0;
	}

	return ret;
}

Session * SessionManager :: remove( uint16_t key, uint16_t * seq )
{
	int row = key / 1024, col = key % 1024;

	Session * ret = NULL;

	SessionEntry_t * list = mArray[ row ];
	if( NULL != list ) {
		ret = list[ col ].mSession;
		if( NULL != seq ) * seq = list[ col ].mSeq;

		list[ col ].mSession = NULL;
		list[ col ].mSeq++;

		mCount--;
	}

	return ret;
}



//----------------------------------session---------------------------------

Session :: Session( Sid_t sid )
			:mHeadLenConst(sizeof(NET_HEAD))
			,mSid(sid)
			,mTaskBase(NULL)
			,mArg(NULL)
			,mTotalDataLen(0)
			,mHasRecvLen(0)
			,mbRecvHead(true)
{
	mReadEvent  = (struct event*)malloc( sizeof( struct event ) );
	mWriteEvent = (struct event*)malloc( sizeof( struct event ) );
	mTimeEvent	= (struct event*)malloc( sizeof( struct event ) );

	mStatus  	= eNormal;
	mRunning 	= 0;
	mWriting 	= 0;
	mReading 	= 0;
}

Session :: Session( Sid_t sid, short type)
			:mHeadLenConst(sizeof(NET_HEAD))
			,mSid(sid)
			,mTaskBase(NULL)
			,mArg(NULL)
			,mTotalDataLen(0)
			,mHasRecvLen(0)
			,mbRecvHead(true)
{
	mReadEvent  = (struct event*)malloc( sizeof( struct event ) );
	mWriteEvent = (struct event*)malloc( sizeof( struct event ) );
	mTimeEvent	= (struct event*)malloc( sizeof( struct event ) );

	mStatus  	= eNormal;
	mRunning 	= 0;
	mWriting 	= 0;
	mReading 	= 0;
	switch(type) {
		case VIDEO_RECV_MSG:
			if(!mTaskBase)
				mTaskBase = new TaskVideoRecv( this, mSid );
			break;

		case FILE_RECV_MSG:
				mTaskBase = new TaskFileRecv( this, mSid );
			break;
	}
}

Session :: Session( Sid_t sid, short type, char*filepath)
			:mHeadLenConst(sizeof(NET_HEAD))
			,mSid(sid)
			,mTaskBase(NULL)
			,mArg(NULL)
			,mTotalDataLen(0)
			,mHasRecvLen(0)
			,mbRecvHead(true)
{
	mReadEvent  = (struct event*)malloc( sizeof( struct event ) );
	mWriteEvent = (struct event*)malloc( sizeof( struct event ) );
	mTimeEvent	= (struct event*)malloc( sizeof( struct event ) );

	mStatus  	= eNormal;
	mRunning 	= 0;
	mWriting 	= 0;
	mReading 	= 0;
	switch(type) {
		case VIDEO_RECV_MSG:
			if(!mTaskBase)
				mTaskBase = new TaskVideoRecv( this, mSid, filepath );
			break;

		case FILE_RECV_MSG:
				mTaskBase = new TaskFileRecv( this, mSid, filepath );
			break;
	}
}

Session :: ~Session()
{
	free( mReadEvent );
	mReadEvent = NULL;

	free( mWriteEvent );
	mWriteEvent = NULL;

	free( mTimeEvent );
	mTimeEvent = NULL;

	if(mTaskBase!=NULL) {
		delete mTaskBase;
		mTaskBase = NULL;
	}
}

int Session :: setHeartBeat() {
	return mTaskBase!=NULL?mTaskBase->setHeartCount():0;
}

int Session :: recvEx(char*pData, int len) {
	int recvCount = 0, recvRet = 0;
	do{
		recvRet = recv(mSid.mKey, pData+recvCount, len-recvCount, 0);
		if(recvRet<=0)
			return recvRet;

		recvCount += recvRet;
	}while(recvCount<len);
	return recvCount;
}

int Session :: readBuffer() {
	int ret = 0;

	if(mTaskBase!=NULL) {
		ret = mTaskBase->readBuffer();
	}
	else
	{
		if(mbRecvHead) {
			ret = recv(mSid.mKey, mReadBuff+mHasRecvLen, mHeadLenConst-mHasRecvLen, 0);
			if(ret>0) {
				mHasRecvLen  += ret;
				if(mHasRecvLen==mHeadLenConst) {

					LPNET_HEAD head = (LPNET_HEAD)mReadBuff;
					mTotalDataLen   = head->dwLength;
					mHasRecvLen = 0;
					mbRecvHead  = false;

					GLOGE("Session flag:%08x frameLen:%d ret:%d\n", head->dwFlag, mTotalDataLen, ret);
					//GLOGE("Session flag:%08x ret:%d data:%s", cmdbuf->dwFlag, ret, cmdbuf->lpData);
					ret = recvPackData();
				}
			}
		}
		else
			ret = recvPackData();

	}//mTaskBase==NULL

	return ret;
}

int Session :: writeBuffer() {
	int ret = 0;
	if(mTaskBase!=NULL) {
		ret = mTaskBase->writeBuffer();
	}
	return ret;
}

int Session ::recvPackData() {
	int ret = recv(mSid.mKey, mReadBuff+mHeadLenConst+mHasRecvLen, mTotalDataLen-mHasRecvLen, 0);
	if(ret>0) {
		mHasRecvLen += ret;
		if(mHasRecvLen==mTotalDataLen) {

//			if(ret>0){
//				for(int i=0;i<mTotalDataLen;i++)
//					printf("%c",mReadBuff[i]);
//				printf("\n\n");
//			}

		    int lValueLen,dataType=-1;
		    char acValue[256] = {0};//new char[256];
		    memset(acValue,0, 256);
			LPNET_CMD pCmdbuf = (LPNET_CMD)mReadBuff;
		    PROTO_GetValueByName(mReadBuff, (char*)"play path", acValue, &lValueLen);
		    if(lValueLen>0)
		    	dataType 	= 0;
		    else {
		    	PROTO_GetValueByName(mReadBuff, (char*)"get path", acValue, &lValueLen);
		    	if(lValueLen>0)
		    		dataType = 1;
		    }
		    GLOGE("filename:%s cmd:%d dataType:%d\n", acValue, pCmdbuf->dwCmd, dataType);

		    if(access(acValue, F_OK)!=0) {
		    	GLOGE("filename %s is no exist.\n",acValue);
		    	return 0;
		    }

			switch(dataType) {
				case 0:
					mTaskBase = new TaskVideoSend( this, mSid, acValue );
					break;

				case 1:
					mTaskBase = new TaskFileSend( this, mSid, acValue );
					break;

				case FILE_RECV_MSG:
					break;
			}
			mTotalDataLen = 0;
			mHasRecvLen   = 0;
			mbRecvHead    = true;

			//SAFE_DELETE(acValue);
			//GLOGE("Session flag:%08x frameLen:%d ret:%d", pCmdbuf->dwFlag, pCmdbuf->dwLength, ret);
			//GLOGE("Session flag:%08x ret:%d data:%s", pCmdbuf->dwFlag, ret, pCmdbuf->lpData);
		}
	}
	return ret;
}

struct event * Session :: getReadEvent()
{
	return mReadEvent;
}

struct event * Session :: getWriteEvent()
{
	return mWriteEvent;
}

struct event * Session :: getTimeEvent()
{
	return mTimeEvent;
}

void Session :: setArg( void * arg )
{
	mArg = arg;
}

void * Session :: getArg()
{
	return mArg;
}

Sid_t Session :: getSid()
{
	return mSid;
}

void Session :: setStatus( int status )
{
	mStatus = status;
}

int Session :: getStatus()
{
	return mStatus;
}

int Session :: getRunning()
{
	return mRunning;
}

void Session :: setRunning( int running )
{
	mRunning = running;
}

int Session :: getWriting()
{
	return mWriting;
}

void Session :: setWriting( int writing )
{
	mWriting = writing;
}

int Session :: getReading()
{
	return mReading;
}

void Session :: setReading( int reading )
{
	mReading = reading;
}



