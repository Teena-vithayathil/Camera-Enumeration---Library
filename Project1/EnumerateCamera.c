
#include "EnumerateCamera.h"

void getCameraDetails() {
	HRESULT hr;
	IMFAttributes *pAttributes;
	IMFActivate **ppDevices;
	UINT32 pcSourceActive = 0, i, pcchLength;
	WCHAR *camera_name, *camera_path;
	char name[50], path[100];

	//Intializing media foundation functions
	hr = MFStartup(MF_VERSION,MFSTARTUP_FULL);
	if (SUCCEEDED(hr)) {

		//Create an attribute store
		hr = MFCreateAttributes(&pAttributes, 1);
		if (SUCCEEDED(hr)) {

			//Defining the source type: video capturing devices
			hr = pAttributes->lpVtbl->SetGUID(pAttributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
			if (SUCCEEDED(hr)) {

				//Enumerate the devices
				hr = MFEnumDeviceSources(pAttributes, &ppDevices, &pcSourceActive);
				if (SUCCEEDED(hr)) {
					if (pcSourceActive > 0) {
						printf("Total no of camera devices connected: %d\n", pcSourceActive);
						for (i = 0; i < pcSourceActive; i++) {
							//Accessing name of the camera
							hr = ppDevices[i]->lpVtbl->GetAllocatedString(ppDevices[i], &MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &camera_name, &pcchLength);
							if (SUCCEEDED(hr)) {
								WideCharToMultiByte(CP_UTF8, 0, camera_name, -1,name, sizeof(name), NULL, NULL);
								printf("Camera name: %s\n", name);
								CoTaskMemFree(camera_name);
							}

							//Accessing path of the camera
							hr = ppDevices[i]->lpVtbl->GetAllocatedString(ppDevices[i], &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &camera_path, &pcchLength);
							if (SUCCEEDED(hr)) {
								WideCharToMultiByte(CP_UTF8, 0, camera_path, -1, path, sizeof(path), NULL, NULL);
								printf("Camera path: %s\n\n", path);
								CoTaskMemFree(camera_path);
							}
						}
					}
					else {
						printf("No devices connected\n");
					}
				}
			}
		}
	}
}

