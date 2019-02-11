
#include <unistd.h>
#include <android/native_window_jni.h>
#include "basedef.h"
#include "RealCameraCodec.h"

RealCameraCodec::RealCameraCodec()
		:mFormat(NULL)
		,mCodec(NULL)
		,mCodecCall(NULL)
		,mVideoWidht(0)
		,mVideoHeight(0)
		,mUvlen(0)
		,mYlen(0)
{
	mCamera = CameraContext::getInstance();

	InitExtratorSymbols(&mSymbols);
	mFormat = mSymbols.AMediaFormat.newfmt();
}

RealCameraCodec::~RealCameraCodec() {

	if(mFormat) {
		mSymbols.AMediaFormat.deletefmt(mFormat);
		mFormat = NULL;
	}

	ReleaseExtratorSymbols(&mSymbols);
}

bool RealCameraCodec::openCamera( int cameraId, jstring clientName) {
	bool ret = true;
	int camSet = mCamera->CameraSetup(this, cameraId, clientName);
	return camSet>=0;
}

bool RealCameraCodec::closeCamera() {
	bool ret = true;
	if(mCamera) {
		mCamera->ReleaseLib();
		delete mCamera;
		mCamera = NULL;
	}
	return ret;
}

void RealCameraCodec::SetCameraParameter(jstring params) {
	mCamera->SetCameraParameter(params);
}

jstring RealCameraCodec::GetCameraParameter() {
	return mCamera->GetCameraParameter();
}

int RealCameraCodec::startPlayer(void *surface, int w, int h) {
	int rest = 1;

	mCodec    = mSymbols.AMediaCodec.createEncoderByType("video/avc");
	if(!mCodec)
		GLOGE("AMediaCodec.createEncoderByType for %s successful", "video/avc");


		mSymbols.AMediaFormat.getInt32(mFormat, "width", &mVideoWidht);
		mSymbols.AMediaFormat.getInt32(mFormat, "height", &mVideoHeight);
		GLOGW("mVideoWidht:%d mVideoHeight:%d\n", mVideoWidht, mVideoHeight);
		mUvlen	= mVideoWidht*mVideoHeight/2;
		mYlen	= mVideoWidht*mVideoHeight;

		mSymbols.AMediaFormat.setString(mFormat, "mime", "video/avc");
		GLOGW("format string:%s\n", mSymbols.AMediaFormat.toString(mFormat));

		if (mSymbols.AMediaCodec.configure(mCodec, mFormat, NULL, NULL, 1) != AMEDIA_OK)	//flags
		{
			GLOGE("AMediaCodec.configure failed");
		}else
			GLOGW("AMediaCodec.configure successful.");


	    if (mSymbols.AMediaCodec.start(mCodec) != AMEDIA_OK)
	        GLOGE("AMediaCodec.start failed");

	    mCamera->StartPreview((ANativeWindow*)surface);

	return rest;
}

int RealCameraCodec::stopPlayer() {
	int rest = -1;

	if(mCodec) {
		mSymbols.AMediaCodec.stop(mCodec);
		mSymbols.AMediaCodec.deletemc(mCodec);
	}

	return rest;
}

void RealCameraCodec::setInt32(const char*key, int value) {
	 mSymbols.AMediaFormat.setInt32(mFormat, key, value);
}

void RealCameraCodec::setCodecCall(IVideoCallback*call) {
	mCodecCall = call;
}

void RealCameraCodec::VideoSource(VideoFrame *pBuf) {
	media_status_t status;

	int i = 0, uv = 0;
	uint8_t* yuv = (uint8_t*)pBuf->addrVirY;
	while(i<mUvlen) {
		uv = mYlen+i;
		uint8_t tmp = yuv[uv];
		yuv[uv] = yuv[uv+1];
		yuv[uv+1] = tmp;
		i+=2;
	}


	GLOGW("AMediaCodec.dequeueInputBuffer begin buf len:%d ", pBuf->length);
	int index = mSymbols.AMediaCodec.dequeueInputBuffer(mCodec, 10000);					//int64_t timeoutUs
	GLOGW("AMediaCodec.dequeueInputBuffer value:%d", index);
	if(index>=0) {
		uint8_t *p_mc_buf;
		size_t i_mc_size;
		p_mc_buf = mSymbols.AMediaCodec.getInputBuffer(mCodec, index, &i_mc_size);		//size_t idx, size_t *out_size
		GLOGW("AMediaCodec.getInputBuffer mcSize:%d", i_mc_size);
		memcpy(p_mc_buf, (uint8_t*)pBuf->addrVirY, pBuf->length);
		status = mSymbols.AMediaCodec.queueInputBuffer(mCodec, index, 0, pBuf->length,
				50000, 0);
		GLOGW("mSymbols.AMediaCodec.queueInputBuffer status:%d", status);
	} else {
		usleep(10*1000);
	}

	//usleep(1*1000);
	AMediaCodecBufferInfo info;
	ssize_t out_index = mSymbols.AMediaCodec.dequeueOutputBuffer(mCodec, &info, 10000);	//AMediaCodecBufferInfo *info, int64_t timeoutUs
	GLOGW("AMediaCodec.dequeueOutputBuffer out_index:%d size:%d", out_index, info.size);
	if((out_index>=0)&&info.size>0) {
		size_t out_size = 0;
		uint8_t *data = mSymbols.AMediaCodec.getOutputBuffer(mCodec, out_index, &out_size);
		//fwrite(data, 1, info.size, mpFile);
		if(mCodecCall) {
			VideoFrame buff;
			buff.addrVirY = (int)data;
			buff.length   = info.size;
			mCodecCall->VideoSource(&buff);
		}
	}
	//usleep(2*1000);

	status = mSymbols.AMediaCodec.releaseOutputBuffer(mCodec, out_index, true);
	GLOGW("AMediaCodec.releaseOutputBuffer status:%d", status);
}


