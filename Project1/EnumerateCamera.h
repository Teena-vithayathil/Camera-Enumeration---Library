#ifndef ENUMERATE_H
#define ENUMERATE_H

#include <Windows.h>
#include <winnt.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mfplay.h>
#include <mftransform.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <mfapi.h>
#include <wincodec.h>
#include <dshow.h>
#include <initguid.h>
#include <combaseapi.h>
#include <strmif.h>
#include<string.h>
#include <devpropdef.h>
#include <cfgmgr32.h>
#include <SetupAPI.h>

HRESULT hr;

typedef struct {
	char name[50];
	char path[260];
	IMFActivate *ppDevices;
}camera_details;

typedef BOOL(WINAPI *FN_SetupDiGetDevicePropertyW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	const DEVPROPKEY *PropertyKey,
	DEVPROPTYPE *PropertyType,
	PBYTE PropertyBuffer,
	DWORD PropertyBufferSize,
	PDWORD RequiredSize,
	DWORD Flags
	);

BOOL ReadFirmwareVersion(UINT32 *handle);

UINT32 getCameraDetails(camera_details *cameras);

void getFrames(IMFActivate *ppDevice);

void getUVCSettings(IMFActivate *ppDevice);

void setUVCSettings(IMFActivate *pDevice, char type[50]);

BOOL InitExtensionUnit(char USBInstanceID[MAX_PATH], UINT32 **handle);

BOOL DeInitExtension(UINT32 *handle);

BOOL ResetDevice(UINT32 *handle);

#endif

