#ifndef __VideoDecoder_hpp__
#define __VideoDecoder_hpp__


#include <stdio.h>

#include "h264.h"
#include "mediaextrator.h"
#include "VideoBase.hpp"


class VideoDecoder : public VideoBase {
	public:
		VideoDecoder( void*surface );
		virtual ~VideoDecoder();
		int onDataComing(void*data, int len);

	private:
		struct symext 		mSymbols;
		AMediaCodec*	 	mCodec;
	    AMediaFormat* 		mFormat;
	    void*				mSurface;
	    bool 				mbSpsDone;
	    bool 				mbPpsDone;
	    int 				mCount;
};


#endif
