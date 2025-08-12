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


typedef struct {
	char name[50];
	char path[100];
	IMFActivate *ppDevices;
}camera_details;

UINT32 getCameraDetails(camera_details *cameras);

void getFrames(IMFActivate *ppDevice);

void getUVCSettings(IMFActivate *ppDevice);
#endif

