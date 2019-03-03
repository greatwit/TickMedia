
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TaskVideoRecv.hpp"
#include "BufferCache.hpp"
#include "EventCall.hpp"

#include "event.h"
#include "h264.h"


#define TAG "TaskVideoRecv"
#include "basedef.h"


#ifdef 	__ANDROID__
#define	FILE_PATH	"/sdcard/w.h264"
#else
#define	FILE_PATH	"recv.h264"
#endif

const int	 BUFFER_LEN  = 1024*1024+1500;

	TaskVideoRecv::TaskVideoRecv( Session*sess, Sid_t &sid )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
				,mvBase(NULL)
				,mwFile(NULL)
	{
		mSendBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(BUFFER_LEN);

		//mwFile = fopen("real.h264", "w");

		LPNET_CMD	pCmd = (LPNET_CMD)mSendBuffer.cmmd;
		int nLength = sprintf(pCmd->lpData, "<play real=\"%s\"/>", "/sdcard/ModuleTest/720p.h264");
		pCmd->dwFlag 	= NET_FLAG;
		pCmd->dwCmd 	= MODULE_MSG_LOGIN;
		pCmd->dwIndex 	= 0;
		pCmd->dwLength 	= nLength;
		mSendBuffer.totalLen 	= sizeof(NET_CMD)+nLength;
		mSendBuffer.bProcCmmd 	= true;
		int ret = tcpSendData();
	}


	TaskVideoRecv::TaskVideoRecv( Session*sess, Sid_t &sid, char*filepath )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
				,mvBase(NULL)
	{
		mSendBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(BUFFER_LEN);

		mwFile = fopen(FILE_PATH, "w");


		LPNET_CMD	pCmd = (LPNET_CMD)mSendBuffer.cmmd;
		int nLength = sprintf(pCmd->lpData, "<play path=\"%s\"/>", filepath);
		pCmd->dwFlag 	= NET_FLAG;
		pCmd->dwCmd 	= MODULE_MSG_LOGIN;
		pCmd->dwIndex 	= 0;
		pCmd->dwLength 	= nLength;
		mSendBuffer.totalLen 	= sizeof(NET_CMD)+nLength;
		mSendBuffer.bProcCmmd 	= true;
		int ret = tcpSendData();
	}


	TaskVideoRecv::~TaskVideoRecv() {

		if(mwFile != NULL) {
			GLOGW("file seek:%ld\n", ftell(mwFile));

			fclose(mwFile);
			mwFile = NULL;
		}

		mRecvBuffer.releaseMem();

		if(mvBase) {
			delete mvBase;
			mvBase = NULL;
		}
	}

	void TaskVideoRecv::setBase(VideoBase*base) {
		mvBase = base;
	}

	int TaskVideoRecv::sendEx(void*data, int len) {
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

	int TaskVideoRecv::tcpSendData()
	{
		int ret = 0;
		int &hasProcLen = mSendBuffer.hasProcLen;
		if(mSendBuffer.bProcCmmd) {
			ret = sendEx(mSendBuffer.cmmd+hasProcLen, mSendBuffer.totalLen-hasProcLen);
			if(ret>0)
				hasProcLen += ret;
			else
				GLOGE("tcpSendData cmd errno:%d ret:%d.", errno, ret);

			if(hasProcLen == mSendBuffer.totalLen) {
				mSendBuffer.reset();
			}
		}

		return ret;
	}

	int TaskVideoRecv::SendCmd(int dwCmd, int dwIndex, void* lpData, int nLength)
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

		GLOGW("send head len:%u \n", sizeof(nc));
		if (nLength == 0)
		{
			return 0;
		}
		if ((iRet = sendEx(lpData, nLength))<0)
		{
			GLOGE("send lpdata err len = %d",nLength);
			return iRet;
		}
		GLOGW("send data len:%d lpData:%s\n", nLength, (char*)lpData);
		return iRet;
	}

	int TaskVideoRecv::readBuffer() {
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

	int TaskVideoRecv::recvPackData() {
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
						if(mwFile)
							fwrite(lpFrame->lpData, 1, lpFrame->nLength, mwFile);
						GLOGW("frame len:%d\n", lpFrame->nLength);

						if(mvBase)mvBase->onDataComing(lpFrame->lpData, lpFrame->nLength);
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

	int TaskVideoRecv::writeBuffer() {

		return 0;
	}


