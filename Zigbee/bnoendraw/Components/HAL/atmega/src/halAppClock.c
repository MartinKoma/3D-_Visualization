/**************************************************************************//**
  \file  halAppClock.c

  \brief Implementation of appTimer hardware-dependent module.

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
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halAppClock.h>
#include <halDbg.h>
#include <halDiagnostic.h>
#include <sysEvents.h>
#if defined(_SYSTEM_TIME_ON_SLEEP_TIMER_)
  #include <halSleepTimerClock.h>
#endif

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
  \brief Returns the time left value for the smallest app timer.
  \return time left for the smallest application timer.
******************************************************************************/
uint32_t halGetTimeToNextAppTimer(void);

/******************************************************************************
                   Local functions prototypes section
 ******************************************************************************/
#if defined(_SYSTEM_TIME_ON_SLEEP_TIMER_)
static void smallestTimerRequestHandler(SYS_EventId_t eventId, SYS_EventData_t data);
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
static uint32_t halAppTime = 0ul;     // time of application timer
uint8_t halAppTimeOvfw = 0;
static volatile uint8_t halAppIrqCount = 0;

#if defined(_SYSTEM_TIME_ON_SLEEP_TIMER_)
static SYS_EventReceiver_t appTimerEventListener = {.func = smallestTimerRequestHandler};
#endif

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Initialization appTimer clock.
******************************************************************************/
void halInitAppClock(void)
{
  OCR4A = TOP_TIMER_COUNTER_VALUE; // 1 millisecond timer interrupt interval.
  TCCR4B = (1 << WGM12);           // CTC mode
  halStartAppClock();              // clock source is cpu divided by 8
  TIMSK4 |= (1 << OCIE4A);         // Enable TC4 interrupt

#if defined(_SYSTEM_TIME_ON_SLEEP_TIMER_)
  SYS_SubscribeToEvent(BC_EVENT_MAX_SLEEP_TIME_REQUEST, &appTimerEventListener);
#endif
}

/******************************************************************************
Return time of sleep timer.

Returns:
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfAppTimer(void)
{
  halAppSystemTimeSynchronize();
  return halAppTime;
}

/******************************************************************************
Return system time in us

Parameters:
  mem - memory for system time
Returns:
  none.
******************************************************************************/
void halGetSystemTimeUs(uint64_t *mem)
{
#if (F_CPU == 4000000ul)
  volatile uint16_t counterTemp = TCNT4;
  ATOMIC_SECTION_ENTER
  uint64_t timeTemp = 1000ull * halAppTime + (counterTemp << 1);
  *mem = ( (TIFR4 & (1 << OCF4A)) && (counterTemp <= TCNT4) ) ? timeTemp + 10000 : timeTemp ;
  ATOMIC_SECTION_LEAVE
#endif
#if (F_CPU == 8000000ul)
  volatile uint16_t counterTemp = TCNT4;
  ATOMIC_SECTION_ENTER
  uint64_t timeTemp = 1000ull * halAppTime + counterTemp;
  *mem = ( (TIFR4 & (1 << OCF4A)) && (counterTemp <= TCNT4) ) ? timeTemp + 10000 : timeTemp ;
  ATOMIC_SECTION_LEAVE
#endif
#if (F_CPU == 16000000ul)
  volatile uint16_t counterTemp = TCNT4;
  ATOMIC_SECTION_ENTER
  uint64_t timeTemp = 1000ull * halAppTime + (counterTemp >> 1);
  *mem = ( (TIFR4 & (1 << OCF4A)) && (counterTemp <= TCNT4) ) ? timeTemp + 10000 : timeTemp ;
  ATOMIC_SECTION_LEAVE
#endif
}

/**************************************************************************//**
\brief Takes account of the sleep interval.

\param[in]
  interval - time of sleep
******************************************************************************/
void halAdjustSleepInterval(uint32_t interval)
{
  halAppTime += interval;
  halPostTask(HAL_TIMER4_COMPA);
}

/**************************************************************************//**
Synchronization system time which based on application timer.
******************************************************************************/
void halAppSystemTimeSynchronize(void)
{
  uint8_t tmpCounter;
  uint32_t tmpValue;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    tmpCounter = halAppIrqCount;
    halAppIrqCount = 0;
  END_MEASURE(HALATOM_APP_TIMER_SYNC_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  tmpValue = tmpCounter * HAL_APPTIMERINTERVAL;
  halAppTime += tmpValue;
  if (halAppTime < tmpValue)
    halAppTimeOvfw++;
}

/******************************************************************************
Interrupt handler
******************************************************************************/
ISR(TIMER4_COMPA_vect)
{
  BEGIN_MEASURE
  halAppIrqCount++;
  halPostTask(HAL_TIMER4_COMPA);
  // infinity loop spy
  SYS_InfinityLoopMonitoring();
  END_MEASURE(HALISR_TIMER4_COMPA_MEASURE_CODE)
}

#if defined(_SYSTEM_TIME_ON_SLEEP_TIMER_)
/**************************************************************************//**
  \brief Processes BC_EVENT_MAX_SLEEP_TIME_REQUEST request

  \param[in] eventId - id of raised event;
  \param[in] data    - event's data
 ******************************************************************************/
static void smallestTimerRequestHandler(SYS_EventId_t eventId, SYS_EventData_t data)
{
  uint32_t *minValue = (uint32_t *)data;
  uint32_t timeToFire = halGetTimeToNextAppTimer();

  if (*minValue <= HAL_MIN_SLEEP_TIME_ALLOWED)
  {
    *minValue = 0;
    return;
  }

  if (*minValue > timeToFire)
  {
    if (timeToFire > HAL_MIN_SLEEP_TIME_ALLOWED)
      *minValue = timeToFire;
    else
      *minValue = 0;
  }
  (void)eventId;
}
#endif

// eof halAppClock.c
