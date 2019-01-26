#ifndef __TaskVideoSend_hpp__
#define __TaskVideoSend_hpp__

#include <stdio.h>
#include <unistd.h>

#include "h264.h"
#include "TaskBase.hpp"

class BufferCache;

class TaskVideoSend :public TaskBase {
	public:
		TaskVideoSend( Session*sess, Sid_t& sid, char*filename );
		virtual ~TaskVideoSend();
		virtual int StartTask();
		virtual int StopTask();
		virtual int readBuffer();
		virtual int writeBuffer();

		void packetHead(int fid, short pid, int len, unsigned char type, LPPACK_HEAD lpPack);
		int  tpcSendMsg(unsigned char msg);

	private:
		int tcpSendData();
		int sendEx(char*data,int len);
		int recvPackData();
		int pushSendCmd(int iVal, int index=0);

	private:
		struct tagRecvBuffer 		mRecvBuffer;
		struct tagNALSendBuffer 	mSendBuffer;

		FILE						*mwFile;
		Session						*mSess;
		int							mSeqid;
		bool						mbSendingData;

		int  mPackHeadLen;
		int  mRecvDataLen;
		int  mRecvHeadLen;
		int  mTotalLen;

};


#endif
