/**************************************************************************//**
  \file  halAppClock.h

  \brief Declarations of appTimer hardware-dependent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      5/12/07 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALAPPCLOCK_H
#define _HALAPPCLOCK_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>
#include <halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/** \brief system timer interval in ms */
#define HAL_APPTIMERINTERVAL 10ul
/** \brief frequency prescaler for system timer */
#define TIMER_FREQUENCY_PRESCALER  8
/** \brief timer counter top value */
#define TOP_TIMER_COUNTER_VALUE  ((F_CPU/1000ul) / TIMER_FREQUENCY_PRESCALER) * HAL_APPTIMERINTERVAL
/** \brief cpu clk / 8 */
#define HAL_CLOCK_SELECTION_MASK      (1 << CS11)

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initialization appTimer clock.
******************************************************************************/
void halInitAppClock(void);

/**************************************************************************//**
\brief Synchronization system time which based on application timer.
******************************************************************************/
void halAppSystemTimeSynchronize(void);

/**************************************************************************//**
\brief Return time of sleep timer.

\return
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfAppTimer(void);

/**************************************************************************//**
\brief Return system time in us

\param[out]
  mem - memory for system time
******************************************************************************/
void halGetSystemTimeUs(uint64_t *mem);

/**************************************************************************//**
\brief Takes account of the sleep interval.

\param[in]
  interval - time of sleep
******************************************************************************/
void halAdjustSleepInterval(uint32_t interval);

/******************************************************************************
                 Inline static functions prototypes section.
******************************************************************************/
/**************************************************************************//**
\brief Enables appTimer clock.
******************************************************************************/
INLINE void halStartAppClock(void)
{
  TCCR4B |= HAL_CLOCK_SELECTION_MASK;
}

/**************************************************************************//**
\brief Disables appTimer clock.
******************************************************************************/
INLINE void halStopAppClock(void)
{
  TCCR4B &= ~HAL_CLOCK_SELECTION_MASK; // stop the timer
}

#endif /*_HALAPPCLOCK_H*/

// eof halAppClock.h
