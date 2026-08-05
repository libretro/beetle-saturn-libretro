LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/..

DEBUG                    := 0
NEED_CD                  := 1
NEED_BPP                 := 32
NEED_DEINTERLACER        := 1
NEED_THREADING           := 1
NEED_TREMOR              := 1
HAVE_CHD                 := 1
FLAGS                    :=

include $(CORE_DIR)/Makefile.common

COREFLAGS := -funroll-loops $(INCFLAGS) -D__LIBRETRO__ -D_LOW_ACCURACY_ -DINLINE="inline" $(FLAGS)

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
# ndk-build derives LOCAL_OBJECTS from LOCAL_SRC_FILES verbatim, so a source
# listed twice in Makefile.common becomes the same .o twice on the link line
# and lld rejects it as a duplicate symbol. The GNU Makefile links with $^ and
# never sees such a slip, so guard it here.
LOCAL_SRC_FILES    := $(sort $(SOURCES_CXX) $(SOURCES_C))
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_CXXFLAGS     := $(COREFLAGS) -std=c++11
LOCAL_LDFLAGS      := -Wl,-version-script=$(CORE_DIR)/link.T
LOCAL_CPP_FEATURES := exceptions

# armv5 clang workarounds
ifeq ($(TARGET_ARCH_ABI),armeabi)
  LOCAL_LDLIBS   := -latomic
endif

include $(BUILD_SHARED_LIBRARY)
