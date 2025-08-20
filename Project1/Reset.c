#include "EnumerateCamera.h"
#define MAX_NUMBER_OF_DEVICES 15
#define BUFFER_LENGTH 65
#define STAGE2_RESP_VIDEO_DEVICE 0x32
#define DEVICE_RESET 0x04



HANDLE g_WriteHandle[BUFFER_LENGTH];
HANDLE g_ReadHandle[BUFFER_LENGTH];
int ind;
unsigned char g_InputPacketBuffer[BUFFER_LENGTH];


BOOL ResetDevice(UINT32 *handle) {
	if (handle == NULL || handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	for (ind = 0; ind < MAX_NUMBER_OF_DEVICES; ind++) {
		if (g_WriteHandle[ind] == handle) {
			break;
		}
	}
	if (ind >= MAX_NUMBER_OF_DEVICES) {
		printf("Matching handle not found\n");
	}

	DWORD dwBytesWritten = 0;
	memset(g_InputPacketBuffer, 0x00, BUFFER_LENGTH);
	g_InputPacketBuffer[1] = STAGE2_RESP_VIDEO_DEVICE;
	g_InputPacketBuffer[2] = DEVICE_RESET;

	if (g_WriteHandle[ind] != INVALID_HANDLE_VALUE) {
		if (WriteFile(g_WriteHandle[ind], &g_InputPacketBuffer, BUFFER_LENGTH, &dwBytesWritten, 0) == FALSE) {
			printf("Write file failed\n");
		}
		else {
			printf("Resetting the device\n");
			return TRUE;
		}
	}
	printf("Reset Device Failed\n");
	return FALSE;
}


