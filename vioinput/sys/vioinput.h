/**********************************************************************
 * Copyright (c) 2010-2015 Red Hat, Inc.
 *
 * File: vioinput.h
 *
 * Author(s):
 *
 * Main include file
 * This file contains various routines and globals
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
**********************************************************************/
#pragma once
#include "public.h"

// If defined, will expose absolute axes as a tablet device only if they
// don't come with mouse buttons.
#define EXPOSE_ABS_AXES_WITH_BUTTONS_AS_MOUSE

EVT_WDF_DRIVER_DEVICE_ADD VIOInputEvtDeviceAdd;

EVT_WDF_INTERRUPT_ISR     VIOInputInterruptIsr;
EVT_WDF_INTERRUPT_DPC     VIOInputQueuesInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE  VIOInputInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE VIOInputInterruptDisable;

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

//
// These are the device attributes returned by the mini driver in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//
#define HIDMINI_PID     0x1052
#define HIDMINI_VID     0x1AF4
#define HIDMINI_VERSION 0x0001

typedef struct _tagDynamicArray
{
    PVOID  Ptr;
    SIZE_T Size;
    SIZE_T MaxSize;
} DYNAMIC_ARRAY, *PDYNAMIC_ARRAY;

typedef struct virtio_input_event
{
    unsigned short type;
    unsigned short code;
    unsigned long value;
} VIRTIO_INPUT_EVENT, *PVIRTIO_INPUT_EVENT;

struct _tagInputDevice;

typedef struct _tagInputClassCommon
{
// the first byte of a HID report is always report ID
#define HID_REPORT_ID_OFFSET 0
#define HID_REPORT_DATA_OFFSET 1

    // this class's HID report
    PUCHAR pHidReport;
    // size of this class's data in the HID report
    SIZE_T cbHidReportSize;
    // report ID of this class
    UCHAR uReportID;
    // the HID report is dirty and should be sent up
    BOOLEAN bDirty;

    NTSTATUS(*EventToReportFunc)(struct _tagInputClassCommon *pClass, PVIRTIO_INPUT_EVENT pEvent);
    NTSTATUS(*ReportToEventFunc)(struct _tagInputClassCommon *pClass, struct _tagInputDevice *pContext,
                                 WDFREQUEST Request, PUCHAR pReport, ULONG cbReport);
    VOID(*CleanupFunc)(struct _tagInputClassCommon *pClass);

} INPUT_CLASS_COMMON, *PINPUT_CLASS_COMMON;

#define MAX_INPUT_CLASS_COUNT 5

typedef struct _tagInputDevice
{
    VIRTIO_WDF_DRIVER      VDevice;

    WDFINTERRUPT           QueuesInterrupt;

    struct virtqueue       *EventQ;
    struct virtqueue       *StatusQ;

    WDFSPINLOCK            EventQLock;
    WDFSPINLOCK            StatusQLock;

    WDFQUEUE               IoctlQueue;
    WDFQUEUE               HidQueue;

    HID_DESCRIPTOR         HidDescriptor;
    HID_DEVICE_ATTRIBUTES  HidDeviceAttributes;
    PHID_REPORT_DESCRIPTOR HidReportDescriptor;

    // array of pointers to input class descriptors, each responsible
    // for one device class (e.g. mouse)
    PINPUT_CLASS_COMMON    InputClasses[MAX_INPUT_CLASS_COUNT];
    ULONG                  uNumOfClasses;
} INPUT_DEVICE, *PINPUT_DEVICE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(INPUT_DEVICE, GetDeviceContext)

#define VIOINPUT_DRIVER_MEMORY_TAG (ULONG)'niIV'

// Virtio-input data structures:

enum virtio_input_config_select
{
    VIRTIO_INPUT_CFG_UNSET = 0x00,
    VIRTIO_INPUT_CFG_ID_NAME = 0x01,
    VIRTIO_INPUT_CFG_ID_SERIAL = 0x02,
    VIRTIO_INPUT_CFG_ID_DEVIDS = 0x03,
    VIRTIO_INPUT_CFG_PROP_BITS = 0x10,
    VIRTIO_INPUT_CFG_EV_BITS = 0x11,
    VIRTIO_INPUT_CFG_ABS_INFO = 0x12,
};

struct virtio_input_absinfo
{
    unsigned long min;
    unsigned long max;
    unsigned long fuzz;
    unsigned long flat;
    unsigned long res;
};

struct virtio_input_devids
{
    unsigned short bustype;
    unsigned short vendor;
    unsigned short product;
    unsigned short version;
};

union virtio_input_raw_cfg_data
{
    char string[128];
    unsigned char bitmap[128];
    struct virtio_input_absinfo abs;
    struct virtio_input_devids ids;
};

typedef struct virtio_input_cfg_data
{
    unsigned char size;
    union virtio_input_raw_cfg_data u;
} VIRTIO_INPUT_CFG_DATA, *PVIRTIO_INPUT_CFG_DATA;

struct virtio_input_config
{
    unsigned char select;
    unsigned char subsel;
    unsigned char size;
    unsigned char reserved[5];
    union virtio_input_raw_cfg_data u;
};

typedef struct virtio_input_event_with_request
{
    VIRTIO_INPUT_EVENT Event;
    WDFREQUEST Request;
} VIRTIO_INPUT_EVENT_WITH_REQUEST, *PVIRTIO_INPUT_EVENT_WITH_REQUEST;

// Event types
#define EV_SYN        0x00
#define EV_KEY        0x01
#define EV_REL        0x02
#define EV_ABS        0x03
#define EV_LED        0x11

// Button codes
#define BTN_MOUSE     0x110
#define BTN_LEFT      0x110
#define BTN_RIGHT     0x111
#define BTN_MIDDLE    0x112
#define BTN_SIDE      0x113
#define BTN_EXTRA     0x114
#define BTN_FORWARD   0x115
#define BTN_BACK      0x116
#define BTN_TASK      0x117

#define BTN_JOYSTICK  0x120
#define BTN_GAMEPAD   0x130

#define BTN_TOUCH     0x14a
#define BTN_STYLUS    0x14b

#define BTN_WHEEL     0x150
#define BTN_GEAR_DOWN 0x150
#define BTN_GEAR_UP   0x151

// Relative axis codes
#define REL_X         0x00
#define REL_Y         0x01
#define REL_Z         0x02
#define REL_RX        0x03
#define REL_RY        0x04
#define REL_RZ        0x05
#define REL_HWHEEL    0x06
#define REL_DIAL      0x07
#define REL_WHEEL     0x08
#define REL_MISC      0x09

// Absolute axis codes
#define ABS_X         0x00
#define ABS_Y         0x01
#define ABS_Z         0x02
#define ABS_RX        0x03
#define ABS_RY        0x04
#define ABS_RZ        0x05
#define ABS_THROTTLE  0x06
#define ABS_RUDDER    0x07
#define ABS_WHEEL     0x08
#define ABS_GAS       0x09
#define ABS_BRAKE     0x0a
#define ABS_PRESSURE  0x18
#define ABS_DISTANCE  0x19
#define ABS_TILT_X    0x1a
#define ABS_TILT_Y    0x1b
#define ABS_MISC      0x28

// LED codes
#define LED_NUML      0x00
#define LED_CAPSL     0x01
#define LED_SCROLLL   0x02
#define LED_COMPOSE   0x03
#define LED_KANA      0x04
#define LED_SLEEP     0x05
#define LED_SUSPEND   0x06
#define LED_MUTE      0x07
#define LED_MISC      0x08
#define LED_MAIL      0x09
#define LED_CHARGING  0x0a

NTSTATUS
VIOInputFillQueue(
    IN struct virtqueue *vq,
    IN WDFSPINLOCK Lock
);

NTSTATUS
VIOInputAddInBuf(
    IN struct virtqueue *vq,
    IN PVIRTIO_INPUT_EVENT buf
);

NTSTATUS
VIOInputAddOutBuf(
    IN struct virtqueue *vq,
    IN PVIRTIO_INPUT_EVENT buf
);

NTSTATUS
VIOInputBuildReportDescriptor(
    PINPUT_DEVICE pContext
);

VOID
EvtIoDeviceControl(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    size_t OutputBufferLength,
    size_t InputBufferLength,
    ULONG IoControlCode
);

VOID
ProcessInputEvent(
    PINPUT_DEVICE pContext,
    PVIRTIO_INPUT_EVENT pEvent
);

NTSTATUS
ProcessOutputReport(
    PINPUT_DEVICE pContext,
    WDFREQUEST Request,
    PHID_XFER_PACKET pPacket
);

NTSTATUS
RequestCopyFromBuffer(
    WDFREQUEST Request,
    PVOID SourceBuffer,
    size_t NumBytesToCopyFrom
);

BOOLEAN
DynamicArrayReserve(
    PDYNAMIC_ARRAY pArray,
    SIZE_T cbSize
);

BOOLEAN
DynamicArrayAppend(
    PDYNAMIC_ARRAY pArray,
    PVOID pData,
    SIZE_T cbLength
);

VOID
DynamicArrayDestroy(
    PDYNAMIC_ARRAY pArray
);

PVOID
DynamicArrayGet(
    PDYNAMIC_ARRAY pArray,
    SIZE_T *pcbSize
);

VOID
HIDAppend1(
    PDYNAMIC_ARRAY pArray,
    UCHAR tag
);

VOID
HIDAppend2(
    PDYNAMIC_ARRAY pArray,
    UCHAR tag,
    LONG value
);

BOOLEAN
DecodeNextBit(
    PUCHAR pBitmap,
    PUCHAR pValue
);

VOID
GetAbsAxisInfo(
    PINPUT_DEVICE pContext,
    ULONG uAbsAxis,
    struct virtio_input_absinfo *pAbsInfo
);

NTSTATUS
RegisterClass(
    PINPUT_DEVICE pContext,
    PINPUT_CLASS_COMMON pClass
);

NTSTATUS
HIDMouseProbe(
    PINPUT_DEVICE pContext,
    PDYNAMIC_ARRAY pHidDesc,
    PVIRTIO_INPUT_CFG_DATA pRelAxes,
    PVIRTIO_INPUT_CFG_DATA pAbsAxes,
    PVIRTIO_INPUT_CFG_DATA pButtons
);

NTSTATUS
HIDKeyboardProbe(
    PINPUT_DEVICE pContext,
    PDYNAMIC_ARRAY pHidDesc,
    PVIRTIO_INPUT_CFG_DATA pKeys,
    PVIRTIO_INPUT_CFG_DATA pLeds
);

NTSTATUS
HIDConsumerProbe(
    PINPUT_DEVICE pContext,
    PDYNAMIC_ARRAY pHidDesc,
    PVIRTIO_INPUT_CFG_DATA pKeys
);

NTSTATUS
HIDTabletProbe(
    PINPUT_DEVICE pContext,
    PDYNAMIC_ARRAY pHidDesc,
    PVIRTIO_INPUT_CFG_DATA pAxes,
    PVIRTIO_INPUT_CFG_DATA pButtons
);

NTSTATUS
HIDJoystickProbe(
    PINPUT_DEVICE pContext,
    PDYNAMIC_ARRAY pHidDesc,
    PVIRTIO_INPUT_CFG_DATA pAxes,
    PVIRTIO_INPUT_CFG_DATA pButtons
);

static inline PVOID VIOInputAlloc(SIZE_T cbNumberOfBytes)
{
    PVOID pPtr = ExAllocatePoolWithTag(
        NonPagedPool,
        cbNumberOfBytes,
        VIOINPUT_DRIVER_MEMORY_TAG);
    if (pPtr)
    {
        RtlZeroMemory(pPtr, cbNumberOfBytes);
    }
    return pPtr;
}

static inline VOID VIOInputFree(PVOID *pPtr)
{
    if (*pPtr)
    {
        ExFreePoolWithTag(*pPtr, VIOINPUT_DRIVER_MEMORY_TAG);
        *pPtr = NULL;
    }
}
