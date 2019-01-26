CXXFLAGS += -std=c++11

CC 	= gcc
CPP = g++
CFLAGS 		= -Wall -DHAVE_CONFIG_H -g -fPIC #-DHAVE_SYS_TIME_H
CXXFLAGS 	+= -Dlinux -DHAV_FFMPEG -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE

THREAD_PATH  = ../common/gthread
NAREDEC_PATH = ../common/NalBareflow

COMPLIE_INCL = -I./net -I../common -I$(THREAD_PATH) -I$(FFMPEG_PATH) -I$(FFMPEG_PATH)/include
#COMPLIE_LIB  = -L$(FFMPEG_PATH)/lib -lavformat -lavcodec -lavdevice -lavfilter -lswresample -lswscale -lavutil
CFLAGS   += -I./ -I./net -I../common
CXXFLAGS += $(COMPLIE_INCL)
LDFLAGS  += $(COMPLIE_LIB) -lpthread

TARGET = EventServer EventClient

#--------------------------------------------------------------------

all: $(TARGET)

OBJECTS = net/buffer.o net/epoll.o net/epoll_sub.o net/event.o net/evbuffer.o net/signal.o net/log.o net/net_protocol.o \
		  BufferCache.o DataUtils.o IOUtils.o Session.o EventCall.o  ActorStation.o \
		  TaskBase.o TaskVideoRecv.o TaskVideoSend.o TaskFileSend.o TaskFileRecv.o \
		  $(THREAD_PATH)/gthreadpool.o $(NAREDEC_PATH)/NALDecoder.o

EventServer=$(OBJECTS) EventServer.o TcpServer.o
EventClient=$(OBJECTS) EventClient.o TcpClient.o
 
EventServer:$(EventServer) 
	 $(CPP) -o EventServer $(EventServer) $(LDFLAGS)

EventClient:$(EventClient) 
	 $(CPP) -o EventClient $(EventClient) $(LDFLAGS)

.PHONY:clean

clean:
	rm *.o net/*.o $(NAREDEC_PATH)/*.o $(THREAD_PATH)/*.o

	