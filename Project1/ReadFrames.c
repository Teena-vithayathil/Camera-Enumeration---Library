#include "EnumerateCamera.h"

UINT32 count, i;
int id;
camera_details cameras[10];


void getFrames(IMFActivate *ppDevice) {
	IMFMediaSource *pMediaSource = NULL;
	IMFAttributes *pAttributes = NULL;
	IMFSourceReader *ppSourceReader;
	IMFMediaType *pType = NULL;
	UINT32 width = 0, height = 0;


	//Intializing COM runtime
	//hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)) {
		
		//Initializing media foundation function
		hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
		if (SUCCEEDED(hr)) {
		
			//Activating selected device
			hr = ppDevice->lpVtbl->ActivateObject(ppDevice,&IID_IMFMediaSource,(void**)&pMediaSource);
			if (SUCCEEDED(hr)) {

				//Creating source reader
				hr = MFCreateSourceReaderFromMediaSource(pMediaSource, NULL, &ppSourceReader);
				if (SUCCEEDED(hr)) {

					//Setting media type
					hr = MFCreateMediaType(&pType);
					if (SUCCEEDED(hr)) {
						hr = pType->lpVtbl->SetGUID(pType, &MF_MT_MAJOR_TYPE, &MFMediaType_Video);
						hr = pType->lpVtbl->SetGUID(pType, &MF_MT_SUBTYPE, &MFVideoFormat_RGB32);
						if (SUCCEEDED(hr)) {
							hr = ppSourceReader->lpVtbl->SetCurrentMediaType(ppSourceReader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pType);

							while (1) {
								DWORD streamIndex, flags;
								LONGLONG timestamp;
								IMFSample* pSample = NULL;
								hr = ppSourceReader->lpVtbl->ReadSample(ppSourceReader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, &pSample);
								if (SUCCEEDED(hr)) {
									if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
									if (pSample) {
										IMFMediaBuffer *pBuffer = NULL;
										hr = pSample->lpVtbl->ConvertToContiguousBuffer(pSample, &pBuffer);
										if (SUCCEEDED(hr)) {
											BYTE* pData = NULL;
											DWORD maxLength = 0, currentLength = 0;
											hr = pBuffer->lpVtbl->Lock(pBuffer, &pData, &maxLength, &currentLength);
											if (SUCCEEDED(hr)) {
												//Get frame size from media type
												//hr = MFGetAttributeSize(pType, &MF_MT_FRAME_SIZE, &width, &height);
											

												pBuffer->lpVtbl->Unlock(pBuffer);
											}
										}
										pBuffer->lpVtbl->Release(pBuffer);
									}
									pSample->lpVtbl->Release(pSample);
								}
							}
						}
					}
				}
				else {
					printf("No device capture devcies found\n");
				}
				
			}
			CoTaskMemFree(ppDevice);
		}
	}

	MFShutdown();
}

