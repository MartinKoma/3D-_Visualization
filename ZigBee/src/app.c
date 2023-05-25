/**************************************************************************//**
  \file app.c

  \brief bno055 Integration 

  \author ...

******************************************************************************/


#include <appTimer.h>
#include <zdo.h>
#include <app.h>
#include <sysTaskManager.h>
#include <usartManager.h>
#include <halTaskManager.h>
#include <util/delay.h>
#include <bno055_port.h>
#include <stdio.h>
#include <adc.h>



/********************** Batterieüberwachung ******************************* */
uint8_t adcData=0;





/* ************************* ZIGBEE **************************** */
static uint8_t deviceType;
static void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo);
static ZDO_StartNetworkReq_t networkParams;


BEGIN_PACK
typedef struct _AppMessage_t{
	uint8_t header[APS_ASDU_OFFSET]; //APS header
	uint8_t data[MAX_INDEX+1];  // buffer for data transfer. MAX_INDEX defined in app.h
	uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET]; // Footer
} PACK AppMessage_t;
END_PACK

// array for Data-Transmission. Sensor-Data from read_sensor
static AppMessage_t TransmitData;




/*
TransmitData.data[0]..[MAX_INDEX]

[DEVICE_ID_INDEX] = DEV_ID // configuration.h

[GYRO_INDEX+0] = gx lsb //16 bit solution
[GYRO_INDEX+1] = gx msb 
[GYRO_INDEX+2] = gy lsb 
[GYRO_INDEX+3] = gy msb 
[GYRO_INDEX+4] = gz lsb 
[GYRO_INDEX+5] = gz msb 

[ACCEL_INDEX+0] = ax lsb // 14 bit
[ACCEL_INDEX+1] = ax msb
[ACCEL_INDEX+2] = ay lsb
[ACCEL_INDEX+3] = ay msb
[ACCEL_INDEX+4] = az lsb
[ACCEL_INDEX+5] = az msb

[MAG_INDEX+0] = mx lsb //13 bit
[MAG_INDEX+1] = mx msb
[MAG_INDEX+2] = my lsb //13 bit
[MAG_INDEX+3] = my msb
[MAG_INDEX+4] = mz lsb //15 bit
[MAG_INDEX+5] = mz msb

[QUAT_INDEX+0] = qw lsb //14 bit
[QUAT_INDEX+1] = qw msb
[QUAT_INDEX+2] = qx lsb
[QUAT_INDEX+3] = qx msb
[QUAT_INDEX+4] = qy lsb
[QUAT_INDEX+5] = qy msb
[QUAT_INDEX+6] = qz lsb
[QUAT_INDEX+7] = qz msb

[CALIB_INDEX+0] = gyro calib state 0..3
[CALIB_INDEX+1] = accel calib state 0..3
[CALIB_INDEX+2] = mag calib state 0..3

[BAT_INDEX] = bat state 0..150

* sample for data transformation (quat.w)
s16 value16 = (s16)((((s32)	((s8)data[QUAT_INDEX+1])) <<	8) | (data[QUAT_INDEX]));
* Quat-Auflösung 14 bit = 16384. Der Wert muss daher durch 2^14 geteilt werden.
float w=(float) ( (float) value16 / (float) (2^14));


*/


static void readSensorDoneCb(void)
{
	TransmitData.data[BAT_INDEX]=adcData;
}

static HAL_AdcDescriptor_t adcdescriptor = {
	.resolution = RESOLUTION_8_BIT,
	.sampleRate = ADC_4800SPS,
	.voltageReference = AVCC,
	.bufferPointer = &adcData,
	.selectionsAmount = 1,
	.callback = readSensorDoneCb
};


APS_DataReq_t dataReq;

static void APS_DataConf(APS_DataConf_t *confInfo);
static void initTransmitData(void);
static SimpleDescriptor_t simpleDescriptor;
static APS_RegisterEndpointReq_t endPoint;
static void initEndpoint(void);
void APS_DataInd(APS_DataInd_t *indData);


/* ************************* SENSOR  *********************************** */

HAL_AppTimer_t readSensorTimer;
HAL_AppTimer_t resetsensortimer;
static void readSensorTimerFired(void);
static void resettimerfired(void);

static void set_external_crystal();

static void reset_sensor(void){
	if (bno055_initialize()==0)
	{
		bno055_set_sys_rst(1);	// reset
		resetsensortimer.interval = 650;
		resetsensortimer.mode = TIMER_ONE_SHOT_MODE;
		resetsensortimer.callback = resettimerfired;
		HAL_StartAppTimer(&resetsensortimer);
	}
	
	
	
	
	
}

static void resettimerfired(void){
	SYS_PostTask(APL_TASK_ID);
}


static void sensor_init(void)
{  
	if (bno055_initialize()==0)
	{
	
	bno055_set_power_mode(POWER_MODE_NORMAL);	
	
	set_external_crystal();
	
	// Non fusion data only ACC and GYRO		
	// We need non fusion data
	// Fusion Data: Quaternion, Euler angles, Rotation vector,	Linear acceleration, Gravity, Heading
	//bno055_set_operation_mode(OPERATION_MODE_AMG); // ANY MOTION FOR GYR, ACC AND MAG	
	bno055_set_operation_mode(OPERATION_MODE_NDOF); // fusion mode for reading quat
	
	// green led on sensor is ready
	PORTB |= (1 << PB4);
	
	// Timer for read sensor data
	readSensorTimer.interval= 50; // read sensor data with 100 hz
	readSensorTimer.mode=TIMER_REPEAT_MODE;
	readSensorTimer.callback=readSensorTimerFired;
	HAL_StartAppTimer(&readSensorTimer);
	
	} 
}


// Read Sensor Data from bno055 and save to TransmitData
static void read_sensor_data (void)
{	
	
	if(DEV_ID == 1){
			
			
			u8 bno055_accel_data[6];
			u8 bno055_gyro_data[6];
			u8 bno055_mag_data[6];
			bno055_read_accel_xyz_raw(bno055_accel_data);
			bno055_read_gyro_xyz_raw(bno055_gyro_data);
			bno055_read_mag_xyz_raw(bno055_mag_data);
			// Put bno055 data in TransmitData
			// gyro 6 byte
			for (int i=0; i<6; i++)
			TransmitData.data[GYRO_INDEX+i]=bno055_gyro_data[i];
			
			// accel 6 byte
			for (int i=0; i<6; i++)
			TransmitData.data[ACCEL_INDEX+i]=bno055_accel_data[i];
			
			// accel 6 byte
			for (int i=0; i<6; i++)
			TransmitData.data[MAG_INDEX+i]=bno055_mag_data[i];
		
		
	}else{
		u8 bno055_quat_data[8];
		bno055_read_quaternion_wxyz_raw(bno055_quat_data);
		// quat 8 byte: if needed. All 0 if OPERATION_MODE_ACCGYRO
		for (int i=0; i<8; i++)
		TransmitData.data[QUAT_INDEX+i]=bno055_quat_data[i];
		
		
		
	}
		
		
		
}


// Read Calibration states 
static void read_calibration_data (void)
{	
	// read calibration status and turn off red, yellow led, if =3 
	u8 v_gyro_calib_u8=0;
	u8 v_accel_calib_u8=0;	
	u8 v_mag_calib_u8=0;
	bno055_get_gyro_calib_stat(&v_gyro_calib_u8);
	bno055_get_accel_calib_stat(&v_accel_calib_u8);
	bno055_get_mag_calib_stat(&v_mag_calib_u8);
	
	// if gyro calib state is !=3: yellow led on
	if (v_gyro_calib_u8!=3) PORTB |= (1 << PB6);
	else PORTB &= ~(1 << PB6); // yellow off
	
	// if accel calib state is !=3: red led on
	if (v_accel_calib_u8!=3) PORTB |= (1 << PB7);
	else 
	{
		// if mag calib state is !=3: red led blinking
		if (v_mag_calib_u8!=3)
		PORTB ^= (1 << PB7); // blink red			
		else PORTB &= ~(1 << PB7);	// red off
	}
	
	// transmit calib-states		
	TransmitData.data[CALIB_INDEX]=v_gyro_calib_u8;		
	TransmitData.data[CALIB_INDEX+1]=v_accel_calib_u8;		
	TransmitData.data[CALIB_INDEX+2]=v_mag_calib_u8;	
	
}

// Timer-Callback to handle Read Sensor
static void readSensorTimerFired(void){
	
	// only from state APP_READ_BAT we change to state APP_READ_SENSOR
	if (appstate==APP_READ_BAT)	
		appstate=APP_READ_SENSOR;
	SYS_PostTask(APL_TASK_ID);
}

void APL_TaskHandler(void){

	switch(appstate){
		case APP_INIT:
			DDRB |= (1<<PB4) | (1<<PB6) | (1<<PB7);	
			#if CS_DEVICE_TYPE == DEV_TYPE_COORDINATOR
			
			PORTB &= ~(1 << PB6);	
			PORTB &= ~(1 << PB7);	
			#else // yellow and red on (for calib state)
			PORTB |= (1 << PB7);
			PORTB |= (1 << PB6);		
			TransmitData.data[DEVICE_ID_INDEX]=DEV_ID; // from configuration.h
			
			
			#endif
			PORTB &= ~(1 << PB4); // green led off. 		
			
			HAL_OpenAdc(&adcdescriptor);
			
				
			
			appInitUsartManager();
			appWriteDataToUsart((uint8_t*)"open\r\n", sizeof("open\r\n")-1); // nur zum testen		
			appstate=APP_STARTJOIN_NETWORK;
			SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_STARTJOIN_NETWORK:
			networkParams.ZDO_StartNetworkConf = ZDO_StartNetworkConf;
			ZDO_StartNetworkReq(&networkParams);
			appstate=APP_INIT_ENDPOINT;
			SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_INIT_ENDPOINT:
			initEndpoint();
			#if CS_DEVICE_TYPE == DEV_TYPE_COORDINATOR
				PORTB |= (1 << PB4); // green led off.
				
				appstate=APP_NOTHING;
			#else
				
				for (int i = 1; i < MAX_INDEX; i++)
				{
					TransmitData.data[i] = 0;
				}
				appstate=APP_RESET_SENSOR;
			#endif		
			SYS_PostTask(APL_TASK_ID);
		break;
		case APP_RESET_SENSOR:
			reset_sensor();
			appstate=APP_INIT_SENSOR;
		
		break;
		
		case APP_INIT_SENSOR:		
			initTransmitData();
			sensor_init();	
			
			appstate=APP_READ_BAT;			
			SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_READ_BAT:		
			HAL_ReadAdc(&adcdescriptor, HAL_ADC_CHANNEL0);
			// wait until readSensorTimerFired 			
		break;
								
		case APP_READ_SENSOR:
			// Bat state done. Save value
			//TransmitData.data[BAT_INDEX]=adcData;	
			// green led blinking	
			PORTB ^= (1 << PB4); 	
			read_sensor_data();
			appstate=APP_READ_CALIB;	
			SYS_PostTask(APL_TASK_ID);	
		break;	
		
		case APP_READ_CALIB:		
			read_calibration_data();		
			appstate=APP_TRANSMIT;	
			SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_TRANSMIT:
		
			APS_DataReq(&dataReq);
		break;
		
		case APP_NOTHING:
		
		break;
	}
}



/* ZIGBEE and Network */
 
static void initEndpoint(void){
	simpleDescriptor.AppDeviceId = 1;
	simpleDescriptor.AppProfileId = 1;
	simpleDescriptor.endpoint = 1;
	simpleDescriptor.AppDeviceVersion = 1;
	endPoint.simpleDescriptor= &simpleDescriptor;
	endPoint.APS_DataInd = APS_DataInd;
	APS_RegisterEndpointReq(&endPoint);
}


static void set_external_crystal(){	u8 sys_trigger;	_bno055_i2c_bus_read(0x3F,sys_trigger,1);	sys_trigger |= 1UL << 7;	_bno055_i2c_bus_write(0x3F,sys_trigger,1);}


// coordinator received data
void APS_DataInd(APS_DataInd_t *indData)
{
 
 // red LED blinking at coordinator
 PORTB ^= (1 << PB7);
 
	
}
 
static void initTransmitData(void){
	dataReq.profileId=1;
	dataReq.dstAddrMode =APS_SHORT_ADDRESS;
	dataReq.dstAddress.shortAddress= CPU_TO_LE16(0);
	dataReq.dstEndpoint =1;
	dataReq.asdu=TransmitData.data;
	dataReq.asduLength=sizeof(TransmitData.data);
	dataReq.srcEndpoint = 1;
	dataReq.APS_DataConf=APS_DataConf;
}

static void APS_DataConf(APS_DataConf_t *confInfo){
	if (confInfo->status == APS_SUCCESS_STATUS){		
		appstate=APP_READ_BAT;
		SYS_PostTask(APL_TASK_ID);
	}
	appstate=APP_READ_BAT;
	SYS_PostTask(APL_TASK_ID);
}



void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo){
	if (ZDO_SUCCESS_STATUS == confirmInfo->status){
		CS_ReadParameter(CS_DEVICE_TYPE_ID,&deviceType);
		
		}else{
		//appWriteDataToUsart((uint8_t*)"Error\r\n",sizeof("Error\r\n")-1);
	}
	SYS_PostTask(APL_TASK_ID);
}

void ZDO_MgmtNwkUpdateNotf(ZDO_MgmtNwkUpdateNotf_t *nwkP){nwkP = nwkP;}
	
void ZDO_WakeUpInd(void){}
	
#ifdef _BINDING_
void ZDO_BindIndication(ZDO_BindInd_t *bindI){(void)bindI;}
void ZDO_UnbindIndication(ZDO_UnbindInd_t *unbindI){(void)unbindI;}
#endif //_BINDING_

/**********************************************************************//**
  \brief The entry point of the program. This function should not be
  changed by the user without necessity and must always include an
  invocation of the SYS_SysInit() function and an infinite loop with
  SYS_RunTask() function called on each step.

  \return none
**************************************************************************/
int main(void)
{
  //Initialization of the System Environment
  SYS_SysInit();
  

  //The infinite loop maintaing task management
  for(;;)
  {
    //Each time this function is called, the task
    //scheduler processes the next task posted by one
    //of the BitCloud components or the application
    SYS_RunTask();
  }
}

//eof app.c
