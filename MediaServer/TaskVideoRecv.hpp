#ifndef __TaskVideoRecv_hpp__
#define __TaskVideoRecv_hpp__


#include <stdio.h>

#include "h264.h"
#include "TaskBase.hpp"

class BufferCache;

class TaskVideoRecv :public TaskBase {
	public:
		TaskVideoRecv( Session*sess, Sid_t &sid );
		TaskVideoRecv( Session*sess, Sid_t &sid, char*filepath );
		virtual ~TaskVideoRecv();
		virtual int StartTask();
		virtual int StopTask();
		virtual int readBuffer();
		virtual int writeBuffer();

	private:
		int tcpSendData();
		int sendEx(void*data, int len);
		int SendCmd(int dwCmd, int dwIndex, void* lpData, int nLength);

		int recvPackData();

		struct tagCmdBuffer 		mSendBuffer;
		struct tagFileProcBuffer 	mRecvBuffer;
		Session			*mSess;
		FILE			*mwFile;

		int 	mPackHeadLen;

		int  	mRecvDataLen;
		int  	mRecvHeadLen;
		int  	mTotalLen;
};


#endif
