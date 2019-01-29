
#include "basedef.h"
#include "VideoDecoder.hpp"

VideoDecoder::VideoDecoder( void*surface ) {

	if(MC_API_ERROR == MediaCodecNdk_Init(&mApi))
		GLOGE("MediaCodecNdk_Init error.");

	mApi.configure(&mApi, 0);

	mApi.set_output_surface(&mApi, surface, 0);

	union mc_api_args args;
	args.video.p_surface = surface;
	args.video.i_width 	= 1280;
	args.video.i_height = 720;
	args.video.i_angle 	= 0;
	int err = mApi.start(&mApi, &args);
	GLOGE("function %s,line:%d mApi.start res:%d",__FUNCTION__,__LINE__,err);

}

VideoDecoder::~VideoDecoder() {
	mApi.stop(&mApi);
}

int VideoDecoder::onDataComing(void*data, int len) {
	int err=0;
	long timeoutUs = 10000;
	int index = mApi.dequeue_in(&mApi, timeoutUs);
	err = mApi.queue_in(&mApi, index, data, len, 10000, false);
	GLOGW("queue_in err:%d", err);

	index = mApi.dequeue_out(&mApi, 12000);

	mc_api_out mcout;
	err = mApi.get_out(&mApi, index, &mcout);
	GLOGI("renderBuffer---------rest:%d", err);

	return 0;
}
