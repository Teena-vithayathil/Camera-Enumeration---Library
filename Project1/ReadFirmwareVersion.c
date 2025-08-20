#include "EnumerateCamera.h"
#define BUFFER_LENGTH 65
#define READFIRMWAREVERSION	0x40
#define MAX_NUMBER_OF_DEVICES 15
#define TIMEOUT 5000
#define ERROR_UVC_COMM_FAILED 0X20000005	
#define ERROR_INVALID_EXCEPTION 0X20000011 

HANDLE g_WriteHandle[BUFFER_LENGTH];
HANDLE g_ReadHandle[BUFFER_LENGTH];
CRITICAL_SECTION g_critsec;
char g_tszUniqueID[MAX_PATH];
unsigned char g_OutputPacketBuffer[BUFFER_LENGTH];	
unsigned char g_InputPacketBuffer[BUFFER_LENGTH];		
unsigned char g_OutputPacketBufferPreview[BUFFER_LENGTH];
unsigned char InputPacketBufferPreview[BUFFER_LENGTH];
int g_iCurrInstanceIndex = 0;
DWORD g_BytesWritten = 0;
DWORD g_BytesRead = 0;
OVERLAPPED g_lpOverlapped;
DWORD g_dwWait;

BOOL DataReadWrite(UINT32 handle) {
	if (!g_critsec.DebugInfo) {
		return FALSE;
	};
	EnterCriticalSection(&g_critsec);
	for (g_iCurrInstanceIndex = 0; g_iCurrInstanceIndex < MAX_NUMBER_OF_DEVICES; g_iCurrInstanceIndex++) {
		if ((HANDLE)handle == g_WriteHandle[g_iCurrInstanceIndex]) {
			break;
		}
	}
	if (g_iCurrInstanceIndex >= MAX_NUMBER_OF_DEVICES) {
		printf("Matched handle for input handle not found\n");
		return FALSE;
	}
	if (g_WriteHandle[g_iCurrInstanceIndex] != INVALID_HANDLE_VALUE) {
		WriteFile(g_WriteHandle[g_iCurrInstanceIndex], &g_InputPacketBuffer, BUFFER_LENGTH, &g_BytesWritten, &g_lpOverlapped);
		WaitForSingleObject(g_lpOverlapped.hEvent, TIMEOUT);
		if (!GetOverlappedResult(g_WriteHandle[g_iCurrInstanceIndex], &g_lpOverlapped, &g_BytesWritten, TRUE)) {
			SetLastError(ERROR_UVC_COMM_FAILED);
			OutputDebugStringW(L"Write File Faliled\n");
		}
		else {
			memset(g_OutputPacketBuffer, 0x00, BUFFER_LENGTH);
			ReadFile(g_ReadHandle[g_iCurrInstanceIndex], &g_OutputPacketBuffer, BUFFER_LENGTH, &g_BytesRead, &g_lpOverlapped);
			g_dwWait = WaitForSingleObject(g_lpOverlapped.hEvent, TIMEOUT);
			if (g_dwWait == WAIT_OBJECT_0) {
				if (GetOverlappedResult(g_ReadHandle[g_iCurrInstanceIndex], &g_lpOverlapped, &g_BytesRead, TRUE)) {
					if (g_BytesRead > 0) {
						LeaveCriticalSection(&g_critsec);
						return TRUE;
					}
				}
			}
			else if (g_dwWait == WAIT_TIMEOUT) {
				printf("DataReadWrite: WaitForSingleObject WAIT_TIMEOUT\n");
			}
			else {
				printf("DataReadWrite:CancelIo***\n");
				if (CancelIo(g_ReadHandle[g_iCurrInstanceIndex])) {
					printf("DataReadWrite: CancelIo GetLastError() = %d\n", GetLastError());
				}
			}
		}
	}
	else {
		SetLastError(ERROR_INVALID_EXCEPTION);
	}
	LeaveCriticalSection(&g_critsec);
	return FALSE;
}

BOOL ReadFirmwareVersion(UINT32 *handle) {
	UINT8 major_version, minor_version_1;
	UINT16 minor_version_2, minor_version_3;
	memset(g_InputPacketBuffer, 0x00, BUFFER_LENGTH);
	g_InputPacketBuffer[1] = READFIRMWAREVERSION;
	if (DataReadWrite(handle)) {
		major_version = g_OutputPacketBuffer[2];
		minor_version_1 = g_OutputPacketBuffer[3];
		minor_version_2 = (g_OutputPacketBuffer[4] << 8) + g_OutputPacketBuffer[5];
		minor_version_3 = (g_OutputPacketBuffer[6] << 8) + g_OutputPacketBuffer[7];;
		printf("Firmware Version: %d.%d.%d.%d \n", major_version, minor_version_1, minor_version_2, minor_version_3);
		printf("Read Firmware Successfull\n");
		return TRUE;
	}
	else
	{
		printf("Failed to read firmware version\n");
		return FALSE;
	}
}
