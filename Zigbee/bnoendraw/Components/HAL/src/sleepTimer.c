/**************************************************************************//**
  \file   sleepTimer.c

  \brief  The implementation of the sleep timer.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/05/07 E. Ivanov - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sleepTimer.h>
#include <appTimer.h>
#include <halSleepTimerClock.h>
#include <halSleep.h>
#include <halDbg.h>
#include <sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NULL_POINTER                      -1
#define HAL_TIME_CAN_NOT_BE_COUNTED           -2
#define HAL_SLEEP_TIMER_HAS_ALREADY_STARTED   -3
#define HAL_SLEEP_TIMER_HAS_ALREADY_STOPPED   -1

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if !defined(HAL_USE_ASYNC_TIMER)
void sleepTimerFired(void);
#endif

/******************************************************************************
                   Implementations section
******************************************************************************/
#if defined(HAL_USE_ASYNC_TIMER)
/******************************************************************************
Starts sleep timer. Interval must be greater one tick time.
  Parameters:
    sleepTimer - address of the HAL_SleepTimer_t.
  Returns:
    -1 - NULL pointer, \n
    -2 - interval can not be counted out, \n
    -3 - sleep timer has already started. \n
     0 - otherwise.
******************************************************************************/
int HAL_StartSleepTimer(HAL_SleepTimer_t *sleepTimer)
{
  uint32_t tempValue;

  if (!sleepTimer)
    return HAL_NULL_POINTER;

  // Convert millisecond interval to the sleep timer ticks.
  tempValue = (halSleepTimerFrequency() * sleepTimer->interval) / 1000ul;
  if (!tempValue)
    return HAL_TIME_CAN_NOT_BE_COUNTED;// Can't count out interval

  if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState) // there is active timer
    return HAL_SLEEP_TIMER_HAS_ALREADY_STARTED;

  // Start asynchronous timer2.
  halSetSleepTimerInterval(tempValue);
  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STARTED;
  halSleepControl.sleepTimer = *sleepTimer;

  return 0;
}// end sleepTimer_start

/******************************************************************************
Removes timer.
Parameters:
  sleepTimer - is not used now. For capabilities for old version.
Returns:
  -1 - there is no active sleep timer.
   0 - otherwise.
******************************************************************************/
int HAL_StopSleepTimer(HAL_SleepTimer_t *sleepTimer)
{
  (void)sleepTimer;

  // there is no active timer
  if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
    return HAL_SLEEP_TIMER_HAS_ALREADY_STOPPED;
  halClearTimeControl();
  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;

  return 0;
}

/******************************************************************************
Interrupt handler about sleep interval was completed.
******************************************************************************/
void halAsyncTimerHandler(void)
{
  // there isn't work timer
  if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
    return;

  if (TIMER_REPEAT_MODE == halSleepControl.sleepTimer.mode)
  {
    SYS_E_ASSERT_FATAL(halSleepControl.sleepTimer.callback, SLEEPTIMER_NULLCALLBACK_0);
    halSleepControl.sleepTimer.callback();

    // user can stop timer in callback
    if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
      return;

    halSetSleepTimerInterval(halSleepControl.sleepTimer.interval);
  }
  else
  {
    if (HAL_ACTIVE_MODE == halSleepControl.wakeupStation)
     halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
    if (halSleepControl.sleepTimer.callback)
      halSleepControl.sleepTimer.callback();
  }
}

#else // HAL_USE_ASYNC_TIMER

/**************************************************************************//**
\brief Dummy function for compatibility with end device mode
\param[in]
  sleepTimer - pointer to sleep timer structure.
\return
   0 - always.
******************************************************************************/
int HAL_StartSleepTimer(HAL_SleepTimer_t *sleepTimer)
{
  if (!sleepTimer)
    return HAL_NULL_POINTER;

  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STARTED;
  halSleepControl.sleepTimer = *sleepTimer;
  halSleepControl.sleepTimer.callback = sleepTimerFired;
  HAL_StartAppTimer((HAL_AppTimer_t *)&halSleepControl.sleepTimer);
  return 0;
}

/**************************************************************************//**
\brief Stops sleep timer.
\param[in]
  sleepTimer - is not used now. For capabilities for old version.
\return
   0 - always.
******************************************************************************/
int HAL_StopSleepTimer(HAL_SleepTimer_t *sleepTimer)
{
  (void)sleepTimer;

  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
  HAL_StopAppTimer((HAL_AppTimer_t *)&halSleepControl.sleepTimer);
  return 0;
}

/**************************************************************************//**
\brief Sleep timer has fired.
******************************************************************************/
void sleepTimerFired(void)
{
  if (halSleepControl.callback)
    halSleepControl.callback(halSleepControl.startPoll);
}

#endif // HAL_USE_ASYNC_TIMER
//eof sleepTimer.c
