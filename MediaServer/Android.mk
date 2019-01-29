LOCAL_PATH := $(call my-dir)

LOCAL_PROJECT_ROOT := $(LOCAL_PATH)#$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)))


THREAD_PATH  = ../common/gthread
NAREDEC_PATH = ../common/NalBareflow
CODEC_PATH	 = ../Codec
MCNDK_PATH	 = ../Codec/Mcndk


include $(CLEAR_VARS)

APP_ALLOW_MISSING_DEPS=true

LOCAL_CFLAGS := -D__ANDROID__ -DHAVE_CONFIG_H -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
LOCAL_CPPFLAGS 	+= -std=c++11
LOCAL_MODULE := netmedia

LOCAL_C_INCLUDES += \
				   $(LOCAL_PROJECT_ROOT)/net \
				   $(LOCAL_PROJECT_ROOT)/../common \
				   $(LOCAL_PROJECT_ROOT)/$(CODEC_PATH) \
				   $(LOCAL_PROJECT_ROOT)/$(MCNDK_PATH) \
				   $(LOCAL_PROJECT_ROOT)/$(THREAD_PATH) \
				   external/stlport/stlport bionic

LOCAL_SRC_FILES := net/buffer.c \
				net/epoll.c \
				net/epoll_sub.c \
				net/event.c \
				net/evbuffer.c \
				net/signal.c \
				net/log.c \
				net/net_protocol.c \
				$(THREAD_PATH)/gthreadpool.cpp \
				$(NAREDEC_PATH)/NALDecoder.cpp \
				$(CODEC_PATH)/VideoBase.cpp \
				$(CODEC_PATH)/VideoDecoder.cpp \
				$(MCNDK_PATH)/mediacodec_ndk.c \
				$(MCNDK_PATH)/mediaextrator_ndk.c \
				ActorStation.cpp \
				BufferCache.cpp \
				DataUtils.cpp \
				EventCall.cpp \
				IOUtils.cpp \
				Session.cpp \
				TaskBase.cpp \
				TaskFileSend.cpp \
				TaskFileRecv.cpp \
				TaskVideoSend.cpp \
				TaskVideoRecv.cpp \
				TcpClient.cpp \
				TcpServer.cpp \
				NativeApi.cpp

#LOCAL_SHARED_LIBRARIES := avformat avcodec avutil swresample
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)

