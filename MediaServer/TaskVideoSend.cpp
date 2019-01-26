

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "TaskVideoSend.hpp"
#include "EventCall.hpp"

#include "event.h"
#include "net_protocol.h"
#include "basedef.h"

#ifdef 	__ANDROID__
#define	FILE_PATH	"/sdcard/camera_640x480.h264"
#else
#define	FILE_PATH	"h264/camera_640x480.h264"
#endif

	TaskVideoSend::TaskVideoSend( Session*sess, Sid_t& sid, char*filename )
				:mPackHeadLen(sizeof(PACK_HEAD))
				,TaskBase(sid)
				,mSess(sess)
				,mRecvDataLen(0)
				,mRecvHeadLen(0)
				,mTotalLen(0)
				,mSeqid(0)
				,mbSendingData(true)
	{
	    struct stat buf;
	    stat(filename, &buf);

		mwFile = OpenBitstreamFile( filename ); //camera_640x480.h264 //camera_1280x720.h264
		if(mwFile==NULL) {
			GLOGE("open file:%s failed.", filename);
		}

		mRecvBuffer.reset();
		mSendBuffer.reset();
		mSendBuffer.createMem(720*1280);

		char *lpRet   = mSendBuffer.cmmd;
		LPNET_CMD cmd = (LPNET_CMD)lpRet;
		cmd->dwFlag   = NET_FLAG;
		cmd->dwCmd    = MODULE_MSG_LOGINRET;
		cmd->dwIndex  = 0;
		cmd->dwLength = 8;//sizeof(LOGIN_RET);
		LOGIN_RET loginRet 	= {0};
		FILE_INFO info 		= {0};
		info.tmEnd 			= buf.st_size;
		loginRet.nLength 	= sizeof(FILE_INFO);
		loginRet.lRet 		= ERR_NOERROR;
		memcpy(loginRet.lpData, &info, sizeof(FILE_INFO));
		memcpy(cmd->lpData, &loginRet, sizeof(LOGIN_RET));

		mSendBuffer.totalLen 	= sizeof(NET_CMD) + sizeof(LOGIN_RET);
		mSendBuffer.bProcCmmd 	= true;
		int ret = tcpSendData();

		GLOGD("TaskVideoSend filename:%s send ret:%d \n", filename, ret);
	}

	TaskVideoSend::~TaskVideoSend() {
		if(mwFile != NULL) {
			CloseBitstreamFile(mwFile);
			mwFile = NULL;
		}
		mSendBuffer.releaseMem();
	}

	int TaskVideoSend::StartTask() {
		return 0;
	}

	int TaskVideoSend::StopTask() {
		return 0;
	}

	int TaskVideoSend::sendEx(char*data,int len) {
		int leftLen = len, iRet = 0;
		struct timeval timeout;
		int sockId = mSid.mKey;
		do {
			iRet = send(sockId, data+len-leftLen, leftLen, 0);
			if(iRet<0) {
				//GLOGE("send data errno:%d ret:%d.", errno, iRet);
				switch(errno) {
					case EAGAIN: usleep(2000); continue;
					case EPIPE: break;
				}
				return iRet;
			}
			leftLen -= iRet;

		}while(leftLen>0);

		return len - leftLen;
	}

	void TaskVideoSend::packetHead(int fid, short pid, int len, unsigned char type, LPPACK_HEAD lpPack) {
		memset(lpPack, 0, sizeof(PACK_HEAD));
		lpPack->type 		= type;
		lpPack->fid 		= htonl(fid);
		lpPack->pid			= htons(pid);
		lpPack->len 		= htonl(len);
	}

	int TaskVideoSend::tpcSendMsg(unsigned char msg) {
		int iRet = 0;
		char sendData[1500] = {0};
		packetHead(0, 0, mPackHeadLen, msg, (LPPACK_HEAD)sendData);
		iRet = sendEx(sendData, mPackHeadLen);
		return iRet;
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

	int TaskVideoSend::writeBuffer() {
		int ret = 0;
		if(feof(mwFile)) {
			//mRunning = false;
			GLOGW("read file done.");
			return 0;
		}

		if(mSendBuffer.totalLen==0) //take new data and send,totalLen is cmd len first
		{
				int size = GetAnnexbNALU(mwFile, mSendBuffer.data);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
				GLOGE("GetAnnexbNALU size:%d", mSendBuffer.data->len);

				if(size>0) {
					mSendBuffer.totalLen 	= sizeof(NET_CMD) + sizeof(FILE_GET);
					mSendBuffer.bProcCmmd 	= true;
					LPNET_CMD	 cmd 		= (LPNET_CMD)mSendBuffer.cmmd;
					LPFILE_GET frame 		= (LPFILE_GET)(cmd->lpData);
					cmd->dwFlag 			= NET_FLAG;
					cmd->dwCmd 				= MODULE_MSG_VIDEO;
					cmd->dwIndex 			= 0;
					frame->dwPos 			= ftell(mwFile);

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
