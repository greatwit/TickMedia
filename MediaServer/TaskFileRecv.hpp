#ifndef __TaskFileRecv_hpp__
#define __TaskFileRecv_hpp__


#include <stdio.h>

#include "h264.h"
#include "TaskBase.hpp"

class BufferCache;

class TaskFileRecv :public TaskBase {
	public:
		TaskFileRecv( Session*sess, Sid_t &sid );
		TaskFileRecv( Session*sess, Sid_t &sid, char*remoteFile );
		TaskFileRecv( Session*sess, Sid_t &sid, char*remoteFile, char*saveFile );
		virtual ~TaskFileRecv();
		virtual int readBuffer();
		virtual int writeBuffer();

	private:
		int sendEx(void*data, int len);
		int SendCmd(int dwCmd, int dwIndex, void* lpData, int nLength);

		int recvPackData();

		struct tagCmdBuffer 		mCmdBuffer;
		struct tagFileProcBuffer 	mRecvBuffer;
		Session			*mSess;
		FILE			*mwFile;

		int 	mPackHeadLen;

		int  	mRecvDataLen;
		int  	mRecvHeadLen;
		int  	mTotalLen;
};


#endif
