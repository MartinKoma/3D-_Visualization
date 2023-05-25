/**************************************************************************//**
  \file  halSleepTimerClock.h

  \brief Definition for count out requested sleep interval.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/06/07 E. Ivanov - Created
       7/04/09 A. Khromykh - Refactored
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALSLEEPTIMERCLOCK_H
#define _HALSLEEPTIMERCLOCK_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define SLEEPTIMER_CLOCK 32768lu

#if defined(SLEEP_PRESCALER_1)
  #define SLEEPTIMER_DIVIDER                  1ul
  #define SLEEPTIMER_PRESCALER               (1u << CS20)                               // No prescaling
#elif defined(SLEEP_PRESCALER_8)
  #define SLEEPTIMER_DIVIDER                  8ul
  #define SLEEPTIMER_PRESCALER               (1u << CS21)                               // clk/8
#elif defined(SLEEP_PRESCALER_32)
  #define SLEEPTIMER_DIVIDER                  32ul
  #define SLEEPTIMER_PRESCALER               ((1u << CS20) | (1u << CS21))               // clk/32
#elif defined(SLEEP_PRESCALER_64)
  #define SLEEPTIMER_DIVIDER                  64ul
  #define SLEEPTIMER_PRESCALER               (1u << CS22)                                // clk/64
#elif defined(SLEEP_PRESCALER_128)
  #define SLEEPTIMER_DIVIDER                  128ul
  #define SLEEPTIMER_PRESCALER               ((1u << CS20) | (1u << CS22))               // clk/128
#elif defined(SLEEP_PRESCALER_256)
  #define SLEEPTIMER_DIVIDER                  256ul
  #define SLEEPTIMER_PRESCALER               ((1u << CS21) | (1u << CS22))                // clk/256
#elif defined(SLEEP_PRESCALER_1024)
  #define SLEEPTIMER_DIVIDER                  1024ul
  #define SLEEPTIMER_PRESCALER               ((1u << CS20) | (1u << CS21) | (1u << CS22)) // clk/1024
#endif

#define HAL_ASSR_FLAGS ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB))
// to write some value for correct work of the asynchronous timer
#define SOME_VALUE_FOR_SYNCHRONIZATION     0x44

#define HAL_MIN_SLEEP_TIME_ALLOWED          (1000ul * SLEEPTIMER_DIVIDER / SLEEPTIMER_CLOCK)
/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Starts the sleep timer clock.
******************************************************************************/
void halStartSleepTimerClock(void);

/******************************************************************************
Stops the sleep timer clock.
******************************************************************************/
void halStopSleepTimerClock(void);

/******************************************************************************
Sets interval.
Parameters:
  value - contains number of ticks which the timer must count out.
Returns:
  none.
******************************************************************************/
void halSetSleepTimerInterval(uint32_t value);

/******************************************************************************
Returns the sleep timer frequency in Hz.
Parameters:
  none.
Returns:
  the sleep timer frequency in Hz.
******************************************************************************/
uint32_t halSleepTimerFrequency(void);

/**************************************************************************//**
\brief Clear timer control structure
******************************************************************************/
void halClearTimeControl(void);

/**************************************************************************//**
\brief Wake up procedure for all external interrupts
******************************************************************************/
void halWakeupFromIrq(void);

/**************************************************************************//**
\brief Get time of sleep timer.

\return
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfSleepTimer(void);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/******************************************************************************
Disables the sleep timer interrupt.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
INLINE void halDisableSleepTimerInt(void)
{
  // Disables 8-bit Timer/Counter2 compare channel A and overflow interrupt
  TIMSK2 &= (~(1 << OCIE2A) & ~(1 << TOIE2));
}

/******************************************************************************
  Interrupt handler signal implementation
******************************************************************************/
INLINE void halInterruptSleepClock(void)
{
  halPostTask(HAL_ASYNC_TIMER);
}

/******************************************************************************
  Interrupt handler signal implementation
******************************************************************************/
INLINE void halSynchronizeSleepTime(void)
{
  halPostTask(HAL_SYNC_SLEEP_TIME);
}

#endif /* _HALSLEEPTIMERCLOCK_H */

// eof halSleepTimerClock.h
