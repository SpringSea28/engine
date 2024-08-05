LOCAL_PATH:= $(call my-dir)

# Link with shared libraries
# =====================================================

common_SRC_FILES := \
        SkiaBarcode.cpp \
        Codabar.cpp \
        Code128.cpp \
        Code25.cpp \
        Code39.cpp \
        Code93.cpp \
        DataMatrix.cpp \
        Ean.cpp \
        GridMatrix.cpp \
        MaxiCode.cpp \
        Pdf417.cpp \
        QRCode.cpp \
        UPC.cpp

common_C_INCLUDES := $(LOCAL_PATH)
common_LIBRARIES := libzint

# For the device (shared)
# =====================================================

## 0. Clear local variables
include $(CLEAR_VARS)

## 1. Add source files and import/export includes
LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_C_INCLUDES := $(common_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

## 2. Add compile flags and link flags
LOCAL_CFLAGS := $(common_CFLAGS)
LOCAL_STATIC_LIBRARIES := $(common_LIBRARIES)
LOCAL_SHARED_LIBRARIES := libskia

## 3. Define other module parameters
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libskiabarcode

include $(BUILD_SHARED_LIBRARY)

