
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TaskVideoRecv.hpp"
#include "BufferCache.hpp"

#include "h264.h"
#include "basedef.h"

#ifdef 	__ANDROID__
#define	FILE_PATH	"/sdcard/w.h264"
#else
#define	FILE_PATH	"w.h264"
#endif

	TaskVideoRecv::TaskVideoRecv( Session*sess, Sid_t &sid )
				:mPackHeadLen(sizeof(PACK_HEAD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
	{
		mSendBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(720*1280);

		mwFile = fopen(FILE_PATH, "w");

		char lpData[2048];
		int nLength = sprintf(lpData, "<play path=\"%s\"/>", "/sdcard/camera.h264");

		LPNET_CMD	pCmd = (LPNET_CMD)mSendBuffer.cmmd;
		pCmd->dwFlag 	= NET_FLAG;
		pCmd->dwCmd 	= MODULE_MSG_LOGIN;
		pCmd->dwIndex 	= 0;
		pCmd->dwLength 	= nLength;
		mSendBuffer.totalLen 	= sizeof(NET_CMD)+nLength;
		mSendBuffer.bProcCmmd 	= true;
		int ret = tcpSendData();
	}

	TaskVideoRecv::TaskVideoRecv( Session*sess, Sid_t &sid, char*filepath )
				:mPackHeadLen(sizeof(PACK_HEAD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
	{
		mSendBuffer.reset();
		mRecvBuffer.reset();
		mRecvBuffer.createMem(720*1280);

		mwFile = fopen(FILE_PATH, "w");


		LPNET_CMD cmd = (LPNET_CMD)mSendBuffer.cmmd;
		int nLength = sprintf(cmd->lpData, "<play path=\"%s\"/>", filepath);
		cmd->dwFlag   = NET_FLAG;
		cmd->dwCmd    = MODULE_MSG_LOGINRET;
		cmd->dwIndex  = 0;
		cmd->dwLength = nLength;
		mSendBuffer.totalLen  = sizeof(NET_CMD) + nLength;
		mSendBuffer.bProcCmmd = true;

		int ret = tcpSendData();
		GLOGE("tcpSendData data ret:%d.", ret);
	}

	TaskVideoRecv::~TaskVideoRecv() {

		if(mwFile != NULL)
			fclose(mwFile);
	}

	int TaskVideoRecv::StartTask() {
		return 0;
	}

	int TaskVideoRecv::StopTask() {
		return 0;
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
		if(mSendBuffer.bProcCmmd) {
			ret = sendEx(mSendBuffer.cmmd+mSendBuffer.hasProcLen, mSendBuffer.totalLen-mSendBuffer.hasProcLen);
			if(ret>0)
				mSendBuffer.hasProcLen += ret;
			else
				GLOGE("tcpSendData cmd errno:%d ret:%d.", errno, ret);

			GLOGE("tcpSendData ret:%d.", ret);

			if(mSendBuffer.hasProcLen == mSendBuffer.totalLen) {
					mSendBuffer.reset();
			}
		}
		return ret;
	}

	int TaskVideoRecv::readBuffer() {
		int ret = -1;
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		if(mRecvBuffer.bProcCmmd) {
			ret = recv(mSid.mKey, mRecvBuffer.data+hasRecvLen, mPackHeadLen-hasRecvLen, 0);
			GLOGE("readBuffer ret:%d\n", ret);
			if(ret>0) {
				hasRecvLen+=ret;
				if(hasRecvLen==mPackHeadLen) {
					LPNET_CMD head = (LPNET_CMD)mRecvBuffer.data;
					mRecvBuffer.totalLen  = head->dwLength;
					mRecvBuffer.bProcCmmd = false;
					hasRecvLen = 0;

					GLOGE("playback flag:%08x ?totalLen:%d ret:%d\n", head->dwFlag, mRecvBuffer.totalLen, ret);
					ret = recvPackData();
				}
			}
		}//
		else {
			ret = recvPackData();
		}

		return ret;
	}

	int TaskVideoRecv::recvPackData() {
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		int ret = recv(mSid.mKey, mRecvBuffer.data+mPackHeadLen+hasRecvLen, mRecvBuffer.totalLen-hasRecvLen, 0);
		GLOGE("-------------------recvPackData ret:%d\n",ret);
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
						//SendCmd(MODULE_MSG_CONTROL_PLAY, 0, szCmd,len + 1);
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
		GLOGE("TaskVideoRecv write event\n");
		return 0;
	}


