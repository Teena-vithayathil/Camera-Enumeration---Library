#include "EnumerateCamera.h"

void getUVCSettings(IMFActivate *pDevice) {
	HRESULT hr;
	IMFMediaSource *pMediaSource;
	IAMVideoProcAmp *pProcAmp;
	IAMCameraControl *pCamControl;
	//Initalizing media foundation
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
	if (SUCCEEDED(hr)) {

		//Activating selected device
		hr = pDevice->lpVtbl->ActivateObject(pDevice, &IID_IMFMediaSource, &pMediaSource);
		if (SUCCEEDED(hr)) {
			long flag, value;

			//Retrieving IAMVideoProcAmp interface - Video setiings
			hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMVideoProcAmp, &pProcAmp);
			if (SUCCEEDED(hr)) {
				//Retrieving Brightness
				hr = pProcAmp->lpVtbl->Get(pProcAmp, VideoProcAmp_Brightness, &value, &flag);
				if (SUCCEEDED(hr)) {
					printf("Brightness: %ld\n", value);
				}

				//Retrieving white balance
				hr = pProcAmp->lpVtbl->Get(pProcAmp, VideoProcAmp_WhiteBalance, &value, &flag);
				if (SUCCEEDED(hr)) {
					printf("White Balance: %ld\n", value);
				}
			}

			//Retrieving IAMCameraControl - Hardware settings
			hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMCameraControl, &pCamControl);
			if (SUCCEEDED(hr)) {
				//Retrieving Exposure
				hr = pCamControl->lpVtbl->Get(pCamControl, CameraControl_Exposure, &value, &flag);
				if (SUCCEEDED(hr)) {
					printf("Exposure: %ld\n", value);
				}
			}

		}
	}
	MFShutdown();
	CoUninitialize();
}