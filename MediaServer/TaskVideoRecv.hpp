#ifndef __TaskVideoRecv_hpp__
#define __TaskVideoRecv_hpp__


#include <stdio.h>

#include "h264.h"
#include "TaskBase.hpp"
#include "VideoBase.hpp"

class BufferCache;

class TaskVideoRecv :public TaskBase {
	public:
		TaskVideoRecv( Session*sess, Sid_t &sid );
		TaskVideoRecv( Session*sess, Sid_t &sid, char*filepath );
		virtual ~TaskVideoRecv();
		virtual int readBuffer();
		virtual int writeBuffer();

		void		setBase(VideoBase*base);

	private:
		int tcpSendData();
		int sendEx(void*data, int len);
		int SendCmd(int dwCmd, int dwIndex, void* lpData, int nLength);

		int recvPackData();

		struct tagCmdBuffer 		mSendBuffer;
		struct tagFileProcBuffer 	mRecvBuffer;
		Session			*mSess;
		FILE			*mwFile;
		VideoBase 		*mvBase;

		int 	mPackHeadLen;

		int  	mRecvDataLen;
		int  	mRecvHeadLen;
		int  	mTotalLen;
};


#endif
