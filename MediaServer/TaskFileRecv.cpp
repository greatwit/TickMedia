
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TaskFileRecv.hpp"
#include "BufferCache.hpp"
#include "EventCall.hpp"

#include "event.h"
#include "h264.h"
#include "basedef.h"

#ifdef 	__ANDROID__
#define	FILE_PATH	"/sdcard/w.h264"
#else
#define	FILE_PATH	"recv.mp4"
#endif

const int	 BUFFER_LEN  = 1024*1024+1500;

	TaskFileRecv::TaskFileRecv( Session*sess, Sid_t &sid )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
	{
		mCmdBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(BUFFER_LEN);

		mwFile = fopen(FILE_PATH, "w");

		char lpData[2048];
		int nLength = sprintf(lpData, "<get path=\"%s\"/>", "/h264/tmp.mp4");


		if(SendCmd(MODULE_MSG_LOGIN, 0, lpData, nLength)<0)
			GLOGE("send CMD err!");

	}


	TaskFileRecv::TaskFileRecv( Session*sess, Sid_t &sid, char*remoteFile )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
	{
		mCmdBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(BUFFER_LEN);

		mwFile = fopen(FILE_PATH, "w");

		char lpData[2048];
		int nLength = sprintf(lpData, "<get path=\"%s\"/>", remoteFile);


		if(SendCmd(MODULE_MSG_LOGIN, 0, lpData, nLength)<0)
			GLOGE("send CMD err!");

	}

	TaskFileRecv::TaskFileRecv( Session*sess, Sid_t &sid, char*remoteFile, char*saveFile )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
	{
		mCmdBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(BUFFER_LEN);

		mwFile = fopen(saveFile, "w");

		char lpData[2048];
		int nLength = sprintf(lpData, "<get path=\"%s\"/>", remoteFile);


		if(SendCmd(MODULE_MSG_LOGIN, 0, lpData, nLength)<0)
			GLOGE("send CMD err!");
	}

	TaskFileRecv::~TaskFileRecv() {

		GLOGW("file seek:%ld\n", ftell(mwFile));

		if(mwFile != NULL)
			fclose(mwFile);

		mRecvBuffer.releaseMem();
	}

	int TaskFileRecv::sendEx(void*data, int len) {
		int leftLen = len, iRet = 0;

		struct timeval timeout;
		int sockId = mSid.mKey;
		do {
			iRet = send(sockId, (char*)data+len-leftLen, leftLen, 0);

			if(iRet<0) {
				//GLOGE("send data errno:%d ret:%d.", errno, iRet);
				switch(errno) {
				case EAGAIN:
					usleep(2000);
					continue;

				case EPIPE:
					break;
				}
				return iRet;
			}

			leftLen -= iRet;

		}while(leftLen>0);

		return len - leftLen;
	}

	int TaskFileRecv::SendCmd(int dwCmd, int dwIndex, void* lpData, int nLength)
	{
		int iRet = -1;
		NET_CMD nc;
		memset(&nc,0,sizeof(nc));
		nc.dwFlag = NET_FLAG;
		nc.dwCmd = dwCmd;
		nc.dwIndex = dwIndex;
		nc.dwLength = nLength;
		if ((iRet = sendEx(&nc, sizeof(nc)))<0)
		{
			GLOGE("send cmd err len = %d", nLength);
			return iRet;
		}

		GLOGW("send head len:%d \n", sizeof(nc));
		if (nLength == 0)
		{
			return 0;
		}
		if ((iRet = sendEx(lpData, nLength))<0)
		{
			GLOGE("send lpdata err len = %d",nLength);
			return iRet;
		}
		GLOGW("send data len:%d lpData:%s\n", nLength, lpData);
		return iRet;
	}

	int TaskFileRecv::readBuffer() {
		int ret = -1;
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		if(mRecvBuffer.bProcCmmd) {
			ret = recv(mSid.mKey, mRecvBuffer.data+hasRecvLen, mPackHeadLen-hasRecvLen, 0);

			if(ret>0) {
				hasRecvLen+=ret;
				if(hasRecvLen==mPackHeadLen) {
					LPNET_CMD head = (LPNET_CMD)mRecvBuffer.data;
					mRecvBuffer.totalLen  = head->dwLength;
					mRecvBuffer.bProcCmmd = false;
					hasRecvLen = 0;

					GLOGE("playback flag:%08x totalLen:%d ret:%d\n", head->dwFlag, mRecvBuffer.totalLen, ret);
					ret = recvPackData();
				}
			}
		}//
		else{
			ret = recvPackData();
		}

		//GLOGE("--------1-----------recvPackData ret:%d\n",ret);
		//EventCall::addEvent( mSess, EV_READ, -1 );
		return ret;
	}

	int TaskFileRecv::recvPackData() {
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		int ret = recv(mSid.mKey, mRecvBuffer.data+mPackHeadLen+hasRecvLen, mRecvBuffer.totalLen-hasRecvLen, 0);
		//GLOGE("-------------------recvPackData ret:%d\n",ret);
		if(ret>0) {
			hasRecvLen += ret;
			if(hasRecvLen==mRecvBuffer.totalLen) {

				int lValueLen;
			    char acValue[256] = {0};	//new char[256];
			    memset(acValue, 0, 256);
				LPNET_CMD pCmdbuf = (LPNET_CMD)mRecvBuffer.data;
				LPLOGIN_RET lpRet;
				LPFILE_GET  lpFrame;
				switch(pCmdbuf->dwCmd) {

					case MODULE_MSG_CONTROL_PLAY:
						break;

					case MODULE_MSG_VIDEO:
						lpFrame 		= (LPFILE_GET)(pCmdbuf->lpData);
						fwrite(lpFrame->lpData , 1 , lpFrame->nLength , mwFile);
						GLOGW("frame len:%d\n", lpFrame->nLength);
						break;

					case MODULE_MSG_LOGINRET:
						LPLOGIN_RET lpRet = (LPLOGIN_RET)(mRecvBuffer.data+mPackHeadLen);
						LPFILE_INFO lpInfo= (LPFILE_INFO)lpRet->lpData;
						mTotalLen = lpInfo->tmEnd;

						GLOGW("mTotalLen:%d\n", mTotalLen);

						char szCmd[100];
						int len = sprintf(szCmd, "<control name=\"start\" tmstart=\"%d\" tmend=\"%d\" />", 0, mTotalLen);
						SendCmd(MODULE_MSG_CONTROL_PLAY, 0, szCmd,len + 1);
						break;
				}

			    //GLOGE("recv total:%s", mRecvBuffer.buff);

			    mRecvBuffer.reset();
			}
		}
		else if(ret == 0) {

		}

		return ret;
	}

	int TaskFileRecv::writeBuffer() {

		return 0;
	}


