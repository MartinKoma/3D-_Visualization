/**************************************************************************//**
  \file  calibration.c

  \brief the calibration of the internal RC generator.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/06/07 E. Ivanov - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>
#include <calibration.h>
#include <atomic.h>
#include <halW1.h>

/******************************************************************************
                   Defines section
******************************************************************************/
#define INTERNAL_CLOCK F_CPU

// low frequency oscillator clock for 1 cycle measurement
#define EXTERNAL_TICKS 1
// mcu clocks number of one procedure measurement
#define CYCLE_LENGTH 7
// stability crystal oscillator frequency
#define REFERENCE_CLOCK (32768/1024)
// Etalon mcu clock number for 1 ticks of 32 Hz asynchronous timer
#define REFERENCE_COUNT (INTERNAL_CLOCK * EXTERNAL_TICKS) / (REFERENCE_CLOCK * CYCLE_LENGTH)
// up direction
#define UP_DIRECT      2
// down direction
#define DOWN_DIRECT    1

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Calculates number of cycles during EXTERNAL_TICKS period.
Parameters:
  none
Returns:
  number of the cycles.
******************************************************************************/
uint16_t halMeasurement(void);

/******************************************************************************
                   Implementations section
******************************************************************************/
#if (F_CPU != 16000000) && defined(HAL_USE_ASYNC_TIMER)
/******************************************************************************
Performs calibration of the internal RC generator.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halCalibrateInternalRc(void)
{
  uint16_t count;
  uint8_t cycles = 0x80;
  uint16_t counterGate = REFERENCE_COUNT;
  uint8_t direct = 0;

  do
  {
    // perform clock measurement
    count = halMeasurement();
    if (count > REFERENCE_COUNT)
    {
      if ((counterGate < (count - REFERENCE_COUNT)) && (UP_DIRECT == direct))
      { // previous measurement was more correct
        OSCCAL--;
        NOP;
        break;
      }
      OSCCAL--;
      NOP;
      counterGate = count - REFERENCE_COUNT;
      direct = DOWN_DIRECT;
    }

    if (count < REFERENCE_COUNT)
    {
      if ((counterGate < (REFERENCE_COUNT - count)) && (DOWN_DIRECT == direct))
      { // previous measurement was more exactly
        OSCCAL++;
        NOP;
        break;
      }
      OSCCAL++;
      NOP;
      counterGate = REFERENCE_COUNT - count;
      direct = UP_DIRECT;
    }

    if (REFERENCE_COUNT == count)
      break;

  } while (--cycles);
}

/******************************************************************************
Performs calibration of the main clock generator.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void HAL_CalibrateMainClock(void)
{
  if (INTERNAL_RC == halGetClockSource())
    halCalibrateInternalRc();
}

/******************************************************************************
Starts calibration after program starting or waking up from power down.
******************************************************************************/
void halStartingCalibrate(void)
{
  uint16_t i;

  for (i = 0; i < 5000; i++)
  { /* wait for 1 second start up low frequency generator*/
    __delay_us(200);  // 200 us
  }
  HAL_CalibrateMainClock();
}
#else
/******************************************************************************
Performs calibration of the main clock generator.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void HAL_CalibrateMainClock(void)
{
}

/******************************************************************************
Starts calibration after program starting or waking up from power down.
******************************************************************************/
void halStartingCalibrate(void)
{
}
#endif
// eof calibration.c
