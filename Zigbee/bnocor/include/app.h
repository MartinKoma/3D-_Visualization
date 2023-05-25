/**************************************************************************//**
  \file app.h

  \brief Usart-Anwendung Headerdatei.

  \author
    Markus Krauﬂe

******************************************************************************/

#ifndef _APP_H
#define _APP_H

#define APP_SENDE_INTERVAL    1000

typedef enum{
	APP_INIT,
	APP_STARTJOIN_NETWORK,
	APP_TRANSMIT,
	APP_INIT_ENDPOINT,
	APP_INIT_TRANSMITDATA,
	APP_NOTHING
} AppState_t;

#define DEVICE_ID_INDEX 0
#define GYRO_INDEX 7
#define ACCEL_INDEX 1
#define MAG_INDEX 13
#define QUAT_INDEX 19
#define CALIB_INDEX 27
#define BAT_INDEX 30
#define MAX_INDEX BAT_INDEX



#endif
// eof app.h