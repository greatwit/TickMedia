APP_STL := gnustl_static
#APP_STL :=  stlport_shared
#APP_CPPFLAGS += -fexceptions
#LOCAL_CFLAGS := -D__ANDROID__ -DHAVE_CONFIG_H -DHAVE_SYS_TIME_H
APP_CPPFLAGS += -frtti
APP_ABI := armeabi
APP_PLATFORM := android-18
TARGET_PLATFORM := android-18