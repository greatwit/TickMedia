/*
 * FileName:       gs_tlib.h
 * Author:         mingjiawan  Version: 2.0  Date: 2011-10-22
 * Description:    
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

//#include "global.h"
#include <pthread.h>


#ifdef __cplusplus
	  extern "C"{
#endif

#define FRAME_MAX_MESSAGE_BUF_SIZE	(128*1024)
#define FRAME_SOCKET_TIMEOUT 		(2000)   //��λ����

#define	USE_FRAME
#define	MAX_BLOCKSIZE	(300*1024)//1024*1024*2
#define	MAX_BLOCKFRAME	(30)
#define VIDEO	1
#define AUDIO	2

#ifndef false
#define false	0
#endif

#ifndef true
#define true	1
#endif

#define FRAME_VIDEO_I	1
#define FRAME_VIDEO_P	2
#define FRAME_AUDIO		3
#define FRAME_TALKAUDIO			4

#define AUDCODEC_G711_ULAW 1
#define AUDCODEC_G711_ALAW 2
#define AUDCODEC_PCM 3
#define AUDCODEC_AAC 4

#define PACKED		__attribute__((packed, aligned(1))) //
#define PACKED4		__attribute__((packed, aligned(4))) //

#define ZeroMemory(p,t) memset(p,0,t)

#pragma   pack(1)
typedef struct TagFrameHead
{
	unsigned int	gs_frame_type;//video frame type
	unsigned int	gs_av_type;  // 1:means video,2:means audio
	unsigned int	gs_size;//audio or video data size
	unsigned int	gs_frameRate_samplingRate;//video frame rate or audio samplingRate
	unsigned int	gs_timestamp;
	unsigned int	gs_video_cap;//video's capability
	unsigned int	gs_reserved; 
}TagFrameHead;
#pragma   pack()

//===============================================================
#define NET_FLAG	0xfefdfcfb



//cmds
enum MODULE_MSG_ID{
	MODULE_MSG_PING,
	MODULE_MSG_LOGIN ,
	MODULE_MSG_LOGINRET,

	MODULE_MSG_ADDMDL,
	MODULE_MSG_DELMDL,

	MODULE_MSG_GETDEV,
	MODULE_MSG_DEVINFO,

	MODULE_MSG_GETSERVER,
	MODULE_MSG_SERINFO,

	MODULE_MSG_GETSSERVER,
	MODULE_MSG_SSERINFO,	//10
	
	MODULE_MSG_GETSERVICE,
	MODULE_MSG_SERVICEINFO,

	MODULE_MSG_ADDSERVER,
	MODULE_MSG_MODSERVER,
	MODULE_MSG_DELSERVER,

	MODULE_MSG_ADDSSVR,
	MODULE_MSG_MODSSVR,
	MODULE_MSG_DELSSVR,

	MODULE_MSG_ADDSVC,
	MODULE_MSG_DELSVC,		//20

	MODULE_MSG_ADDDEV,
	MODULE_MSG_MODDEV,
	MODULE_MSG_DELDEV,

	MODULE_MSG_POINTVALUE,//������ݡ�����������״̬��������Ϣ�ȡ���Ŀǰ����������������״̬�������벻����
	MODULE_MSG_VIDEO,
	MODULE_MSG_CHCOUNT,

	MODULE_MSG_GETINDEX,
	MODULE_MSG_EXERET,

	MODULE_MSG_STARTSERVICE,
	MODULE_MSG_STOPSVC,		//30

	MODULE_MSG_WRITEPV,
	MODULE_MSG_MANAGEUSERLOGIN,
	MODULE_MSG_GETSVRUSER,
	MODULE_MSG_USERRET,

	MODULE_MSG_AddUser,//����û�
	MODULE_MSG_ModifyUser,//�޸��û�
	MODULE_MSG_DeleteUser,//ɾ���û�
	MODULE_MSG_QueryUser,//��ѯ�û�

	MODULE_MSG_AddUserAuthority,//����û�Ȩ��
	MODULE_MSG_ModifyUserAuthority,//�޸��û�Ȩ��	//40
	MODULE_MSG_DeleteUserAuthority,//ɾ���û�Ȩ��
	MODULE_MSG_QueryUserAuthority,//��ѯ�û�Ȩ��

	MODULE_MSG_QueryLoginfo,//��ѯ��־--------------����Ϊ���������
	MODULE_MSG_QueryLoginfoResponse,//��ѯ��־��Ӧ-----------------����Ϊ����������
	MODULE_MSG_QueryUserResponse,//��ѯ�û���Ӧ
	MODULE_MSG_QueryUserAuthorityResponse,//��ѯ�û�Ȩ����Ӧ
	MODULE_MSG_SYSUSERLOGIN,//���ĵ�¼���������
	MODULE_MSG_AddLoginfo,//�����־
	MODULE_MSG_AddModuleType,//��ӷ���ģ������
	MODULE_MSG_DeleteModuleType,//ɾ������ģ������		//50
	MODULE_MSG_AddService_Device,//��ӷ������豸�İ�
	MODULE_MSG_DeleteService_Device,//ɾ���������豸�İ�
	MODULE_MSG_GETAUDIOHEAD,
	MODULE_MSG_AUDIODATA,
	MODULE_MSG_ModifyService_Device,//�޸ķ������豸�İ�
	MODULE_MSG_MODDEVRESPONSE,//�޸ķ������豸��Ӧ
	MODULE_MSG_STARTSEND,
	MODULE_MSG_ENDSEND,
	MODULE_MSG_DEVPOINT,
	MODULE_MSG_SETPARAM,//�����豸����		//60
	MODULE_MSG_GETPARAM,//��ȡ�豸����
	MODULE_MSG_DEVPARAM,
	
	MODULE_MSG_MANAGERGETSERVER,//����ͻ��˻�ȡҵ�����˷����豸��Ϣ
	MODULE_MSG_MANAGERSERINFO,//����ͻ��˻�ȡҵ�����˷����豸��Ϣ
	MODULE_MSG_MANAGERINFO,//����ͻ��˻�ȡҵ�����˷����豸��Ϣ
	
	MODULE_MSG_GETDEVSTATUS,
	MODULE_MSG_DEVSTATUS,
	MODULE_MSG_CONTROL_PLAY,
	MODULE_MSG_DATAEND,
	MODULE_MSG_SECTION_END,		//70
	MODULE_MSG_GETDEVICE_SVR_INFO,//ͨ��sn��ȡ���Ѱ󶨵�������豸������Ӧ��ҵ�������ip�����·����Ӧ��sip����Ķ˿�22103
	MODULE_MSG_RESDEVICE_SVR_INFO,
	MODULE_MSG_DEVWORKST,
	MODULE_MSG_OPENAUIDO,
	MODULE_MSG_CLOSEAUDIO,
	MODULE_MSG_USERMSGDATA,
	MODULE_MSG_UMDRET,
	MODULE_MSG_SENDMESSAGE, // ��Ϣ���͵��豸��
	MODULE_MSG_GETOPENVIDEOPARAM,//��ȡ����Ƶ��صĲ���
	MODULE_MSG_ONSYSTEM_LONGIN,//��ʼ������Ϣ�����ip�Ͷ˿ڵ���Ϣ��	//80
	MODULE_MSG_RESPONSEMESSAGE_IP_PORT_INFO,//��ȡ��Ϣ�����ip�Ͷ˿ڵ���Ϣ��
	MODULE_MSG_REQUESTMESSAGE_IP_PORT_INFO,//�ظ���Ϣ�����ip�Ͷ˿ڵ���Ϣ��
	MODULE_MSG_DEV_REGISTER_MESSAGE,//�豸ע�ᵽ��Ϣ����                              ms��Ҫ���͵���Ϣ
	MODULE_MSG_DEV_UNREGISTER_MESSAGE,//�豸ע������Ϣ����                            ms��Ҫ���͵���Ϣ
	MODULE_MSG_GETMSGSVR,//׼��ͬ���б�                                              ms��Ҫ���͵���Ϣ
	MODULE_MSG_GETDEVLIST,//ͬ���б�׼��ok�����������Է���Ϣ�������ҿ���ת����
	MODULE_MSG_TRANMSG,
	MODULE_MSG_USERMSG,
	MODULE_MSG_GETMSGSERVICEVALUE,//��Ϣ����������豸����״��

	MODULE_MSG_SEEK_CMPD,		//90
	MODULE_MSG_NEEDDATA,
	MODULE_MSG_GETGBDEVINFO,
	MODULE_MSG_GBDEVINFO,
	MODULE_MSG_CRYDATA,
	MODULE_MSG_MODPARAM,
	MODULE_MSG_GETSVCID,
	MODULE_MSG_GETDISKINFO,
	MODULE_MSG_DISKINFO,
	MODULE_MSG_RECORDFILEINFO,
	MODULE_MSG_RESTARTSVC,		//100
	MODULE_MSG_EXECCMD,
	MODULE_MSG_CHANGEIP,
	MODULE_MSG_GETFRIENDS,
};


//err code for client.
#define ERR_NOERROR		0
#define ERR_FUNADDR		1
#define ERR_NEWMEM		2
#define ERR_PARXML		3
#define ERR_NONEEDXML	4
#define ERR_PARAM		5
#define ERR_NOSURPORT	6
#define ERR_FAILE		7
#define ERR_DISCONNECT	8
#define ERR_MAXCOUNT	9
#define ERR_INITDB		10
#define ERR_SNDERROR	11
#define ERR_NETSERVER	12
#define ERR_CONNECT		13
#define ERR_NETSEND		14
#define ERR_QUERYDB		15
#define ERR_OBJERROR	16
#define ERR_NETRECV		17
#define ERR_MSGERROR	18
#define ERR_NOOBJECT	19
#define ERR_NOTWORK		20
#define ERR_DEVTYPE		21
#define ERR_OBJEXIST	22
#define ERR_TIMEOUT		23
#define ERR_DOING		24

////#pragma   pack(1)
typedef struct tagNET_HEAD
{
	unsigned int dwFlag;
	unsigned int dwCmd;
	unsigned int dwIndex;
	unsigned int dwLength;
}NET_HEAD,*LPNET_HEAD;
////#pragma   pack()

//#pragma   pack(1)
typedef struct tagNET_CMD
{
	unsigned int dwFlag;
	unsigned int dwCmd;
	unsigned int dwIndex;
	unsigned int dwLength;
	char  lpData[];
}NET_CMD,*LPNET_CMD;
//#pragma   pack()


//#pragma   pack(1)
typedef struct tagLOGIN_RET
{
	unsigned int lRet;
	int nLength;
	char lpData[1024];
}LOGIN_RET,*LPLOGIN_RET;
//#pragma   pack()

typedef struct tagFILE_GET
{
	unsigned int dwPos;
	int nLength;
	char lpData[];
}FILE_GET,*LPFILE_GET;

//typedef struct tagFILE_DATA
//{
//	DWORD dwPos;
//	int nLength;
//	char lpData[];
//}FILE_DATA,*LPFILE_DATA;

//#pragma   pack(1)
typedef struct tagAV_FRAME
{
	unsigned int dwFrameType;
	int   nLength;
	unsigned int dwTick;
	unsigned int dwTm;
	char  lpData[];
}AV_FRAME,*LPAV_FRAME;
//#pragma   pack()



//��¼������
//�ͻ��˷���ָ�
/*
	char lpData[30960] = "<play path=\"D:test.mp4\"/>";
	int nLength = strlen(lpData);
	NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_LOGIN;
	nc.dwIndex = 0;
	nc.dwLength = nLength;
memcpy(nc.lpData��lpData, nLength);


����������ָ�
	char lpRet[sizeof(NET_CMD) + sizeof(LOGIN_RET)];
	ZeroMemory(lpRet,sizeof(lpRet));
	((LPNET_CMD)lpRet)->dwFlag = NET_FLAG;
	((LPNET_CMD)lpRet)->dwCmd = MODULE_MSG_LOGINRET;
	((LPNET_CMD)lpRet)->dwLength = sizeof(LOGIN_RET);
	LPLOGIN_RET lpLR = (LPLOGIN_RET)((LPNET_CMD)lpRet)->lpData;
	lpLR->nLength = sizeof(lpLR->lpData);

	lpLR->lRet = m_pManager->GetResult(MODULE_MSG_LOGIN,
		((LPNET_CMD)lpData)->lpData,
		((LPNET_CMD)lpData)->dwLength,
		lpLR->lpData,&lpLR->nLength);

((LPNET_CMD)lpRet)->dwLength += (lpLR->nLength - sizeof(lpLR->lpData));

*/



/*
������
����������ָ�
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_PING;
	nc.dwIndex = 0;
	nc.dwLength = 0;
�ͻ�����ָ���
*/

/*
��ʼ�ط�
�ͻ��˷���ָ�
char lpData[1000] = "<control name=\"start\" tmstart=\"0\" tmend=\"3600\" />";// tmstart��tmendΪ��ʼʱ�䣬����Ϊ��λ
	int nLength = strlen(lpData);
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_CONTROL_PLAY;
	nc.dwIndex = 0;
	nc.dwLength = nLength;
memcpy(nc.lpData��lpData, nLength);
����˷���ָ�
�޷���ָ��������̣߳�׼��������
*/

/*
�������ݷ���
�ͻ��˷���ָ�
char lpData[1000] = "<control name=\"setpause\" value=\"0\"/>"
	int nLength = strlen(lpData);
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_CONTROL_PLAY;
	nc.dwIndex = 0;
	nc.dwLength = nLength;
	memcpy(nc.lpData��lpData, nLength);

����˷���������ָ�
AV_FRAME af;
memset(&af,0,sizeof(af));
	af.dwFrameType = nFrameType;
	af.dwTick = pts;
	af.dwTm = pts;
	af.nLength = nLength;// nLengthΪһ֡���ݵĳ���
memcpy(af.lpData��lpData, nLength);
NET_CMD nc;


ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_VIDEO;
	nc.dwIndex = 0;
	nc.dwLength = nLength+sizeof(AV_FRAME);
memcpy(nc.lpData��&af, nLength+sizeof(AV_FRAME));


����˷���һ��������������ָ�//������������300֮֡�󣬷��͵�ָ��
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_SECTION_END;
	nc.dwIndex = 0;
	nc.dwLength = 0;

����˷��������ļ��ط���ָ�
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_DATAEND;
	nc.dwIndex = 0;
	nc.dwLength = 0;


��λ�ط�
�ͻ��˷���ָ�
char lpData[1000] = "<control name=\"seek\" value=\"3000\"/>" //3000������Ƶ�ļ���3��
	int nLength = strlen(lpData);
NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_CONTROL_PLAY;
	nc.dwIndex = 0;
	nc.dwLength = nLength;
	memcpy(nc.lpData��lpData, nLength);



����˷���ָ�
	NET_CMD nc;
	ZeroMemory(&nc,sizeof(nc));
	nc.dwFlag = NET_FLAG;
	nc.dwCmd = MODULE_MSG_SEEK_CMPD;
	nc.dwIndex = 0;
	nc.dwLength = 0;
*/
//==============================================================

#define ARRAY_NUM   100

// lpLR->lpData Ϊ�ļ�ͷ��Ϣ���ṹ�嶨�����£�
//#pragma   pack(1)
typedef struct tagPLAYER_INIT_INFO
{
	int nCodeID;
	int nFps;
	int nWidth;
	int nHeigth;
	int nSampleRate;
	int nBistspersample;
	int nChannel;
	int nAudioFormat;
	int sample_fmt;
	unsigned long long channel_layout;	
	int nCodecFlag;
	int bits_per_sample;
	int bit_rate;
	int me_method;
	int bit_ratetolerance;
	int block_align;
	
	int gop_size;
	int frame_size;
	int frame_number;
	int ildct_cmp;
	int me_subpel_quality;
	int mb_lmax;
	int mb_lmin;
	int me_penalty_compensation;
	float qblur;
	int  flags;
	int extsize;
	char extdata[ARRAY_NUM];
	int  nVideoExtSize;
	char	videoExtData[ARRAY_NUM];
}PLAYER_INIT_INFO,*LPPLAYER_INIT_INFO;//�ļ�ͷ��Ϣ
//#pragma   pack(0)

#pragma   pack(1)
typedef struct tagFILE_INFO
{
	PLAYER_INIT_INFO pi;
	unsigned int tmStart;
	unsigned int tmEnd;
}FILE_INFO,*LPFILE_INFO;//�ļ���Ϣ
#pragma   pack(0)

#pragma   pack(1)
typedef struct tagFILE_INFOEX
{
	FILE_INFO *stFileInfo;
	char lpData[102400];
}FILE_INFOEX,*LPFILE_INFOEX;
#pragma   pack(0)

typedef struct _send_Package 
{
	int				nType_id;
	int				nFrameType;
	void			*pPackageData;
	int				nSize;
	int				nSendCount;
	pthread_mutex_t	refrence_lock;
	int				nRefrenceCount;
	AV_FRAME *av;
}send_Package;

typedef struct _listNode
{
	send_Package		*pNodeData;
	struct _listNode	*pNext;
}listNode;

typedef struct _sendList
{
	int				nlength;
	listNode		*lFirst;
	listNode		*lLast;
	pthread_mutex_t	list_lock;
}sendList;

typedef listNode *		LinkNodePtr;
typedef sendList *		LinkListPtr;
typedef send_Package *	LinkPackagePtr;

int initLinkList(LinkListPtr *list);
int insertLinkList(LinkListPtr *list ,LinkPackagePtr *data);
int removeLinkListNode(LinkListPtr *list);
int destroyLinkList(LinkListPtr *list);
void addRefrence(LinkPackagePtr *package);
void releaseTlibPackage(LinkPackagePtr *package);

typedef struct _FRAME_START_PARAM
{
    short sPort;
    char  acServerAddr[16] ;
    char  serial_number[64] ;
	char  device_place[256];
}PACKED FRAME_START_PARAM;

typedef struct _HIS_VIDEO_FILE_CONTEXT
{
    char    filename[128];
    unsigned char   ClientID[32];
    FILE    *fd;
    unsigned    fileLength;
    short   needClientID;
    short   flag;//0-stop; 1-start; 2-switch file
} PACKED HIS_VIDEO_FILE_CONTEXT;

typedef struct _cam_connection
{
	int				fd;
	int				nVChnID;
	int				nAChnID;
	sendList		*pSend_list;
	unsigned int	nBlockedSize;
	unsigned int	nBlockedFrame;
	unsigned int	nBlockedVideoFrame;
	struct _cam_connection	*pNext;
    int				send_audio_flag;
    int				send_video_flag;
    int				nRunState; //�����߳��Ƿ�������
	unsigned char	bWaitForIFrame;
	unsigned char	bIsServerConn;
	unsigned char	bIsLogin;
	unsigned char	bIsRecording;
    HIS_VIDEO_FILE_CONTEXT his_video_context;
} PACKED cam_connection;

void delay(int nMillisecond);
void hda_av_service_thread();
//void SendDataPackage(int type, int channel_id, unsigned char* data, unsigned int size, TagFrameHead *stSendHead);

void SendDataPackage(int type, unsigned char* data, TagFrameHead *stSendHead);
int SetIpncRS485MotorPtz(unsigned int nPtzcmd, unsigned char cbaudrate);
void RemoveSendPackage(cam_connection *pConn);
void releaseAllPackage(cam_connection *pConn);
#ifdef __cplusplus
}
#endif

#endif

