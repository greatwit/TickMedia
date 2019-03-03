
#include "ActorStation.hpp"
#include "TcpClient.hpp"
#include "TcpServer.hpp"
#include "RealCameraCodec.h"
#include "UpperNdkEncodec.h"

#include <jni.h>
#include "basedef.h"

#include <android/native_window_jni.h>

#define REG_PATH "com/great/happyness/medialib/NativeNetMedia"

JavaVM*		 g_javaVM		= NULL;
jclass 		 g_mClass		= NULL;

ActorStation mStatiion;
TcpClient	 *mpClient		= NULL;
TcpServer	 *mpServer		= NULL;
RealCameraCodec *gRealCam   = NULL;
UpperNdkEncodec *gUpcamEnc	= NULL;

/////////////////////////////////////////////////////Server and real view////////////////////////////////////////////////////////

static jboolean StartNetWork(JNIEnv *env, jobject) {
	if(mStatiion.isRunning() == 0) {
		mStatiion.startup();
		return true;
	}
	return false;
}

static jboolean StopNetWork(JNIEnv *env, jobject) {
	if(mStatiion.isRunning() == 1) {
		mStatiion.shutdown();
		return true;
	}
	return false;
}

static jboolean StartServer(JNIEnv *env, jobject obj, jstring localip, jint destport)
{
	if(mpServer==NULL) {
		mpServer = new TcpServer("", destport);
		mpServer->setMaxThreads( 10 );
		mpServer->registerEvent(mStatiion.getEventArg());
	}

	g_mClass = (jclass)env->NewGlobalRef(obj);

	return true;
}

static jboolean StopServer(JNIEnv *env, jobject)
{
	if(mpServer!=NULL) {
		mpServer->shutdown();
		delete mpServer;
		mpServer = NULL;
	}
	return true;
}

///////////////////////////////////////////////////////upper cam//////////////////////////////////////

static jboolean SetUpcamEncView(JNIEnv *env, jobject, jint sessionId)
{
	if(mpServer && gRealCam) {

		mpServer->setRealView(sessionId, gUpcamEnc);
		return true;
	}
	return false;
}

static jboolean StartUpcamEncodec(JNIEnv *env, jobject) {
	bool ret = true;
	if(gUpcamEnc==NULL)
		gUpcamEnc = new UpperNdkEncodec();

	gUpcamEnc->startPlayer(0, 0);

	GLOGW("StartCamndkEncodec \n");

	return ret;
}

static jboolean StopUpcamEncodec(JNIEnv *env, jobject) {
	bool ret = true;
	if(gUpcamEnc) {
		gUpcamEnc->stopPlayer();
		delete gUpcamEnc;
		gUpcamEnc = NULL;
	}
	return ret;
}

static jboolean UpcamEncSetInt32(JNIEnv *env, jobject, jstring key, jint value) {
	jboolean isOk  = JNI_FALSE;
	if(gUpcamEnc==NULL)
		gUpcamEnc = new UpperNdkEncodec();

	const char *ck = env->GetStringUTFChars(key, &isOk);
	gUpcamEnc->setInt32(ck, value);
	env->ReleaseStringUTFChars(key, ck);
	return true;
}

static jboolean UpcamEncProvide(JNIEnv* env, jobject, jbyteArray javaCameraFrame, jint length) {

	jbyte* cameraFrame = env->GetByteArrayElements(javaCameraFrame, NULL);
	gUpcamEnc->ProvideNV21Data(reinterpret_cast<uint8_t*>(cameraFrame), length);
	env->ReleaseByteArrayElements(javaCameraFrame, cameraFrame, JNI_ABORT);

	return true;
}

///////////////////////////////////////////////////////native cam//////////////////////////////////////

static jboolean StartRealView(JNIEnv *env, jobject, jint sessionId)
{
	if(mpServer && gRealCam) {

		mpServer->setRealView(sessionId, gRealCam);
		return true;
	}
	return false;
}

//seven api
static jboolean StartRealCamCodec(JNIEnv *env, jobject, jobject surface) {
	bool ret = true;
	if(gRealCam==NULL)
		gRealCam = new RealCameraCodec();

	ANativeWindow *pAnw = ANativeWindow_fromSurface(env, surface);

	gRealCam->startPlayer(pAnw, 0, 0);

	return ret;
}

static jboolean StopRealCamCodec(JNIEnv *env, jobject) {
	bool ret = true;
	if(gRealCam) {
		gRealCam->stopPlayer();
		delete gRealCam;
		gRealCam = NULL;
	}
	return ret;
}

static void SetRealCameraParam(JNIEnv *env, jobject, jstring params) {
	gRealCam->SetCameraParameter(params);
}

static jstring GetRealCameraParam(JNIEnv *env, jobject) {
	return gRealCam->GetCameraParameter();
}

static jboolean RealCodecSetInt32(JNIEnv *env, jobject, jstring key, jint value) {
	jboolean isOk  = JNI_FALSE;

	const char *ck = env->GetStringUTFChars(key, &isOk);
	gRealCam->setInt32(ck, value);
	env->ReleaseStringUTFChars(key, ck);

	return true;
}

static jboolean OpenRealCamera(JNIEnv *env, jobject, jint cameraId, jstring clientName) {
	if(gRealCam==NULL)
		gRealCam = new RealCameraCodec();

	gRealCam->openCamera(cameraId, clientName);

	return true;
}

static jboolean CloseRealCamera(JNIEnv *env, jobject) {
	if(gRealCam) {
		gRealCam->closeCamera();
	}

	return true;
}


////////////////////////////////////////////client/////////////////////////////////////////////////////

static jboolean StartFileRecv(JNIEnv *env, jobject, jstring destip, jint destport, jstring remoteFile, jstring saveFile)//ip port remotefile savefile
{
	int ret = 0;
	if(mpClient==NULL) {

		jboolean isOk = JNI_FALSE;
		const char*rfile = env->GetStringUTFChars(remoteFile, &isOk);
		const char*sfile = env->GetStringUTFChars(saveFile, &isOk);
		const char *ip = env->GetStringUTFChars(destip, &isOk);
		mpClient = new TcpClient();
		ret = mpClient->connect( ip, destport, rfile, sfile );

		if(ret < 0) {
			delete mpClient;
			mpClient = NULL;
			return false;
		}

		mpClient->registerEvent(mStatiion.getEventArg());

		env->ReleaseStringUTFChars(remoteFile, rfile);
		env->ReleaseStringUTFChars(saveFile, sfile);
		env->ReleaseStringUTFChars(destip, ip);

		return true;
	}
	return false;
}

static jboolean StopFileRecv(JNIEnv *env, jobject)
{
	if(mpClient!=NULL){
		mpClient->disConnect();
		delete mpClient;
		mpClient = NULL;
	}
	return true;
}

//-----------------------------------------------------------------video client---------------------------------------------------------------------------------

static jboolean StartRealVideoRecv(JNIEnv *env, jobject, jstring destip, jint destport, jobject surface)//ip port remotefile surface
{
	int ret = 0;
	if(mpClient==NULL) {

		jboolean isOk = JNI_FALSE;
		const char *ip 		= env->GetStringUTFChars(destip, &isOk);
		ANativeWindow *pAnw = ANativeWindow_fromSurface(env, surface);
		mpClient = new TcpClient();
		ret = mpClient->connect( ip, destport, pAnw );

		if(ret < 0) {
			delete mpClient;
			mpClient = NULL;
			return false;
		}

		mpClient->registerEvent(mStatiion.getEventArg());
		env->ReleaseStringUTFChars(destip, ip);

		return true;
	}
	return false;
}

static jboolean StartFileVideoRecv(JNIEnv *env, jobject, jstring destip, jint destport, jstring remoteFile, jobject surface)//ip port remotefile surface
{
	int ret = 0;
	if(mpClient==NULL) {

		jboolean isOk = JNI_FALSE;
		const char*filepath = env->GetStringUTFChars(remoteFile, &isOk);
		const char *ip 		= env->GetStringUTFChars(destip, &isOk);
		ANativeWindow *pAnw = ANativeWindow_fromSurface(env, surface);
		mpClient = new TcpClient();
		ret = mpClient->connect( ip, destport, filepath, pAnw );

		if(ret < 0) {
			delete mpClient;
			mpClient = NULL;
			return false;
		}

		mpClient->registerEvent(mStatiion.getEventArg());
		env->ReleaseStringUTFChars(remoteFile, filepath);
		env->ReleaseStringUTFChars(destip, ip);

		return true;
	}
	return false;
}

static jboolean StopVideoRecv(JNIEnv *env, jobject)
{
	if(mpClient!=NULL){
		mpClient->disConnect();
		delete mpClient;
		mpClient = NULL;
	}
	return true;
}

static JNINativeMethod video_method_table[] = {

		{"StartNetWork", "()Z", (void*)StartNetWork },
		{"StopNetWork", "()Z", (void*)StopNetWork },
		{"StartServer", "(Ljava/lang/String;I)Z", (void*)StartServer },
		{"StopServer", "()Z", (void*)StopServer },

		{"SetUpcamEncView", "(I)Z", (void*)SetUpcamEncView },
		{"StartUpcamEncodec", "()Z", (void*)StartUpcamEncodec },
		{"StopUpcamEncodec", "()Z", (void*)StopUpcamEncodec },
		{ "UpcamEncSetInt32", "(Ljava/lang/String;I)Z", (void*)UpcamEncSetInt32 },
		{ "UpcamEncProvide", "([BI)Z", (void *)UpcamEncProvide },

		{"StartRealView", "(I)Z", (void*)StartRealView },
		{ "StartRealCamCodec", "(Landroid/view/Surface;)Z", (void *)StartRealCamCodec },
		{ "StopRealCamCodec", "()Z", (void *)StopRealCamCodec },
		{ "SetRealCameraParam", "(Ljava/lang/String;)V", (void *)SetRealCameraParam },
		{ "GetRealCameraParam", "()Ljava/lang/String;", (void *)GetRealCameraParam },
		{ "RealCodecSetInt32", "(Ljava/lang/String;I)Z", (void*)RealCodecSetInt32 },
		{ "OpenRealCamera", "(ILjava/lang/String;)Z", (void *)OpenRealCamera },
		{ "CloseRealCamera", "()Z", (void *)CloseRealCamera },



		{"StartFileRecv", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)Z", (void*)StartFileRecv },
		{"StopFileRecv", "()Z", (void*)StopFileRecv },
		{"StartRealVideoRecv", "(Ljava/lang/String;ILandroid/view/Surface;)Z", (void*)StartRealVideoRecv },
		{"StartFileVideoRecv", "(Ljava/lang/String;ILjava/lang/String;Landroid/view/Surface;)Z", (void*)StartFileVideoRecv },
		{"StopVideoRecv", "()Z", (void*)StopVideoRecv },
};

int registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* methods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        GLOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        GLOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		GLOGE("GetEnv failed!");
		return result;
	}

	g_javaVM = vm;

	GLOGW("JNI_OnLoad......");
	registerNativeMethods(env,
			REG_PATH, video_method_table,
			NELEM(video_method_table));

	return JNI_VERSION_1_4;
}

