#ifndef __TaskVideoRealSend_hpp__
#define __TaskVideoRealSend_hpp__


#include "h264.h"

#include "IVideoCallback.h"
#include "TaskBase.hpp"
#include "GQueue.h"



class TaskVideoRealSend : public TaskBase, public IVideoCallback{

	public:
		TaskVideoRealSend( Session*sess, Sid_t& sid, char*filename );
		virtual ~TaskVideoRealSend();

		virtual int setHeartCount();
		virtual int readBuffer();
		virtual int writeBuffer();

		void setSurface(void *surface);

	private:
		int tcpSendData();
		int sendEx(char*data,int len);
		int recvPackData();
		int pushSendCmd(int iVal, int index=0);

		int sendVariedCmd(int iVal);

		void VideoSource(VideoFrame *pBuf);

	private:
		struct tagRecvBuffer 		mRecvBuffer;
		struct tagRealSendBuffer 	mSendBuffer;
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
