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
#include <util/delay.h>
#include <bno055_port.h>
//#include <math.h>
#include <stdio.h>
#include <adc.h>

HAL_AppTimer_t readHalTimer;
static void readHalTimerFired(void);

static void readHalTimerFired(void){
	//appstate=APP_NOTHING;	
	SYS_PostTask(APL_TASK_ID);
}

u8 mystate=0;
static bool xtwibusy=false;
static void __sensorReadData(bool result)
{
	xtwibusy=false;
	if (mystate==0)
	{
	
	appstate=APP_I2C_DONE;
	
	} else appstate=APP_NOTHING;
	SYS_PostTask(APL_TASK_ID);
	mystate++;
	
	
}

static HAL_I2cDescriptor_t xp_packet={
	.tty = TWI_CHANNEL_0,
	.clockRate = I2C_CLOCK_RATE_62,
	.f = __sensorReadData,
	.id = BNO055_I2C_ADDR1
};

s8 x_bno055_i2c_bus_write(u8 reg_addr, u8 *reg_data, u8 cnt);
s8 x_bno055_i2c_bus_read(u8 reg_addr, u8 *reg_data, u8 cnt);

// All we need for the bno-BitCloud-Interface

int __bno055_initialize(void);
s8 __bno055_i2c_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 __bno055_i2c_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void __bno055_delay_msek(u32 msek);


u8 xpage_id=0;
u8 xchip_id=0;

/*!
 *	@brief
 *	This function is used for initialize
 *	read page id and chip id for proof of function
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 * 
 */
s8 __bno055_init()
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;
	u8 v_data_u8 = 0;
	u8 v_page_zero_u8 = 0;
	
	/* Write the default page as zero*/
	com_rslt = x_bno055_i2c_bus_write(0X07, &v_page_zero_u8, 1);
	/* Read the chip id of the sensor from page
	zero 0x00 register*/
	//com_rslt += x_bno055_i2c_bus_read(0x00, &xchip_id, 1);
	//xchip_id = v_data_u8;
	
	/* Read the page id from the register 0x07*/
	//com_rslt += x_bno055_i2c_bus_read (0X07, &v_data_u8, 1);
	//xpage_id = v_data_u8;

	return com_rslt;
}


// Helper-functions
s8 x_bno055_i2c_bus_write(u8 reg_addr, u8 *reg_data, u8 cnt)
{
	return __bno055_i2c_bus_write(0,reg_addr,reg_data,cnt);
}

s8 x_bno055_i2c_bus_read(u8 reg_addr, u8 *reg_data, u8 cnt)
{
	return __bno055_i2c_bus_read(0,reg_addr,reg_data,cnt);
}



int __bno055_initialize(void)
{
	
	readHalTimer.interval= 100; // Werte alle 20 ms auslesen und anschließend senden
	readHalTimer.mode=TIMER_REPEAT_MODE;
	readHalTimer.callback=readHalTimerFired;
	HAL_StartAppTimer(&readHalTimer);
	
	//p_packet.f=&f;
	xp_packet.tty = TWI_CHANNEL_0;
	xp_packet.clockRate = I2C_CLOCK_RATE_62; 
	//xp_packet.f = __sensorReadData; 
	xp_packet.id = BNO055_I2C_ADDR1;	
	
		
	HAL_OpenI2cPacket(&xp_packet);
	
	
	/*bno055.bus_write =bno055_i2c_bus_write;
	bno055.bus_read = bno055_i2c_bus_read;
	bno055.delay_msec =bno055_delay_msek;	*/
	//bno055_init(&bno055);
	__bno055_init();
	// if chip.id is set, sensor is ready
	//if (bno055.chip_id>0)
	if (xchip_id>0)
	return 0;
	return -1;
}


/*	\Brief: The function is used as I2C bus write. With BitCloud HAL structures and functions
 *	\Return : Status of the I2C write
 *	\param dev_addr : The device address of the sensor: Set in bno055_initialize
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
s8 __bno055_i2c_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	int32_t ierror = 0;
   
    // if last state was I2C_TRANSAC_FAIL something went wrong
	if (xp_packet.service.state == 7) //I2C_TRANSAC_FAIL
	return -1;
	
	// if we are still busy, 
	if (xtwibusy) return -1;
	
	
	dev_addr = dev_addr; // not needed. Just for unused param message
	xp_packet.internalAddr=reg_addr;	
	xp_packet.lengthAddr = HAL_ONE_BYTE_SIZE;
	xp_packet.data = reg_data;
	xp_packet.length = cnt;
	xp_packet.service.state = 1; //I2C_IDLE
	
	xtwibusy=true;
	
	ierror=HAL_WriteI2cPacket (&xp_packet); 	
//	SYS_PostTask(APL_TASK_ID);
	return (int8_t)ierror;
	if (ierror==0)
	{
		
		while (xtwibusy) // wait, until I2C is ready and state is I2C_TRANSAC_SUCCESS
		{
			// Das ist wichtig! Nur so werden im Hintergrund die i2c-Befehle abgearbeitet						
			 //halPostTask(HAL_TWI);
			 
			 if (xp_packet.service.state == 6) //I2C_TRANSAC_SUCCESS -> Siehe i2cPacket.c
			 xp_packet.f(true); // call callback
			 
			 if (xp_packet.service.state == 7)
			 xp_packet.f(false); // call callback
			
		}
	}
		
		
	if (xp_packet.service.state == 7) 
	{		
		return -1;
	}
	
	return (int8_t)ierror;
}

 /*	\Brief: The function is used as I2C bus read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of byte of data to be read
 */
s8 __bno055_i2c_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	int32_t ierror = 0;
	if (xp_packet.service.state == 7) //I2C_TRANSAC_FAIL
	return -1;
	
	// if we are still busy, error ...
	if (xtwibusy) return -1;
		
	dev_addr = dev_addr; // not needed. Just for unused param message
	xp_packet.internalAddr=reg_addr;
	xp_packet.lengthAddr = HAL_ONE_BYTE_SIZE;
	xp_packet.data = reg_data;
	xp_packet.length = cnt;
	xp_packet.service.state = 1; //I2C_IDLE

	xtwibusy=true;	
	ierror=HAL_ReadI2cPacket (&xp_packet);
	//SYS_PostTask(APL_TASK_ID);
	return (int8_t)ierror;
	if (ierror==0)
	while (xtwibusy) 
	{	
		 
		if (xp_packet.service.state == 6) //I2C_TRANSAC_SUCCESS -> Siehe i2cPacket.c	
			xp_packet.f(true); // call callback
		 
		if (xp_packet.service.state == 7)
			xp_packet.f(false); // call callback
	}

	if (xp_packet.service.state == 7) 
	{
		return -1;
	}
	
	return (int8_t)ierror;
}


/*	Brief : The delay routine
 *	\param : delay in ms
*/
// Wird von der Sensor-Lib aufgerufenb mit msek zwischen 10 und 500.
// Z,B. in bno055_set_operation_mode -> dort 600 ms delay nötig
void __bno055_delay_msek(u32 msek)
{
	for (u32 i=0; i<msek; i++)
      _delay_us(1000);
}


/********************** Batterieüberwachung ******************************* */
uint8_t adcData=0;

static void readSensorDoneCb(void)
{	
	appstate=APP_READ_CALIB;	
	SYS_PostTask(APL_TASK_ID);
}

static HAL_AdcDescriptor_t adcdescriptor = {
	.resolution = RESOLUTION_8_BIT,
	.sampleRate = ADC_4800SPS,
	.voltageReference = AVCC,
	.bufferPointer = &adcData,
	.selectionsAmount = 1,
	.callback = readSensorDoneCb
};

/* ************************* ZIGBEE **************************** */
static uint8_t deviceType;
static void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo);
static ZDO_StartNetworkReq_t networkParams;


BEGIN_PACK
typedef struct _AppMessage_t{
	uint8_t header[APS_ASDU_OFFSET]; //APS header
	uint8_t data[24];
	uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET]; // Footer
} PACK AppMessage_t;
END_PACK

// Struct for Data-Transmission. Sensor-Data from read_sensor
static AppMessage_t TransmitData;

APS_DataReq_t dataReq;

static void APS_DataConf(APS_DataConf_t *confInfo);
static void initTransmitData(void);
static SimpleDescriptor_t simpleDescriptor;
static APS_RegisterEndpointReq_t endPoint;
static void initEndpoint(void);
void APS_DataInd(APS_DataInd_t *indData);


/* ************************* SENSOR  *********************************** */




HAL_AppTimer_t readSensorTimer;
static void readSensorTimerFired(void);


static void sensor_init(void)
{
    
	if (bno055_initialize()==0)
	{
	
	bno055_set_power_mode(POWER_MODE_NORMAL);	
	
	
	// Non fusion data only ACC and GYRO		
	// We need non fusion data
	// Fusion Data: Quaternion, Euler angles, Rotation vector,	Linear acceleration, Gravity, Heading
	//bno055_set_operation_mode(OPERATION_MODE_NDOF);	
	bno055_set_operation_mode(OPERATION_MODE_NDOF); //OPERATION_MODE_ACCGYRO);
	
	PORTB |= (1 << PB4);
	
	readSensorTimer.interval= 20; // Werte alle 20 ms auslesen und anschließend senden
	readSensorTimer.mode=TIMER_REPEAT_MODE;
	readSensorTimer.callback=readSensorTimerFired;
	HAL_StartAppTimer(&readSensorTimer);
	
	} 
}

u8 bno055_quat_data[8];
// Read Sensor Data from bno055 and save to TransmitData
static void read_sensor_data (void)
{
	
	
	u8 bno055_accel_data[6];
	u8 bno055_gyro_data[6];
	//u8 bno055_quat_data[8];
	
	bno055_read_accel_xyz_raw(bno055_accel_data);	
	bno055_read_gyro_xyz_raw(bno055_gyro_data);	
	// for tests and control	
	bno055_read_quaternion_wxyz_raw(bno055_quat_data);
	
	// Put bno055 data in TransmitData
	// gyro 6 byte
	for (int i=0; i<6; i++)
	TransmitData.data[i]=bno055_gyro_data[i];
	
	// accel 6 byte
	for (int i=6; i<12; i++)
	TransmitData.data[i]=bno055_accel_data[i-6];
	
	// quat 8 byte: if needed. All 0 if OPERATION_MODE_ACCGYRO
	for (int i=12; i<20; i++)
	TransmitData.data[i]=bno055_quat_data[i-12];
	
	// Bat state
	TransmitData.data[20]=adcData;
		
}


// Read Calibration status
static void read_calibration_data (void)
{
	
	// read calibration status and turn off red, yellow led, if =3 
	u8 v_gyro_calib_u8=0;
	u8 v_accel_calib_u8=0;	
	bno055_get_gyro_calib_stat(&v_gyro_calib_u8);
	bno055_get_accel_calib_stat(&v_accel_calib_u8);
	if (v_gyro_calib_u8!=3) PORTB |= (1 << PB6);
	else PORTB &= ~(1 << PB6);
	if (v_accel_calib_u8!=3) PORTB |= (1 << PB7);
	else PORTB &= ~(1 << PB7);
	
	// transmit calib-state?
	TransmitData.data[21]=v_gyro_calib_u8;	
	TransmitData.data[22]=v_accel_calib_u8;	
}

// Timer-Callback to handle Read Sensor
static void readSensorTimerFired(void){
	//appstate=APP_NOTHING;
	if (appstate==APP_WAIT_FOR_READ)	
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
		#else
		PORTB |= (1 << PB7);
		PORTB |= (1 << PB6);
		#endif
		PORTB &= ~(1 << PB4);	
		HAL_OpenAdc(&adcdescriptor);	
		appInitUsartManager();		
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
			appstate=APP_NOTHING;
		#else
			appstate=APP_INIT_SENSOR;
		#endif
		
		SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_INIT_SENSOR:		
		initTransmitData();
		//sensor_init();	
		__bno055_initialize();
		appstate=APP_WAIT_FOR_READ;			
		SYS_PostTask(APL_TASK_ID);
		break;
		
		case APP_WAIT_FOR_READ:
		// wait for next Read-Event: for timer 	
		break;
						
		case APP_READ_SENSOR:
		PORTB ^= (1 << PB4); 	
		read_sensor_data();
		appstate=APP_READ_BAT;	
		SYS_PostTask(APL_TASK_ID);	
		break;			
		
		case APP_READ_BAT:		
		HAL_ReadAdc(&adcdescriptor, HAL_ADC_CHANNEL0);			
		break;
		
		case APP_I2C_DONE:
			PORTB ^= (1 << PB4); 
			x_bno055_i2c_bus_read(0x00, &xchip_id, 1);
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

double decpart;
// Testausgabe für https://adafruit.github.io/Adafruit_WebSerial_3DModelViewer/
void quatDataToUart(uint8_t* data)
{

   
	float f;
	s16 value16;	
	char arr[50]; 	 
	
	// Ausgabe von quat.w 
	
	
	value16 = (s16)((((s32)	((s8)data[13])) <<	8) | (data[12]));	
	//Auflösung 14 bit = 16384: bno055 hat 14 bit Auflösung für quats. Der Wert muss daher durch 2^14 geteilt werden.	
	f=(float) value16 / 16384.0;	  
	
	// Integerpart
	int intpart = ((int)f);
	decpart = (f) - (intpart);
	//Nachkommapart
	int dec = (int)(decpart*1000);  
	
	//sprintf auf ATmega kennt %f für double und float-Umwandlung nicht. Deshalb %d usw
	if (f<0)
	sprintf(arr,"Quaternion: -%d.%03d",abs(intpart), abs(dec));	 
	else
	sprintf(arr,"Quaternion: %d.%03d",intpart, abs(dec));	 
	
	appWriteDataToUsart(arr, strlen(arr));
	
	// Ausgabe von quat.x 	
	 value16 = (s16)((((s32)	((s8)data[15])) <<	8) | (data[14]));
	 f=(float) value16 / 16384.0;	 
	 intpart = ((int)f);
	 decpart = (f) - (intpart);
	 dec = (int)(decpart*1000);
	 if (f<0)
	 sprintf(arr,", -%d.%03d",abs(intpart), abs(dec));	 
	 else
	 sprintf(arr,", %d.%03d",abs(intpart), abs(dec));	 
	 appWriteDataToUsart(arr, strlen(arr));
	 
	 // Ausgabe von quat.y 
	 value16 = (s16)((((s32)	((s8)data[17])) <<	8) | (data[16]));
	 f=(float) value16 / 16384.0;
	 intpart = ((int)f);
	 decpart = (f) - (intpart);
	 dec = (int)(decpart*1000);
	 if (f<0)
	 sprintf(arr,", -%d.%03d",abs(intpart), abs(dec));
	 else
	 sprintf(arr,", %d.%03d",abs(intpart), abs(dec));
	 appWriteDataToUsart(arr, strlen(arr));
	 
	 // Ausgabe von quat.z 
	 value16 = (s16)((((s32)	((s8)data[19])) <<	8) | (data[18]));
	 f=(float) value16 / 16384.0;
	 intpart = ((int)f);
	 decpart = (f) - (intpart);
	 dec = (int)(decpart*1000);
	 if (f<0)
	 sprintf(arr,", -%d.%03d",abs(intpart), abs(dec));
	 else
	 sprintf(arr,", %d.%03d",abs(intpart), abs(dec));
	 appWriteDataToUsart(arr, strlen(arr));
	 appWriteDataToUsart((uint8_t*)"\r\n",2);
	 
	 
}

// Koordinator empfängt Daten und sendet diese mittels UART an den PC
void APS_DataInd(APS_DataInd_t *indData){
 
 PORTB ^= (1 << PB7);
 
 // Hier zu Testzwecken: Für adafruit 3d Webviewer
 quatDataToUart(indData->asdu);
 /*
 uint8_t temp[]="                                                                 ";
 
 for (int i=0; i<6; i++)
 uint8_to_hexstr (temp, 50, indData->asdu[i] ,i*2);
 appWriteDataToUsart(temp, 44);
  appWriteDataToUsart((uint8_t*)"\r\n",2);
 */
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
		appstate=APP_WAIT_FOR_READ;
		SYS_PostTask(APL_TASK_ID);
	}
	appstate=APP_WAIT_FOR_READ;
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

