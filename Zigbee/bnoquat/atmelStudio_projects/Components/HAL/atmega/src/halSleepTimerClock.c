/**************************************************************************//**
  \file  halSleepTimer.c

  \brief Module for count out requested sleep interval.

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
/******************************************************************************
                   Includes section
******************************************************************************/
#include <sleepTimer.h>
#include <halSleepTimerClock.h>
#include <halSleep.h>
#include <halDbg.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define LSB_IN_DWORD(A)       ((uint32_t)A & 0x000000FF)
#define MULTIPLY_ON_31d25(A)  (((uint32_t)A << 5) - (uint32_t)A + ((uint32_t)A >> 2))
#define MAX_TIMER_VALUE       0xFF
#define SLEEP_TIMER_ITERATOR  (1000ul * 256ul * SLEEPTIMER_DIVIDER / SLEEPTIMER_CLOCK)

#if defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
  #define HAL_SLEEP_TIMER_PERIOD           7000ul
  #define HAL_SLEEP_TIMER_PERIOD_IN_TICKS  (HAL_SLEEP_TIMER_PERIOD * SLEEPTIMER_CLOCK / SLEEPTIMER_DIVIDER / 1000ul)

  #if (HAL_SLEEP_TIMER_PERIOD >= SLEEP_TIMER_ITERATOR)
    #error Nonstandard sleep timer reload period shall be least then the overflow time!
  #endif
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct
{
  // Contains number of ticks that passed before timer firing.
  volatile uint32_t remainder;
#if !defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
  // Contains number of timer full intervals before reminder will be loaded.
  volatile uint32_t ovfwCount;
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */
} TimerControl_t;

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;

#if defined(HAL_USE_ASYNC_TIMER)
/******************************************************************************
                   Global variables section
******************************************************************************/
static volatile TimerControl_t halTimerControl;
// time of sleep timer in ms.
static uint32_t halSleepTime = 0ul;
// upper byte of sleep time
uint8_t halSleepTimerOvfw = 0;
// interrupt counter
static volatile uint8_t halIrqOvfwCount = 0;

/*****************************************************************************
                  Inline static functions section
******************************************************************************/
#if defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
/**************************************************************************//**
  \brief Calculates the next compare value for sleep timer.
  \param[in]
    currCounter - current value of TCNT2
    syncTimer - request to synchronize timer with a direct call
  \return compare value for loading into OCR2A.
******************************************************************************/
static inline uint8_t calculateNextCompareValue(uint8_t currCounter, bool syncTimer)
{
  uint8_t nextPeriod;
  uint8_t tempValue = LSB_IN_DWORD(~currCounter);

  halIrqOvfwCount = 0;

  // wait for end of synchronization
  while (ASSR & HAL_ASSR_FLAGS);

  if (HAL_SLEEP_TIMER_PERIOD_IN_TICKS < halTimerControl.remainder)
    nextPeriod = HAL_SLEEP_TIMER_PERIOD_IN_TICKS;
  else
  {
    nextPeriod = halTimerControl.remainder;
    halTimerControl.remainder = 0;
  }

  if (nextPeriod > tempValue)
  {
    if (!syncTimer)
    {
      halIrqOvfwCount++;
      halSynchronizeSleepTime();
    }
    return (nextPeriod - tempValue);
  }
  else
  {
    if (syncTimer && (nextPeriod == HAL_SLEEP_TIMER_PERIOD_IN_TICKS))
    {
      halIrqOvfwCount++;
    }
    return (currCounter + nextPeriod);
  }
}
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Clear timer control structure
******************************************************************************/
void halClearTimeControl(void)
{
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    // clear timer control structure
    halTimerControl.remainder = 0;

#if !defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
    halTimerControl.ovfwCount = 0;
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */

  END_MEASURE(HALATOM_CLEAR_TIME_CONTROL_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE
}

/**************************************************************************//**
\brief Wake up procedure for all external interrupts
******************************************************************************/
void halWakeupFromIrq(void)
{
  if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
  {
    halPowerOn(HAL_EXT_IRQ_IS_WAKEUP_SOURCE);
    // enable overflow interrupt
    TIMSK2 |= (1 << TOIE2);
    // disable compare match interrupt
    TIMSK2 &= ~(1 << OCIE2A);
    // clear timer control structure
    halClearTimeControl();
    // stop high sleep timer logic
    halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
  }
}

/******************************************************************************
Starts the sleep timer clock.
******************************************************************************/
void halStartSleepTimerClock(void)
{
  //1. Disable the Timer/Counter2 interrupts by clearing OCIE2x and TOIE2.
  halDisableSleepTimerInt();
  //2. Select clock source by setting AS2 as appropriate.
  ASSR |= (1 << AS2);  // clock source is TOSC1 pin
  //3. Write new values to TCNT2, OCR2x, and TCCR2x.
  TCNT2 = 0;
  TCCR2A = 0x00; // normal operation, OC2A&OC2B disconnected
  TCCR2B = 0x00;
  OCR2A = 0x00;
  //4. To switch to asynchronous operation: Wait for TCN2UB, OCR2xUB, and TCR2xUB.
  while (ASSR & HAL_ASSR_FLAGS);
  //5. Clear the Timer/Counter2 Interrupt Flags.
  TIFR2 = (1 << OCF2A) | (1 << TOV2);
  //6. Enable interrupts, if needed.
  TCCR2B = SLEEPTIMER_PRESCALER; // start timer
  TIMSK2 |= (1 << TOIE2); // enable overflow interrupt
}

/******************************************************************************
Stops the sleep timer clock.
******************************************************************************/
void halStopSleepTimerClock(void)
{
  while (ASSR & HAL_ASSR_FLAGS);
  //1. Disable the Timer/Counter2 interrupts by clearing OCIE2x and TOIE2.
  halDisableSleepTimerInt();
  TCCR2B &= ~SLEEPTIMER_PRESCALER;  // Stops the timer
  GTCCR |= (1 << PSRASY); // Reset prescaler
  while (ASSR & HAL_ASSR_FLAGS);
  // switch of oscillator
  ASSR &= ~(1 << AS2);
}

/******************************************************************************
Sets interval.
Parameters:
  value - contains number of ticks which the timer must count out.
Returns:
  none.
******************************************************************************/
void halSetSleepTimerInterval(uint32_t value)
{
  uint8_t currCounter = TCNT2;

#if defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
  // disable overflow interrupt
  TIMSK2 &= ~(1 << TOIE2);

  halTimerControl.remainder = value;
  OCR2A = calculateNextCompareValue(currCounter, true);

  // clear compare interrupt flag
  TIFR2 = 1 << OCF2A;
  // enable compare match interrupt
  TIMSK2 |= (1 << OCIE2A);
#else
  uint32_t tempValue = LSB_IN_DWORD(~currCounter);

  if (value > tempValue)
  {
    value -= tempValue;
    halTimerControl.ovfwCount = value >> 8;
    halTimerControl.remainder = value & 0xFF;
  }
  else
  { // enough timer reminder before overflow
    currCounter += (uint8_t)value;
    // wait for end of synchronization
    while (ASSR & HAL_ASSR_FLAGS);
    // load compared value
    OCR2A = currCounter;
    // clear compare interrupt flag
    TIFR2 = 1 << OCF2A;
    // enable compare match interrupt
    TIMSK2 |= (1 << OCIE2A);
  }
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */
}

/******************************************************************************
Return time of sleep timer.

Returns:
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfSleepTimer(void)
{
  uint32_t tempValue;
  uint8_t tmpCounter;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
  // read interrupt counter
  tmpCounter = halIrqOvfwCount;
  // read asynchronous counter
  tempValue = TCNT2;
  // wait for setup asynchronous registers
  OCR2B = SOME_VALUE_FOR_SYNCHRONIZATION;
  while (ASSR & HAL_ASSR_FLAGS);
  if (TIFR2 & (1 << TOV2))
  { // there is issued interrupt
    tempValue = TCNT2;
    tempValue += MAX_TIMER_VALUE;
  }
  END_MEASURE(HAL_GET_SLEEP_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  tempValue += tmpCounter * MAX_TIMER_VALUE;

  #if defined(SLEEP_PRESCALER_1024)
    // one tick time 31.25 ms.
    return (halSleepTime + MULTIPLY_ON_31d25(tempValue));
  #else
    #warning 'to do counting sleep timer for that prescaler'
    return (halSleepTime + tempValue * (1000 * SLEEPTIMER_DIVIDER / SLEEPTIMER_CLOCK));
  #endif
}

/******************************************************************************
Returns the sleep timer frequency in Hz.
Parameters:
  none.
Returns:
  frequency.
******************************************************************************/
uint32_t halSleepTimerFrequency(void)
{
  return (SLEEPTIMER_CLOCK / SLEEPTIMER_DIVIDER);
}

/**************************************************************************//**
Synchronization system time which based on sleep timer.
******************************************************************************/
void halSleepSystemTimeSynchronize(void)
{
  uint8_t tmpCounter;
  uint32_t tmpValue;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    tmpCounter = halIrqOvfwCount;
    halIrqOvfwCount = 0;
  END_MEASURE(HALATOM_SLEEP_TIMER_SYNCHRONIZE_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  tmpValue = tmpCounter * SLEEP_TIMER_ITERATOR;
  halSleepTime += tmpValue;
  if (halSleepTime < tmpValue)
    halSleepTimerOvfw++;
}

/******************************************************************************
Compare interrupt handler.
******************************************************************************/
ISR(TIMER2_COMPA_vect)
{
  BEGIN_MEASURE

#if defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
  if (0 != halTimerControl.remainder)
  {
    halTimerControl.remainder -= HAL_SLEEP_TIMER_PERIOD_IN_TICKS;

    if (0 == halTimerControl.remainder)
    {
      // enable overflow interrupt
      TIMSK2 |= (1 << TOIE2);
      // disable compare match interrupt
      TIMSK2 &= ~(1 << OCIE2A);

      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
        halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
      // post task for task manager
      if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
        halInterruptSleepClock();
    }
    else
    {
      OCR2A = calculateNextCompareValue(OCR2A, false);

      // clear compare interrupt flag
      TIFR2 = 1 << OCF2A;
      // enable compare match interrupt
      TIMSK2 |= (1 << OCIE2A);

      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
        HAL_Sleep();
    }
  }
  else
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */
  {
#if defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
    TIMSK2 |= (1 << TOIE2); // enable overflow interrupt
#endif /* HAL_USE_SLEEP_TIMER_RELOAD_PERIOD */
    // disable compare match interrupt
    TIMSK2 &= ~(1 << OCIE2A);
    // nulling for adjusting
    halTimerControl.remainder = 0;
    if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
      halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
    // post task for task manager
    if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
      halInterruptSleepClock();
  }
  END_MEASURE(HALISR_TIMER2_COMPA_MEASURE_CODE)
}

/******************************************************************************
Overflow interrupt handler.
******************************************************************************/
ISR(TIMER2_OVF_vect)
{
  BEGIN_MEASURE
#if !defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD)
  if (0 == halTimerControl.ovfwCount)
  {
    if (0 == halTimerControl.remainder)
    {
      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
        halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
      // post task for task manager
      if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
        halInterruptSleepClock();
    }
    else
    {
      // wait for end of synchronization
      while (ASSR & HAL_ASSR_FLAGS);
      // load compared value
      OCR2A = halTimerControl.remainder;
      // clear compare interrupt flag
      TIFR2 = 1 << OCF2A;
      // enable compare match interrupt
      TIMSK2 |= (1 << OCIE2A);
      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
        HAL_Sleep();
    }
  }
  else
  {
    halTimerControl.ovfwCount--;
    if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
      HAL_Sleep();
  }
#endif /* !defined(HAL_USE_SLEEP_TIMER_RELOAD_PERIOD) */

  halIrqOvfwCount++;
  halSynchronizeSleepTime();

  END_MEASURE(HALISR_TIMER2_OVF_MEASURE_CODE)
}
#else // HAL_USE_ASYNC_TIMER

/******************************************************************************
Starts the sleep timer clock.
******************************************************************************/
void halStartSleepTimerClock(void)
{}

/**************************************************************************//**
\brief Wake up procedure for all external interrupts
******************************************************************************/
void halWakeupFromIrq(void)
{
  if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
  {
    halPowerOn(HAL_EXT_IRQ_IS_WAKEUP_SOURCE);
    // stop high sleep timer logic
    halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
  }
}

#endif // HAL_USE_ASYNC_TIMER

//eof halSleepTimerClock.c

