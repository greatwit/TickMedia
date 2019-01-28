#ifndef __VideoBase_hpp__
#define __VideoBase_hpp__



#include <stdint.h>

#ifndef OWN_SOCK_EXIT
#define OWN_SOCK_EXIT	-1528
#endif


class VideoBase {

	public:
		VideoBase( );
		virtual ~VideoBase();
		virtual int onDataComing(void*data, int len);


	protected:

};


#endif


