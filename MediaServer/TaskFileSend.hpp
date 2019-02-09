#ifndef __TaskFileSend_hpp__
#define __TaskFileSend_hpp__


#include "h264.h"

#include "TaskBase.hpp"
#include "GQueue.h"


class BufferCache;

class TaskFileSend :public TaskBase {

	public:
		TaskFileSend( Session*sess, Sid_t& sid, char*filename );
		virtual ~TaskFileSend();

		virtual int setHeartCount();
		virtual int readBuffer();
		virtual int writeBuffer();

	private:
		int tcpSendData();
		int sendEx(char*data,int len);
		int recvPackData();
		int pushSendCmd(int iVal, int index=0);

		int sendVariedCmd(int iVal);

	private:
		struct tagRecvBuffer 		mRecvBuffer;
		struct tagFileSendBuffer 	mSendBuffer;
		BufferCache 		 		*mInBuffer;
		FILE						*mpFile;
		Session						*mSess;
		GQueue<int>					mMsgQueue;

		int     		mFrameCount;
		unsigned int	mFileLen;

		int 			mPackHeadLen;

		int  			mRecvDataLen;
		int  			mRecvHeadLen;
		unsigned int    mHasReadLen;
		bool			mbSendingData;
};


#endif
