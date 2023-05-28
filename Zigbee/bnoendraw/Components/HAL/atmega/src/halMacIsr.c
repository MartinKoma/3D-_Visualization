/**************************************************************************//**
  \file  halMacIsr.c

  \brief mac interrupts implementation.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      14/01/08 A. Mandychev - Created.
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                        Includes section.
******************************************************************************/
#include <halMacIsr.h>
#include <atomic.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(HAL_3d6864MHz)
  #define HAL_RTIMER_INTERVAL_CALCULATE(period)    (period >> 1)
#elif defined(HAL_4MHz)
  #define HAL_RTIMER_INTERVAL_CALCULATE(period)    (period >> 1)
#elif defined(HAL_7d3728MHz)
  #define HAL_RTIMER_INTERVAL_CALCULATE(period)    (period)
#elif defined(HAL_8MHz)
  #define HAL_RTIMER_INTERVAL_CALCULATE(period)    (period)
#elif defined(HAL_16MHz)
  #define HAL_RTIMER_INTERVAL_CALCULATE(period)    ((uint32_t)period << 1)
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
RTimerDescr_t __rtimer;

/******************************************************************************
                    Prototypes section
******************************************************************************/
/******************************************************************************
  Initializes Rtimer.
******************************************************************************/
void HAL_InitMacIsr(void);

/******************************************************************************
  Redirect interrupt event depending on the TrxState.
  Parameters: none.
  Returns: none.
******************************************************************************/
void phyDispatcheRTimerEvent(void);

/******************************************************************************
  Redirect interrupt event depending on the TrxState.
  Parameters: none.
  Returns: none.
******************************************************************************/
void phyDispatcheRfInterrupt(void);
void phyRxEndInterrupt(void);
void phyTxEndInterrupt(void);
void phyBatLowInterrupt(void);

/******************************************************************************
                    Implementations section
******************************************************************************/
/******************************************************************************
  Initializes Rtimer.
******************************************************************************/
void HAL_InitMacIsr(void)
{
  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
}

/******************************************************************************
  Starts RTimer. Function should be invoked in critical section.
  Parameters:
    source  - source of invocation.
    mode    - RTimer mode.
    period  - RTimer period.
******************************************************************************/
bool HAL_StartRtimer(HAL_RTimerMode_t mode, uint16_t period)
{
  SYS_ASSERT(HAL_RTIMER_STOPPED_MODE == __rtimer.mode, HALMACISR_RTIMER_ALREADY_STARTED);

  __rtimer.period    = (uint16_t)HAL_RTIMER_INTERVAL_CALCULATE(period);
  __rtimer.mode      = mode;
  __rtimer.nextEvent = TCNT4 + __rtimer.period;
  if (__rtimer.nextEvent > TOP_TIMER_COUNTER_VALUE)
    __rtimer.nextEvent -= TOP_TIMER_COUNTER_VALUE;
  OCR4B = __rtimer.nextEvent;
  // clear possible interrupt by setting logical one.
  TIFR4 = (1 << OCF4B);
  // enable interrupt
  TIMSK4 |= (1 << OCIE4B);
  return true;
}

/******************************************************************************
  Stops RTimer. Function should be invoked in critical section.
******************************************************************************/
void HAL_StopRtimer(void)
{
  // clear possible interrupt
  TIFR4 &= ~(1 << OCF4B);
  // disable interrupt
  TIMSK4 &= ~(1 << OCIE4B);
  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
}

/******************************************************************************
  Returns status of RTimer
******************************************************************************/
bool HAL_IsRtimerStopped(void)
{
  if (__rtimer.mode == HAL_RTIMER_STOPPED_MODE)
    return true;
  else
    return false;
}

/******************************************************************************
  Output compare unit (channel B) interrupt handler.
******************************************************************************/
ISR(TIMER4_COMPB_vect)
{
  BEGIN_MEASURE
  if (HAL_RTIMER_ONE_SHOT_MODE == __rtimer.mode)
  {
    TIMSK4 &= ~(1 << OCIE4B);
    __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
  }
  else
  {
    __rtimer.nextEvent += __rtimer.period;
    if (__rtimer.nextEvent > TOP_TIMER_COUNTER_VALUE)
      __rtimer.nextEvent -= TOP_TIMER_COUNTER_VALUE;
    OCR4B = __rtimer.nextEvent;
  }
  phyDispatcheRTimerEvent();
  END_MEASURE(HALISR_TIMER4_COMPB_MEASURE_CODE)
}

/****************************************************************
  Rx End interrupt service routine.
****************************************************************/
ISR(TRX24_RX_END_vect)
{
  BEGIN_MEASURE
  phyRxEndInterrupt();
  END_MEASURE(HALMACISR_TRX24_RX_END_MEASURE_CODE)
}

/****************************************************************
  Rx End interrupt service routine.
****************************************************************/
ISR(TRX24_TX_END_vect)
{
  BEGIN_MEASURE
  phyTxEndInterrupt();
  END_MEASURE(HALMACISR_TRX24_TX_END_MEASURE_CODE)
}

/****************************************************************
  Battery Low interrupt service routine.
****************************************************************/

#ifdef _RF_BAT_MON_
ISR(BAT_LOW_vect)
{
  BEGIN_MEASURE
  phyBatLowInterrupt();
  END_MEASURE(HALMACISR_BAT_LOW_MEASURE_CODE)
}
#endif // _RF_BAT_MON_

// eof halMacIsr.c
