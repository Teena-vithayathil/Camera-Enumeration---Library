#include "EnumerateCamera.h"
#define VID "2560"
#define PID  "0121"
#define DEBUG_ENABLED 0x01
#define MAX_NUMBER_OF_DEVICES 15

#define INITGUID

#ifdef DEFINE_DEVPROPKEY
#undef DEFINE_DEVPROPKEY
#endif
#ifdef INITGUID
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY DECLSPEC_SELECTANY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
#else
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY name
#endif // INITGUID
DEFINE_DEVPROPKEY(DEVPKEY_Device_Parent, 0x4340a6c5, 0x93fa, 0x4706, 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7, 8);     // DEVPROP_TYPE_GUID

HDEVINFO g_DeviceInfoTable = INVALID_HANDLE_VALUE;
GUID g_InterfaceClassGuid = { 0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 };
PSP_DEVICE_INTERFACE_DATA g_InterfaceDataStructue = NULL; 
SP_DEVINFO_DATA g_DevInfoData;
PBYTE g_PropertyValueBuffer;
PSP_DEVICE_INTERFACE_DETAIL_DATA g_DetailedInterfaceDataStructure;
FN_SetupDiGetDevicePropertyW fn_SetupDiGetDevicePropertyW;
HANDLE g_WriteHandle[MAX_NUMBER_OF_DEVICES];
int g_CurrInstanceIndex = 0, g_ExtensionUnitCount = 0;
HANDLE g_ReadHandle[MAX_NUMBER_OF_DEVICES];
OVERLAPPED g_lpOverlapped;
HANDLE grabFrameHandle;
HANDLE otherHIDHandle;
char g_extrctd_InstanceID[15], g_USBInstanceID[15], g_HIDInstanceID[15], *g_InstanceID_substr;
CRITICAL_SECTION g_critsec;
DWORD g_InterfaceIndex = 0, g_ErrorStatus, g_dwRegType, g_dwRegSize, g_StructureSize;
char g_DeviceIDFromRegistry[260], g_buff[260];
HANDLE g_ReadTrgiggerStatusHandle = INVALID_HANDLE_VALUE;

WCHAR wszDeviceInstanceID[MAX_PATH];

BOOL FindMatchDevice(char *DeviceID) {
	char *vid_substr, *pid_substr, vid_extrctd[5], pid_extrctd[5];
	vid_substr = strstr(DeviceID, "VID_");
	if (vid_substr != NULL) {
		strncpy(vid_extrctd, vid_substr + 4, 4);
		vid_extrctd[4] = '\0';
	}
	printf("VID: %s\n",vid_extrctd);
	pid_substr = strstr(DeviceID, L"PID_");
	if (pid_substr != NULL) {
		strncpy(pid_extrctd, pid_substr + 4, 4);
		pid_extrctd[4] = '\0';
	}
	printf("PID: %s\n",pid_extrctd);
	if (strcmp(vid_extrctd, VID) == 0 && strcmp(pid_extrctd, PID) == 0) {
		return TRUE;
		OutputDebugStringW(L"FindMatchDevice Successfull....\n\r");
	}
	else {
		return FALSE;
		OutputDebugStringW(L"FindMatchDevice Unsuccessfull....\n\r");
	}
}



BOOL InitExtensionUnit(char USBInstanceID[MAX_PATH], UINT32 **handle) {
	OutputDebugStringW(L"InitExtensionUnit....1\n\r");

	if (USBInstanceID == NULL) {
		return 0;
	}
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	InitializeCriticalSection(&g_critsec);
	g_InterfaceIndex = 0;
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	DEVPROPTYPE ulPropertyType;
	DEVINST devInstParent;
	CONFIGRET status;
	char szDeviceInstanceID[MAX_PATH];
	

	//memset(g_HIDInstanceID, 0x00, sizeof(g_HIDInstanceID));
	//memset(g_USBInstanceID, 0X00, sizeof(g_USBInstanceID));
	//memset(szDeviceInstanceID, 0x00, MAX_PATH);
	
	g_InstanceID_substr = strstr(strupr(USBInstanceID), "MI_");
	if (g_InstanceID_substr != NULL) {
		strncpy(g_extrctd_InstanceID, g_InstanceID_substr + 6, 10);
		g_extrctd_InstanceID[11] = '\0';
		OutputDebugStringW(L"ImagingDevice USB InstanceID");
		strcpy(g_USBInstanceID, g_extrctd_InstanceID);
		printf("USB Instance ID: %s\n", g_USBInstanceID);
	}
	
	g_InterfaceDataStructue = (PSP_DEVICE_INTERFACE_DATA)malloc(sizeof(SP_DEVICE_INTERFACE_DATA));
	//Populate list of plugged in devices
	g_DeviceInfoTable = SetupDiGetClassDevs(&g_InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); //Returns a handle to all the devices
	while (TRUE) {
		g_InterfaceDataStructue->cbSize = sizeof(SP_DEVICE_INTERFACE_DATA); //Setting up size of SP_DEVICE_INTERFACE_DATA
		//Enumerate through devcies at specified g_InterfaceIndex
		if (SetupDiEnumDeviceInterfaces(g_DeviceInfoTable, NULL, &g_InterfaceClassGuid, g_InterfaceIndex, g_InterfaceDataStructue)) {
			g_ErrorStatus = GetLastError();
			if (ERROR_NO_MORE_ITEMS == g_ErrorStatus) {
				SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
				return NULL;
			}
		}
		else
		{
			g_ErrorStatus = GetLastError();
			SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
			return NULL;
		}

		g_DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SetupDiEnumDeviceInfo(g_DeviceInfoTable, g_InterfaceIndex, &g_DevInfoData);
		//Get size of hardware ID
		SetupDiGetDeviceRegistryProperty(g_DeviceInfoTable, &g_DevInfoData, SPDRP_HARDWAREID, &g_dwRegType, NULL, 0, &g_dwRegSize);
		//Allocate space for hardware ID
		g_PropertyValueBuffer = (BYTE *)malloc(g_dwRegSize);
		if (g_PropertyValueBuffer == NULL) {
			SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
			return NULL;
		}

		//Get hardware ID
		SetupDiGetDeviceRegistryProperty(g_DeviceInfoTable, &g_DevInfoData, SPDRP_HARDWAREID, &g_dwRegType, g_PropertyValueBuffer, g_dwRegSize, NULL);
		wchar_t *wideStr = (wchar_t *)g_PropertyValueBuffer;
		WideCharToMultiByte(CP_ACP, 0, wideStr, -1, g_buff, MAX_PATH, NULL, NULL);
		printf("First Hardware ID: %s\n", g_buff);
		/*strcpy_s(g_DeviceIDFromRegistry, sizeof(g_DeviceIDFromRegistry), (char *)g_PropertyValueBuffer);
		sprintf_s(g_buff, MAX_PATH, "DLL VID & PID = %s \r\n", g_DeviceIDFromRegistry);
		printf("%s", g_buff);*/

		free(g_PropertyValueBuffer);
		if (FindMatchDevice(g_buff)) {
			printf("Found Matching Device\n");
			//Open read and write handles
			SetupDiGetDeviceInterfaceDetail(g_DeviceInfoTable, g_InterfaceDataStructue, NULL, NULL, &g_StructureSize, NULL);
			g_DetailedInterfaceDataStructure = (PSP_DEVICE_INTERFACE_DETAIL_DATA)(malloc(g_StructureSize));
			g_DetailedInterfaceDataStructure->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (g_DetailedInterfaceDataStructure == NULL) {
				SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
				return NULL;
			}
			g_DetailedInterfaceDataStructure->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			SetupDiGetDeviceInterfaceDetail(g_DeviceInfoTable, g_InterfaceDataStructue, g_DetailedInterfaceDataStructure, g_StructureSize, NULL, NULL);

			//checks whether its windows xp then get parent instance handle-> path(device id)
			if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion) == 1) {
				status = CM_Get_Parent(&devInstParent, g_DevInfoData.DevInst, 0);
				if (status == CR_SUCCESS) {
					status = CM_Get_Device_ID(devInstParent, szDeviceInstanceID, (ULONG)szDeviceInstanceID, 0);
					if (status == CR_SUCCESS) {
						printf("CM_Get_Device_ID: InstanceID %s\n", szDeviceInstanceID);
					}
					else {
						printf("CM_Get_Device_ID: Failed status = %d GetLAstError() = %d\n", status, GetLastError());
					}
				}
				else {
					printf("Get Parent: Failed status = %d GetLastError()= %d\n", status, GetLastError());
				}
			}
			//for new windows version we can directly access parent path - &DEVPKEY_Device_Parent
			else {
				HMODULE hSetupApi = GetModuleHandleW(L"setupapi.dll");
				if (hSetupApi == NULL) {
					// If not loaded already, load it
					hSetupApi = LoadLibraryW(L"setupapi.dll");
					if (hSetupApi == NULL) {
						printf("Failed to load setupapi.dll. Error: %lu\n", GetLastError());
						return 1;
					}
				}

				FN_SetupDiGetDevicePropertyW fn_SetupDiGetDevicePropertyW =
					(FN_SetupDiGetDevicePropertyW)GetProcAddress(hSetupApi, "SetupDiGetDevicePropertyW");

				if (fn_SetupDiGetDevicePropertyW == NULL) {
					printf("Failed to get address of SetupDiGetDevicePropertyW. Error: %lu\n", GetLastError());
					return 1;
				}

				if (fn_SetupDiGetDevicePropertyW(g_DeviceInfoTable, &g_DevInfoData, &DEVPKEY_Device_Parent, &ulPropertyType, (BYTE*)wszDeviceInstanceID, sizeof(szDeviceInstanceID), &g_dwRegSize, 0)) {
					WideCharToMultiByte(CP_ACP, 0, wszDeviceInstanceID, -1, szDeviceInstanceID,  sizeof(szDeviceInstanceID), NULL, NULL);
					printf("Parent Property value: %s\n", szDeviceInstanceID);
				}
				else {
					printf("fn_SetupDiGetDevicePropertyW : Failed GetLAstError() = %d \n", GetLastError());
				}
			}

			if (FindMatchDevice(szDeviceInstanceID)) {
				g_InstanceID_substr = strstr(strupr(szDeviceInstanceID), "MI_");
				if (g_InstanceID_substr != NULL) {
					strncpy(g_extrctd_InstanceID, g_InstanceID_substr + 6, 10);
					g_extrctd_InstanceID[11] = '\0';
					strcpy(g_HIDInstanceID, g_extrctd_InstanceID);
					printf("HIDInstanceID: %s\n", g_HIDInstanceID);
				}
			}
			
			if (wcscmp(g_HIDInstanceID, g_USBInstanceID) == 0) {
				//Check for free slots in g_WriteHandle array
				//If the slot is free the handle is either INVALID_HANDLE_VALUE or NULL
				for (g_CurrInstanceIndex = 0; g_CurrInstanceIndex < MAX_NUMBER_OF_DEVICES; g_CurrInstanceIndex++) {
					if ((g_WriteHandle[g_CurrInstanceIndex] == INVALID_HANDLE_VALUE) || (g_WriteHandle[g_CurrInstanceIndex] == NULL)) {
						break;
					}
				}
				OutputDebugStringW(L"Device Found \r\n");
				g_WriteHandle[g_CurrInstanceIndex] = INVALID_HANDLE_VALUE;
				//Opens a write handle at device path
				g_WriteHandle[g_CurrInstanceIndex] = CreateFile((g_DetailedInterfaceDataStructure->DevicePath), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
				//Checks if the write handle opening failed
				if(g_WriteHandle[g_CurrInstanceIndex] == INVALID_HANDLE_VALUE) {
					g_ErrorStatus = GetLastError();
					if (!g_ErrorStatus) {
						SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
						return NULL;
					}
				}

				//Opens a read handle at device path
				g_ReadHandle[g_CurrInstanceIndex] = INVALID_HANDLE_VALUE;
				g_ReadHandle[g_CurrInstanceIndex] = CreateFile((g_DetailedInterfaceDataStructure->DevicePath), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, 0);
				//check whether creation of read handle failed
				if (g_ReadHandle[g_CurrInstanceIndex] == INVALID_HANDLE_VALUE) {
					g_ErrorStatus = GetLastError();
					if (!g_ErrorStatus) {
						SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
						return NULL;
					}
				}

				g_ExtensionUnitCount++;
				//Initializes the OVERLAPPED structure for asynchronous I / O
				g_lpOverlapped.Offset = g_lpOverlapped.OffsetHigh = 0;
				g_lpOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				grabFrameHandle = CreateEvent(NULL, TRUE, TRUE, L"Grab Frame Event");
				if (grabFrameHandle == INVALID_HANDLE_VALUE) {
					CloseHandle(grabFrameHandle);
					grabFrameHandle = INVALID_HANDLE_VALUE;
					return NULL;
				}
				
				otherHIDHandle = CreateEvent(NULL, TRUE, TRUE, L"Other HID Call Event");
				if (otherHIDHandle == INVALID_HANDLE_VALUE) {
					CloseHandle(otherHIDHandle);
					otherHIDHandle = INVALID_HANDLE_VALUE;
					return NULL;
				}

				SetupDiDestroyDeviceInfoList(g_DeviceInfoTable);
				printf("Handle Before: %x\n", *handle);
				//Assigns write handle
				*handle = (UINT32*)(g_WriteHandle[g_CurrInstanceIndex]);

				printf("Handle After: %x\n", *handle);
				return TRUE;
			}
			else
			{
				OutputDebugStringW(L"DeviceConnect: Device Not Found \r\n");
			}
			
		}
		else {
		OutputDebugStringW("DeviceConnect: Device Not Found \r\n");
		}
		g_InterfaceIndex++;
	}
	return FALSE;
	
}

BOOL FindMatchHandle(UINT32 *handle) {
	BOOL isMatchFound = FALSE;

	if (g_ExtensionUnitCount == 0) {
		return FALSE;
	}
	for(g_CurrInstanceIndex = 0; g_CurrInstanceIndex < MAX_NUMBER_OF_DEVICES; g_CurrInstanceIndex++) {
		if (handle == (UINT32*)g_WriteHandle[g_CurrInstanceIndex]) {
			printf("Matching handle found\n");
			isMatchFound = TRUE;
			break;
		}
	}
	return isMatchFound;
}
BOOL DeInitExtension(UINT32 *handle) {
	if (handle == NULL) {
		return FALSE;
	}
	//Check if the handle exist
	if (!FindMatchHandle(handle)) {
		printf("DeinitExtensionUnit::Find handle failed...\n");
		return FALSE;
	}
	//Check whether the write handle creation was unsuccessfull
	if ((g_WriteHandle[g_CurrInstanceIndex] == INVALID_HANDLE_VALUE) || g_WriteHandle[g_CurrInstanceIndex] == NULL) {
		printf("DeinitExtensionUnit::Invalid Handle Value...\n");
		return FALSE;
	}
	//Close the write handle if it is prsesnt
	if ((g_WriteHandle[g_CurrInstanceIndex] != INVALID_HANDLE_VALUE) || g_WriteHandle[g_CurrInstanceIndex] != NULL) {
		CloseHandle(g_WriteHandle[g_CurrInstanceIndex]);
		g_WriteHandle[g_CurrInstanceIndex] = NULL;
	}
	//Close the read handle if it is present
	if ((g_ReadHandle[g_CurrInstanceIndex] != INVALID_HANDLE_VALUE) || g_ReadHandle[g_CurrInstanceIndex] != NULL) {
		CloseHandle(g_ReadHandle[g_CurrInstanceIndex]);
		g_ReadHandle[g_CurrInstanceIndex] = NULL;
	}

	if (g_lpOverlapped.hEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(g_ReadHandle[g_CurrInstanceIndex]);
		g_ReadHandle[g_CurrInstanceIndex] = NULL;
	}

	if (otherHIDHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(otherHIDHandle);
		otherHIDHandle = INVALID_HANDLE_VALUE;
	}
	if (grabFrameHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(grabFrameHandle);
		grabFrameHandle = INVALID_HANDLE_VALUE;
	}
	if (g_ReadTrgiggerStatusHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(g_ReadTrgiggerStatusHandle);
		g_ReadTrgiggerStatusHandle = INVALID_HANDLE_VALUE;
	}

	g_CurrInstanceIndex = 0;
	g_ExtensionUnitCount = 0;
	DeleteCriticalSection(&g_critsec);
	CoUninitialize();
	return TRUE;
}