#ifndef _RealCameraCodec_H_
#define _RealCameraCodec_H_

#include <jni.h>

#include "IVideoCallback.h"

#include "mediaextrator.h"
#include "CameraContext.h"


class RealCameraCodec : public IVideoCallback
{
	public:
		RealCameraCodec();
		virtual ~RealCameraCodec();
		bool openCamera( int cameraId, jstring clientName);
		bool closeCamera();
		void SetCameraParameter(jstring params);
		jstring GetCameraParameter();
		int startPlayer(void *surface, int w, int h);
		int stopPlayer();
		void setInt32(const char*key, int value);

		void VideoSource(VideoFrame *pBuf);
		void setCodecCall(IVideoCallback*call);

	private:
		CameraContext*		mCamera;
		struct symext 		mSymbols;
		AMediaCodec*	 	mCodec;
	    AMediaFormat* 		mFormat;

	    IVideoCallback*		mCodecCall;

	    int 				mVideoWidht;
	    int 				mVideoHeight;
	    int					mUvlen;
	    int					mYlen;
};

#endif
