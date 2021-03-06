
#include <unistd.h>
#include "basedef.h"
#include "VideoDecoder.hpp"

VideoDecoder::VideoDecoder( void*surface )
	:mFormat(NULL)
	,mCodec(NULL)
	,mbSpsDone(true)
	,mbPpsDone(true)
	,mSurface(surface)
	,mCount(1)
{

	InitExtratorSymbols(&mSymbols);
	mFormat = mSymbols.AMediaFormat.newfmt();

	mCodec    = mSymbols.AMediaCodec.createDecoderByType("video/avc");
	if(!mCodec)
		GLOGE("AMediaCodec.createDecoderByType for %s successful", "video/avc");

	mSymbols.AMediaFormat.setString(mFormat, "mime", "video/avc");
	GLOGW("format string:%s\n", mSymbols.AMediaFormat.toString(mFormat));

	mSymbols.AMediaFormat.setInt32(mFormat, "width", 1280);
	mSymbols.AMediaFormat.setInt32(mFormat, "height", 720);
//	mSymbols.AMediaFormat.setInt32(mFormat, "durationUs", 12498744);
//	mSymbols.AMediaFormat.setInt32(mFormat, "frame-rate", 20);
//	mSymbols.AMediaFormat.setInt32(mFormat, "max-input-size", 38981);
}

VideoDecoder::~VideoDecoder() {
	mSymbols.AMediaFormat.deletefmt(mFormat);
	ReleaseExtratorSymbols(&mSymbols);
}

int VideoDecoder::onDataComing(void*buff, int size) {
	int ret=0;
	char*data = (char*)buff;

	switch(data[4]) {
		case 0x67:
			ret = 4;
			for(;ret<size;ret++) {
				if(data[ret]==0x68) {
					if((data[ret-1]==0x01) && (data[ret-2]==0x00) && (data[ret-3]==0x00)) {
						int ppslen = ret-4;
						int spslen = size - ppslen;
						if(mbPpsDone) {
							GLOGE("mSymbols.AMediaFormat.setBuffer 1");
							mSymbols.AMediaFormat.setBuffer(mFormat, "csd-0", data, ppslen);
							GLOGE("mSymbols.AMediaFormat.setBuffer 0");
							mbPpsDone = false;
						}
						if(mbSpsDone) {
							mSymbols.AMediaFormat.setBuffer(mFormat, "csd-1", data+ppslen, spslen);

							GLOGW("format string:%s\n", mSymbols.AMediaFormat.toString(mFormat));

							if (mSymbols.AMediaCodec.configure(mCodec, mFormat, (ANativeWindow*)mSurface, NULL, 0) != AMEDIA_OK)
							{
								GLOGE("AMediaCodec.configure failed");
							}else
								GLOGW("AMediaCodec.configure successful.");

							if (mSymbols.AMediaCodec.start(mCodec) != AMEDIA_OK)
								GLOGE("AMediaCodec.start failed");

							mbSpsDone = false;
						}
						return ret;
					}//if entry
				}
			}

			if(mbPpsDone) {
				GLOGE("mSymbols.AMediaFormat.setBuffer 1");
				mSymbols.AMediaFormat.setBuffer(mFormat, "csd-0", data, size);
				GLOGE("mSymbols.AMediaFormat.setBuffer 2");
				mbPpsDone = false;
			}
			return ret;

		case 0x68:
			if(mbSpsDone) {
				mSymbols.AMediaFormat.setBuffer(mFormat, "csd-1", data, size);

				GLOGW("format string:%s\n", mSymbols.AMediaFormat.toString(mFormat));

				if (mSymbols.AMediaCodec.configure(mCodec, mFormat, (ANativeWindow*)mSurface, NULL, 0) != AMEDIA_OK)
				{
					GLOGE("AMediaCodec.configure failed");
				}else
					GLOGW("AMediaCodec.configure successful.");

				if (mSymbols.AMediaCodec.start(mCodec) != AMEDIA_OK)
					GLOGE("AMediaCodec.start failed");

				mbSpsDone = false;
			}
			return ret;

		default:
			media_status_t status;
			GLOGW("AMediaCodec.dequeueInputBuffer begin.");
			int index = mSymbols.AMediaCodec.dequeueInputBuffer(mCodec, 10000);//int64_t timeoutUs
			GLOGW("AMediaCodec.dequeueInputBuffer value:%d", index);
			if(index>=0) {
				uint8_t *p_mc_buf;
				size_t i_mc_size;
				p_mc_buf = mSymbols.AMediaCodec.getInputBuffer(mCodec, index, &i_mc_size);//size_t idx, size_t *out_size
				GLOGW("AMediaCodec.getInputBuffer mcSize:%d", i_mc_size);
				memcpy(p_mc_buf, data, size);
				status = mSymbols.AMediaCodec.queueInputBuffer(mCodec, index, 0, size,
						mCount*40000, 0);//25frame
				GLOGW("mSymbols.AMediaCodec.queueInputBuffer status:%d", status);
			} else {
				usleep(20*1000);
				return -1;
			}

			//usleep(1*1000);
			AMediaCodecBufferInfo info;
			ssize_t out_index = mSymbols.AMediaCodec.dequeueOutputBuffer(mCodec, &info, 10000);//AMediaCodecBufferInfo *info, int64_t timeoutUs
			GLOGW("AMediaCodec.dequeueOutputBuffer out_index:%d", out_index);
			//usleep(1*1000);
			status = mSymbols.AMediaCodec.releaseOutputBuffer(mCodec, out_index, true);
			GLOGW("AMediaCodec.releaseOutputBuffer status:%d", status);

			mCount++;
		break;
	}

	return 0;
}
