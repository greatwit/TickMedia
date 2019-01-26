
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdarg.h>

#include "h264.h"
#include "basedef.h"

#define	FILE_PATH	"bare720p.h264"

void threadFunc( void *arg ) {
	long seconds = (long) arg;
}

int main( int argc, char ** argv )
{
	if ((argc != 2)) {
		printf("Usage: exe bareflow_filename.\n");
		return 0;
	}

	FILE			*mwFile;
	NALU_t 			*mNALU;

	mwFile = OpenBitstreamFile( argv[1] ); //camera_640x480.h264 //camera_1280x720.h264
	if(mwFile==NULL) {
		GLOGE("open file:%s failed.", argv[1]);
		return 0;
	}

	mNALU  = AllocNALU(8000000);
	int count = 0;
	do{
		count++;
		int size=GetAnnexbNALU(mwFile, mNALU);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		GLOGE("GetAnnexbNALU type:0x%02X size:%d count:%d\n", mNALU->buf[4], size, count);
		if(size<4) {
			GLOGE("get nul error!\n");
			continue;
		}
	}while(!feof(mwFile));

	if(mwFile != NULL)
		fclose(mwFile);

	FreeNALU(mNALU);

	return 0;
}



