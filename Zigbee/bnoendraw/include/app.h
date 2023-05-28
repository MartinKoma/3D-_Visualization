/**************************************************************************//**
  \file app.h

  \brief Usart-Anwendung Headerdatei.

  \author
    Markus Krauﬂe

******************************************************************************/


#ifndef _APP_H
#define _APP_H

#include "stdint.h"

typedef enum{
	APP_INIT,
	APP_STARTJOIN_NETWORK,
	APP_INIT_ENDPOINT,
	APP_RESET_SENSOR,
	APP_INIT_SENSOR,		
	APP_READ_BAT,
	APP_READ_SENSOR,	
	APP_READ_CALIB,	
	APP_TRANSMIT,
	APP_NOTHING	
} AppState_twi;

// start index for transmission data array       1 7 13 19
#define DEVICE_ID_INDEX 0
#define GYRO_INDEX 7
#define ACCEL_INDEX 1            
#define MAG_INDEX 13
#define QUAT_INDEX 19
#define CALIB_INDEX 27
#define BAT_INDEX 30
#define MAX_INDEX BAT_INDEX


static AppState_twi appstate = APP_INIT;

#endif
// eof app.h