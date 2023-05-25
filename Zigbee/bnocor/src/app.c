#include <appTimer.h>
#include <zdo.h>
#include <app.h>
#include <sysTaskManager.h>
#include <usartManager.h>
#include <bspLeds.h>
#include <test.h>

static AppState_t appstate = APP_INIT;
static uint8_t deviceType;
static void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo);
static ZDO_StartNetworkReq_t networkParams;


static SimpleDescriptor_t simpleDescriptor;
static APS_RegisterEndpointReq_t endPoint;
static void initEndpoint(void);
void APS_DataInd(APS_DataInd_t *indData);

HAL_AppTimer_t receiveTimerLed;
HAL_AppTimer_t transmitTimerLed;
HAL_AppTimer_t transmitTimer;
static void receiveTimerLedFired(void);


static void initTimer(void);

BEGIN_PACK
typedef struct _AppMessage_t{
	uint8_t header[APS_ASDU_OFFSET]; //APS header
	uint8_t data[6];
	uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET]; // Footer
} PACK AppMessage_t;
END_PACK
static AppMessage_t transmitData;
APS_DataReq_t dataReq;
static void APS_DataConf(APS_DataConf_t *confInfo);
static void initTransmitData(void);











void APL_TaskHandler(void){
	switch(appstate){
		case APP_INIT:
		appInitUsartManager();
		initTimer();
		BSP_OpenLeds();
		appstate=APP_STARTJOIN_NETWORK;
		SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_STARTJOIN_NETWORK:
		networkParams.ZDO_StartNetworkConf = ZDO_StartNetworkConf;
		
		ZDO_StartNetworkReq(&networkParams);
		appstate=APP_INIT_ENDPOINT;
		
	break;
		case APP_INIT_ENDPOINT:
		initEndpoint();
	#if CS_DEVICE_TYPE == DEV_TYPE_COORDINATOR
		appstate=APP_NOTHING;
	#else
		appstate=APP_INIT_TRANSMITDATA;
	#endif
		SYS_PostTask(APL_TASK_ID);
	break;
		
		appstate=APP_NOTHING;
		
	break;
		case APP_TRANSMIT:
	
		APS_DataReq(&dataReq);
		break;
		
		case APP_NOTHING:
		break;
	}
}


static void initEndpoint(void){
	simpleDescriptor.AppDeviceId = 1;
	simpleDescriptor.AppProfileId = 1;
	simpleDescriptor.endpoint = 1;
	simpleDescriptor.AppDeviceVersion = 1;
	endPoint.simpleDescriptor= &simpleDescriptor;
	endPoint.APS_DataInd = APS_DataInd;
	APS_RegisterEndpointReq(&endPoint);
}


void APS_DataInd(APS_DataInd_t *indData){
	BSP_OnLed(LED_RED);
	HAL_StartAppTimer(&receiveTimerLed);
	
	 if (indData->asdu[0] == 1){
		 rawDataToUart(indData->asdu);		 
	 }else{
		quatDataToUart(indData->asdu); 
	 }
	 
	
}

static void initTimer(void){

	receiveTimerLed.interval= 500;
	receiveTimerLed.mode= TIMER_ONE_SHOT_MODE;
	receiveTimerLed.callback=receiveTimerLedFired;
	
}

static void receiveTimerLedFired(void){
	BSP_OffLed(LED_RED);
}



static void APS_DataConf(APS_DataConf_t *confInfo){
	if (confInfo->status == APS_SUCCESS_STATUS){
		BSP_OnLed(LED_YELLOW);
		HAL_StartAppTimer(&transmitTimerLed);
		appstate=APP_NOTHING;
		SYS_PostTask(APL_TASK_ID);
	}
}



void rawDataToUart(uint8_t* data)
{
	
	float f;
	int16_t value16;
	char arr[50];
	double decpart;
	
	// Ausgabe von accx
	uint8_t index=ACCEL_INDEX;
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	//Auflösung 14 bit = 16384: bno055 hat 14 bit Auflösung für quats. Der Wert muss daher durch 2^14 geteilt werden.
	f=(float) value16 / 100.0;
	
	// Integerpart
	int intpart = ((int)f);
	decpart = (f) - (intpart);
	//Nachkommapart
	int dec = (int)(decpart*1000);
	
	//sprintf auf ATmega kennt %f für double und float-Umwandlung nicht. Deshalb %d usw
	if (f<0)
	sprintf(arr,"0, -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,"0, %03d.%03d",intpart, abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending accy
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 100.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending accz
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 100.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending gyrox
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending gyroy
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending gyroz
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending magx
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending magy
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending magz
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%03d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %03d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	int bat = (int16_t)data[BAT_INDEX];
	
	sprintf(arr,", %03d",abs(bat));
	
	appWriteDataToUsart(arr, strlen(arr));
	appWriteDataToUsart((uint8_t*)"\r\n",strlen("\r\n"));
	
}

void quatDataToUart(uint8_t* data)
{
	
	float f;
	int16_t value16;
	char arr[50];
	double decpart;
	
	// Ausgabe von quat.w
	uint8_t index=QUAT_INDEX;
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	//Auflösung 14 bit = 16384: bno055 hat 14 bit Auflösung für quats. Der Wert muss daher durch 2^14 geteilt werden.
	f=(float) value16 / 16384.0;
	
	// Integerpart
	int intpart = ((int)f);
	decpart = (f) - (intpart);
	//Nachkommapart
	int dec = (int)(decpart*1000);
	
	//sprintf auf ATmega kennt %f für double und float-Umwandlung nicht. Deshalb %d usw
	if (f<0)
	sprintf(arr," 1, -%d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr," 1, %d.%03d",intpart, abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending quat.x
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16384.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	index+=2;
	// sending quat.y
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16384.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr,", -%d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr,", %d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr, strlen(arr));
	
	char arr3[50];
	index+=2;
	//sending quat.z
	value16 = (int16_t)((((int32_t)	((int8_t)data[index+1])) <<	8) | (data[index]));
	f=(float) value16 / 16384.0;
	intpart = ((int)f);
	decpart = (f) - (intpart);
	dec = (int)(decpart*1000);
	if (f<0)
	sprintf(arr3,", -%d.%03d",abs(intpart), abs(dec));
	else
	sprintf(arr3,", %d.%03d",abs(intpart), abs(dec));
	appWriteDataToUsart(arr3, strlen(arr3));
	int bat = (int16_t)data[BAT_INDEX];
	
	sprintf(arr3,", %03d",abs(bat));
	
	appWriteDataToUsart(arr3, strlen(arr3));
	appWriteDataToUsart((uint8_t*)"\r\n",strlen("\r\n"));
	
}









void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo){
	if (ZDO_SUCCESS_STATUS == confirmInfo->status){
		CS_ReadParameter(CS_DEVICE_TYPE_ID,&deviceType);
		if(deviceType==DEV_TYPE_ENDDEVICE){
			appWriteDataToUsart((uint8_t*)"EndDevice\r\n", sizeof("EndDevice\r\n")-1);
		}
		BSP_OnLed(LED_GREEN);
		}else{
		appWriteDataToUsart((uint8_t*)"Error\r\n",sizeof("Error\r\n")-1);
	}
	SYS_PostTask(APL_TASK_ID);
}

void ZDO_MgmtNwkUpdateNotf(ZDO_MgmtNwkUpdateNotf_t *nwkP){
	nwkP = nwkP;
	
	
}
void ZDO_WakeUpInd(void){}

#ifdef _BINDING_
void ZDO_BindIndication(ZDO_BindInd_t *bindI){(void)bindI;}
void ZDO_UnbindIndication(ZDO_UnbindInd_t *unbindI){(void)unbindI;}
#endif //_BINDING_

int main(void){
	SYS_SysInit();
	for(;;){ SYS_RunTask(); }
}