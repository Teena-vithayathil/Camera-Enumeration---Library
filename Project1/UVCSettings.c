#include "EnumerateCamera.h"

//Returns video setting value
int getVideoSettings(IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop, long *flag) {
	long value;
	hr = pProcAmp->lpVtbl->Get(pProcAmp, prop, &value, flag);
	if (SUCCEEDED(hr)) {
		return value;
	}
	else {
		printf("Error fetching value.\n");
		return 0;
	}
}

//Get video setting range, step value
int get_range_VideoSetting(IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop, long *min, long *max, long *step, long *flag) {
	long def;
	hr = pProcAmp->lpVtbl->GetRange(pProcAmp, prop, min, max, step, &def, flag);
	if (SUCCEEDED(hr)) {
		return 1;
	}
	else {
		printf("Error fetching range\n");
		return 0;
	}
}

//Returns camera setting value
int getCameraControls(IAMCameraControl *pCamControl, CameraControlProperty prop, long *flag) {
	long value;
	hr = pCamControl->lpVtbl->Get(pCamControl, prop, &value, flag);
	if (SUCCEEDED(hr)) {
		return value;
	}
	else {
		printf("Error fetching value.\n");
		return 0;
	}
}

//Get camera setting range, step value
 int get_range_CameraControls(IAMCameraControl *pCamControl, CameraControlProperty prop, long *min, long *max, long *step, long *flag) {
	long def;
	hr = pCamControl->lpVtbl->GetRange(pCamControl, prop, min, max, step, &def, &flag);
	if (SUCCEEDED(hr)) {
		return 1;
	}
	else {
		printf("Error fetching range\n");
		return 0;
	}
}

 //Checks the status of flag - Video settings
 int check_flag_VideoSettings(long flag) {
	 int type;
	 if ((flag & VideoProcAmp_Flags_Auto) && (flag & VideoProcAmp_Flags_Manual)){
		 printf("Auto & Manual\n");
		 return 1;
	 }
	 else if (flag & VideoProcAmp_Flags_Auto) {
		 printf("Auto\n");
	 }
	 else {
		 printf("Manual\n");
	 }
	 return 0;
 }

 //Checks the status of flag - Camera controls
 int check_flag_CameraControls(long flag) {
	 int type;
	 if (flag & CameraControl_Flags_Auto && flag & CameraControl_Flags_Manual) {
		 printf("Auto & Manual\n");
		 return 1;
	 }
	 else if (flag & CameraControl_Flags_Auto) printf("Auto\n");
	 else printf("Manual\n");
	 return 0;
 }

void getUVCSettings(IMFActivate *pDevice) {
	IMFMediaSource *pMediaSource;
	IAMVideoProcAmp *pProcAmp;
	IAMCameraControl *pCamControl;

	//Activating selected device
	hr = pDevice->lpVtbl->ActivateObject(pDevice, &IID_IMFMediaSource, &pMediaSource);
	if (SUCCEEDED(hr)) {
		long value,min,max,step, flag, gflag;
		char flag_details[10];
		//Retrieving IAMVideoProcAmp interface - Video setiings
		hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMVideoProcAmp, &pProcAmp);
		if (SUCCEEDED(hr)) {
			//Retrieving Brightness
			value = getVideoSettings(pProcAmp, VideoProcAmp_Brightness, &flag);
			printf("Brightness: %ld\n", value);
			get_range_VideoSetting(pProcAmp, VideoProcAmp_Brightness, &min, &max, &step, &gflag);
			printf("Minimum value: %ld\tMaximum value:%ld\tStep value: %ld\tFlag: ", min, max, step);
			check_flag_VideoSettings(flag);

			//Retrieving white balance
			value = getVideoSettings(pProcAmp, VideoProcAmp_WhiteBalance, &flag);
			printf("White Balance: %ld\n", value);
			get_range_VideoSetting(pProcAmp, VideoProcAmp_WhiteBalance, &min, &max, &step, &gflag);
			printf("Minimum value: %ld\tMaximum value:%ld\tStep value: %ld\tFlag: ", min, max, step);
			check_flag_VideoSettings(flag);
		}

		//Retrieving IAMCameraControl - Hardware settings
		hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMCameraControl, &pCamControl);
		if (SUCCEEDED(hr)) {
			value = getCameraControls(pCamControl, CameraControl_Exposure, &flag);
			printf("Exposure: %ld\n", value);
			get_range_CameraControls(pCamControl, CameraControl_Exposure, &min, &max, &step, &gflag);
			printf("Minimum value: %ld\tMaximum value:%ld\tStep value: %ld\tFlag: ", min, max, step);
			check_flag_CameraControls(flag);
		}

	}
}

//Checking the range, step value for the new value for video settings
int check_range_stepValue_VideoSettings(long value, IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop) {
	long min, max, step, def, flag;
	if (get_range_VideoSetting(pProcAmp, prop, &min, &max, &step, &flag)) {
		if (value >= min && value <= max && value%step == 0){
			return 1;
		}
		else {
			printf("Cannot change value- Issue with Range/ Step value\n");
			return 0;
		}
	}
}

//Checking the range, step value for the new value for camera controls
int check_range_stepValue_CameraControls(long value, IAMCameraControl *pCamControl, CameraControlProperty prop) {
	long min, max, step, def, flag;
	if (get_range_CameraControls(pCamControl, prop, &min, &max, &step, &flag)) {
		if (value >= min && value <= max && value%step == 0) {
			return 1;
		}
		else {
			printf("Cannot change value- Issue with Range/ Step value\n");
			return 0;
		}
	}
}

//Setting the values for video settings
void set_value_VideoSettings(IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop, char type[50]) {
	long value, min, max, step, check_value, flag;

	if (get_range_VideoSetting(pProcAmp, prop, &min, &max, &step, &flag)) {
		printf("Enter new value (Range: %ld - %ld): ", min, max);
		scanf("%ld", &value);
		if (check_range_stepValue_VideoSettings(value, pProcAmp, prop)) {
			hr = pProcAmp->lpVtbl->Set(pProcAmp, prop, value, VideoProcAmp_Flags_Manual);
			if (SUCCEEDED(hr)) {
				check_value = getVideoSettings(pProcAmp, prop, &flag);
				printf("Fetching %s value: %ld\n", type, check_value);
			}
			else {
				printf("Unable to change %s value\n", type);
			}
		}
	}
}

//Setting the values for camera controls
void set_value_CameraControls(IAMCameraControl *pCamControl, CameraControlProperty prop, char type[50]) {
	long value, min, max, step, check_value, flag;

	if (get_range_CameraControls(pCamControl, prop, &min, &max, &step, &flag)) {

		printf("Enter new value (Range: %ld - %ld): ", min, max);
		scanf("%ld", &value);
		if (check_range_stepValue_CameraControls(value, pCamControl, prop)) {
			hr = pCamControl->lpVtbl->Set(pCamControl, prop, value, CameraControl_Flags_Manual);
			if (SUCCEEDED(hr)) {
				check_value = getCameraControls(pCamControl, prop, &flag);
				printf("Fetching %s value: %ld\n", type, check_value);
			}
			else {
				printf("Unable to change %s value\n", type);
			}
		}
	}
}
void set_flag_VideoSettings(IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop, long value, long flag) {
	hr = pProcAmp->lpVtbl->Set(pProcAmp, prop, value, flag);
	if (SUCCEEDED(hr)) {
		printf("Flag changed\n");
	}
	else {
		printf("Unable to change flag\n");
	}
}

void change_flag_VideoSettings(IAMVideoProcAmp *pProcAmp, VideoProcAmpProperty prop) {
	long min, max, step, flag;
	char ch;
	if (get_range_VideoSetting(pProcAmp, prop, &min, &max, &step, &flag)) {
		printf("Supported mode: ");
		if (check_flag_VideoSettings(flag)) {
			printf("Enter your choice (Auto(a) / Manual(m)): ");
			scanf_s(" %c", &ch);
			if (ch == 'm') {
				long value = getVideoSettings(pProcAmp, prop, &flag);
				set_flag_VideoSettings(pProcAmp, prop, value, VideoProcAmp_Flags_Manual);
			}
			else if (ch == 'a') set_flag_VideoSettings(pProcAmp, prop, 0, VideoProcAmp_Flags_Auto);
			else {
				printf("Invalid choice\n");
			}
		}
	}
}

void set_flag_CameraControls(IAMCameraControl *pCamControl, CameraControlProperty prop, long value, long flag) {
	hr = pCamControl->lpVtbl->Set(pCamControl, prop, value, flag);
	if (SUCCEEDED(hr)) {
		printf("Flag changed\n");
	}
	else {
		printf("Unable to change flag\n");
	}
}

void change_flag_CameraControls(IAMCameraControl *pCamControl, CameraControlProperty prop) {
	long min, max, step, flag;
	char ch;
	if (get_range_VideoSetting(pCamControl, prop, &min, &max, &step, &flag)) {
		printf("Supported mode: ");
		if (check_flag_VideoSettings(flag)) {
			printf("Enter your choice (Auto(a) / Manual(m)): ");
			scanf_s(" %c", &ch);
			if (ch == 'm') {
				long value = getVideoSettings(pCamControl, prop, &flag);
				set_flag_VideoSettings(pCamControl, prop, value, CameraControl_Flags_Manual);
			}
			else if (ch == 'a') set_flag_VideoSettings(pCamControl, prop, 0, CameraControl_Flags_Auto);
			else {
				printf("Invalid choice\n");
			}
		}
	}
}

void set_UVC_settings(IMFActivate *pDevice, char type[50]) {
	HRESULT hr;
	IMFMediaSource *pMediaSource;
	IAMVideoProcAmp *pProcAmp;
	IAMCameraControl *pCamControl;
	int ch = 1;
	long flag; 

	//Activating selected device
	hr = pDevice->lpVtbl->ActivateObject(pDevice, &IID_IMFMediaSource, &pMediaSource);
	if (SUCCEEDED(hr)) {
		//Retrieving IAMVideoProcAmp interface - Video settings
		hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMVideoProcAmp, &pProcAmp);
		VideoProcAmpProperty prop;
		if (SUCCEEDED(hr)) {
			if (strcmp(type, "Brightness") == 0) {
				while (ch != 4) {
					printf("1.Get Value \n2.Change Value \n3.Change Flag \n4.Exit \nEnter your choice: ");
					scanf("%d", &ch);
					if (ch==1) printf("Brightness: %ld\n", getVideoSettings(pProcAmp, VideoProcAmp_Brightness, &flag));
					else if (ch==2) set_value_VideoSettings(pProcAmp, VideoProcAmp_Brightness, type);
					else if (ch==3) change_flag_VideoSettings(pProcAmp, VideoProcAmp_Brightness);
					else if (ch==4)break;
					else printf("Invalid option\n");
				}
			}
			else if(strcmp(type, "White Balance") == 0) {
				while (ch != 4) {
					printf("1.Get Value\n2.Change Value \n3.Change Flag \n4.Exit \nEnter your choice: ");
					scanf("%d", &ch);
					if(ch==1) printf("White Balance: %ld\n", getVideoSettings(pProcAmp, VideoProcAmp_WhiteBalance, &flag));
					else if (ch == 2) set_value_VideoSettings(pProcAmp, VideoProcAmp_WhiteBalance, type);
					else if (ch == 3) change_flag_VideoSettings(pProcAmp, VideoProcAmp_WhiteBalance);
					else if (ch == 4)break;
					else printf("Invalid option\n");
				}
			}
		}
		//Retrieving IAMCameraControl - Hardware settings
		hr = pMediaSource->lpVtbl->QueryInterface(pMediaSource, &IID_IAMCameraControl, &pCamControl);
		if (SUCCEEDED(hr)) {
			if (strcmp(type, "Exposure")==0) {
				while (ch != 4) {
					printf("1.Get Value\n2.Change Value \n3.Change Flag \n4.Exit \nEnter your choice: ");
					scanf("%d", &ch);
					if (ch == 1) printf("White Balance: %ld\n", getVideoSettings(pCamControl, CameraControl_Exposure, &flag));
					else if (ch == 2) set_value_VideoSettings(pCamControl, CameraControl_Exposure, type);
					else if (ch == 3) change_flag_VideoSettings(pCamControl, CameraControl_Exposure);
					else if (ch == 4)break;
					else printf("Invalid option\n");
				}
			}
		}
	}
}