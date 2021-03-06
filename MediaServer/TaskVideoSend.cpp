
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TaskVideoSend.hpp"
#include "BufferCache.hpp"
#include "EventCall.hpp"

#include "event.h"
#include "basedef.h"
#include "protocol.h"
#include "net_protocol.h"

#undef   _FILE_OFFSET_BITS
#define  _FILE_OFFSET_BITS	64


//#define 	SEG_FRAME_COUNT 10


	TaskVideoSend::TaskVideoSend( Session*sess, Sid_t& sid, char*filename )
				:mPackHeadLen(sizeof(NET_CMD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mFileLen(0)
				,mHasReadLen(0)
				,mFrameCount(0)
				,mbSendingData(true)
	{
		mRecvBuffer.reset();
		mSendBuffer.reset();
		mSendBuffer.createMem(720*1280);

		mpFile = OpenBitstreamFile( filename );
	    struct stat buf;
	    stat(filename, &buf);
	    mFileLen = buf.st_size;
//		fseek( mpFile, 0, SEEK_END );
//		mFileLen = ftell( mpFile );
//		fseek( mpFile, 0, SEEK_END );
//		rewind( mpFile );

		char *lpRet   = mSendBuffer.cmmd;
		LPNET_CMD cmd = (LPNET_CMD)lpRet;
		cmd->dwFlag   = NET_FLAG;
		cmd->dwCmd    = MODULE_MSG_LOGINRET;
		cmd->dwIndex  = 0;
		cmd->dwLength = sizeof(LOGIN_RET);

		LOGIN_RET loginRet 	= {0};
		FILE_INFO info 		= {0};
		info.tmEnd 			= mFileLen;
		loginRet.nLength 	= sizeof(FILE_INFO);
		loginRet.lRet 		= ERR_NOERROR;
		memcpy(loginRet.lpData, &info, sizeof(FILE_INFO));
		memcpy(cmd->lpData, &loginRet, sizeof(LOGIN_RET));

		mSendBuffer.totalLen 	= sizeof(NET_CMD) + sizeof(LOGIN_RET);
		mSendBuffer.bProcCmmd 	= true;
		int ret = tcpSendData();

		GLOGE("file %s len:%u\n", filename, mFileLen);//get_filesize(filename)
	}


	TaskVideoSend::~TaskVideoSend() {

		if(mpFile != NULL) {
			CloseBitstreamFile(mpFile);
			mpFile = NULL;
		}

		mMsgQueue.clearQueue();
		mSendBuffer.releaseMem();
	}

	int TaskVideoSend::sendVariedCmd(int iVal) {
		LPNET_CMD	pCmd = (LPNET_CMD)mSendBuffer.cmmd;
		pCmd->dwFlag 	= NET_FLAG;
		pCmd->dwCmd 	= iVal;
		pCmd->dwIndex 	= 0;
		pCmd->dwLength 	= 0;
		mSendBuffer.totalLen = sizeof(NET_CMD);
		mSendBuffer.bProcCmmd = true;
		int ret = tcpSendData();
		//GLOGE("-------------------sendVariedCmd:%d", iVal);
		return ret;
	}

	//Here further processing is needed.
	int TaskVideoSend::writeBuffer() {
		int ret = 0;
		if(feof(mpFile)) {
			//mRunning = false;
			GLOGW("read file done.");
			return 0;
		}

		if(mSendBuffer.totalLen==0) //take new data and send,totalLen is cmd len first
		{
				int size = GetAnnexbNALU(mpFile, mSendBuffer.data);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
				GLOGE("GetAnnexbNALU size:%d\n", size);

				if(size>0) {
					mSendBuffer.totalLen 	= sizeof(NET_CMD) + sizeof(FILE_GET);
					mSendBuffer.bProcCmmd 	= true;
					LPNET_CMD	 cmd 		= (LPNET_CMD)mSendBuffer.cmmd;
					LPFILE_GET frame 		= (LPFILE_GET)(cmd->lpData);
					cmd->dwFlag 			= NET_FLAG;
					cmd->dwCmd 				= MODULE_MSG_VIDEO;
					cmd->dwIndex 			= 0;
					frame->dwPos 			= ftell(mpFile);

					mSendBuffer.dataLen  	= size;
					cmd->dwLength 			= mSendBuffer.dataLen+sizeof(FILE_GET); 	//cmd incidental length
					frame->nLength  		= mSendBuffer.dataLen;
				}
				else{
					ret = pushSendCmd(MODULE_MSG_DATAEND);
					return OWN_SOCK_EXIT;
			}
		}
		ret = tcpSendData();

		return ret;
	}

	int TaskVideoSend::setHeartCount() {
//			mMsgQueue.push(MODULE_MSG_PING);
//
//			if( !mbSendingData)
//				EventCall::addEvent( mSess, EV_WRITE, -1 );
//
//			//GLOGE("setHeart----------------\n");
//
//		return mMsgQueue.getSize();
		return 0;
	}

	int TaskVideoSend::sendEx(char*data, int len) {
		int leftLen = len, iRet = 0;

		struct timeval timeout;
		int sockId = mSid.mKey;
		do {
			iRet = send(sockId, data+len-leftLen, leftLen, 0);

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

	int TaskVideoSend::tcpSendData()
	{
		int ret = 0;
		if(mSendBuffer.bProcCmmd) {
			ret = sendEx(mSendBuffer.cmmd+mSendBuffer.hasProcLen, mSendBuffer.totalLen-mSendBuffer.hasProcLen);
			if(ret>0)
				mSendBuffer.hasProcLen += ret;
			else
				GLOGE("tcpSendData cmd errno:%d ret:%d.", errno, ret);

			if(mSendBuffer.hasProcLen == mSendBuffer.totalLen) {
					mSendBuffer.setToVideo();
			}
		}
		else//data
		{
			ret = sendEx((char*)mSendBuffer.data->buf+mSendBuffer.hasProcLen, mSendBuffer.totalLen-mSendBuffer.hasProcLen);
			if(ret>0)
				mSendBuffer.hasProcLen += ret;
			else
				GLOGE("tcpSendData dta errno:%d ret:%d .\n", errno, ret);

			if(mSendBuffer.hasProcLen == mSendBuffer.totalLen) {
				mSendBuffer.reset();
			}
		}
		return ret;
	}

	int TaskVideoSend::pushSendCmd(int iVal, int index) {
		int ret = 0;
		LPNET_CMD	pCmd = (LPNET_CMD)mSendBuffer.cmmd;
		switch(iVal) {
			case MODULE_MSG_DATAEND:
			case MODULE_MSG_SEEK_CMPD:
			case MODULE_MSG_SECTION_END:
			case MODULE_MSG_EXERET:
				pCmd->dwFlag 	= NET_FLAG;
				pCmd->dwCmd 	= iVal;
				pCmd->dwIndex 	= index;
				pCmd->dwLength 	= 0;

				mSendBuffer.totalLen = sizeof(NET_CMD);
				mSendBuffer.bProcCmmd = true;
				ret = tcpSendData();
				break;

			case MODULE_MSG_PING:
				if(mMsgQueue.getSize() < 10)
					mMsgQueue.push(MODULE_MSG_PING);
				break;
		}
		GLOGE("pushSendCmd value:%d ret:%d.\n", iVal, ret);

		return ret;
	}

	int TaskVideoSend::readBuffer() {
		int ret = -1;
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		if(mRecvBuffer.bProcCmmd) {
			ret = recv(mSid.mKey, mRecvBuffer.cmmd+hasRecvLen, mPackHeadLen-hasRecvLen, 0);
			if(ret>0) {
				hasRecvLen+=ret;
				if(hasRecvLen==mPackHeadLen) {
					LPNET_CMD head = (LPNET_CMD)mRecvBuffer.cmmd;
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
		return ret;
	}

	int TaskVideoSend::recvPackData() {
		int &hasRecvLen = mRecvBuffer.hasProcLen;
		int ret = recv(mSid.mKey, mRecvBuffer.cmmd+mPackHeadLen+hasRecvLen, mRecvBuffer.totalLen-hasRecvLen, 0);
		//GLOGE("-------------------recvPackData ret:%d\n",ret);
		if(ret>0) {
			hasRecvLen += ret;
			if(hasRecvLen==mRecvBuffer.totalLen) {

				int lValueLen;
			    char acValue[256] = {0};	//new char[256];
			    memset(acValue, 0, 256);
				LPNET_CMD pCmdbuf = (LPNET_CMD)mRecvBuffer.cmmd;
				if(pCmdbuf->dwCmd == MODULE_MSG_CONTROL_PLAY) {
					PROTO_GetValueByName(mRecvBuffer.cmmd, (char*)"name", acValue, &lValueLen);
					GLOGE("recv control commond acValue:%s\n",acValue);
					if (strcmp(acValue, "start") == 0) {
						memset(acValue, 0, 256);
						PROTO_GetValueByName(mRecvBuffer.cmmd, (char*)"tmstart", acValue, &lValueLen);
						GLOGE("tmstart:%d\n",atoi(acValue));

						memset(acValue, 0, 256);
						PROTO_GetValueByName(mRecvBuffer.cmmd, (char*)"tmend", acValue, &lValueLen);
						GLOGE("tmend:%d\n",atoi(acValue));
						EventCall::addEvent( mSess, EV_WRITE, -1 );
					}
					else if(strcmp(acValue, "setpause") == 0) {
						memset(acValue, 0, 256);
						PROTO_GetValueByName(mRecvBuffer.cmmd, (char*)"value", acValue, &lValueLen);
						int value = atoi(acValue);
						GLOGE("control setpause value:%d\n", value);
					}
					else if(strcmp(acValue, "send") == 0) {
						mbSendingData = true;
						EventCall::addEvent( mSess, EV_WRITE, -1 );
					}
				}
			    //GLOGE("recv total:%s", mRecvBuffer.buff);

			    mRecvBuffer.reset();
			}
		}
		else if(ret == 0) {

		}

		return ret;
	}



