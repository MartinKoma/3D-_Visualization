/**************************************************************************//**
  \file  halSleep.c

  \brief Implementation of sleep modes.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      1/12/09 A. Khromykh - Created
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
#include <halIrq.h>
#include <halDbg.h>
#include <halEeprom.h>
#include <sysEvents.h>
#include <sysIdleHandler.h>

/******************************************************************************
                   Defines section
******************************************************************************/
#define RF_TRX_STATE_MASK                 0x1f
#define RF_TRX_OFF_STATE                  0x08
#define RF_TRX_FORCE_OFF                  0x03  

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Performs calibration of the main clock generator.
******************************************************************************/
void halStartingCalibrate(void);

#ifdef _HAL_RF_RX_TX_INDICATOR_

/**************************************************************************//**
\brief  Turn on pin 1 (DIG3) and pin 2 (DIG4) to indicate the transmit state of
the radio transceiver.
******************************************************************************/
void phyRxTxSwitcherOn(void);

/**************************************************************************//**
\brief  Turn off pin 1 (DIG3) and pin 2 (DIG4) to indicate the transmit state of
the radio transceiver.
******************************************************************************/
void phyRxTxSwitcherOff(void);

#endif

#ifdef _HAL_ANT_DIVERSITY_
/**************************************************************************//**
\brief  Turn on pin 9 (DIG1) and pin 10 (DIG2) to enable antenna select.
******************************************************************************/
void phyAntennaSwitcherOn(void);

/**************************************************************************//**
\brief  Turn off pin 9 (DIG1) and pin 10 (DIG2) to disable antenna select.
******************************************************************************/
void phyAntennaSwitcherOff(void);

#endif // _HAL_ANT_DIVERSITY_

/****************************************************************************
                   Static functions prototypes section
******************************************************************************/
static void halPutCpuToSleep(HAL_SleepMode_t sleepMode);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;
extern volatile uint16_t SYS_taskFlag;

#if defined(HAL_USE_ASYNC_TIMER)
/******************************************************************************
                   Global variables section
******************************************************************************/
static uint32_t halTimeStartOfSleep = 0ul;

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Switch on system power.

\param[in]
  wakeupSource - wake up source
******************************************************************************/
void halPowerOn(const uint8_t wakeupSource)
{
  halSleepControl.wakeupStation = HAL_ACTIVE_MODE;
  halSleepControl.wakeupSource = wakeupSource;

  TRXPR &= ~(1 << SLPTR);

  halPostTask(HAL_WAKEUP);
}

#if defined(__ICCAVR__)
#pragma optimize=medium
#endif /* __ICCAVR__ */
/******************************************************************************
\brief Puts cpu into sleep mode.

\param[in]
  sleepMode - sleep mode
  NOTES:
  the application should be sure the poweroff will not be
  interrupted after the execution of the sleep().
******************************************************************************/
static void halPutCpuToSleep(HAL_SleepMode_t sleepMode)
{
#if defined(_SLEEP_WHEN_IDLE_)
  if (SYS_IsSleepWhenIdleEnabled())
  {
    cli();
    if (SYS_taskFlag)
    {
      // During device power off procedure some source has posted a task
      // so the power on procedure shall be called
      if (HAL_ACTIVE_MODE != halSleepControl.wakeupStation)
        halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
      // post task for task manager
      if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
        halInterruptSleepClock();
      sei();
      return;
    }
  }
#endif /* _SLEEP_WHEN_IDLE_ */

  halSleepControl.sleepState = HAL_SLEEP_STATE_CONTINUE_SLEEP;
  halSleepControl.sleepMode = sleepMode;

  if (HAL_SLEEP_MODE_IDLE == sleepMode)
  {
    SMCR = (1 << SE); // Idle mode
#if defined(_SLEEP_WHEN_IDLE_)
    sei();
#endif /* _SLEEP_WHEN_IDLE_ */
    __SLEEP;
    SMCR = 0;
  }
  else if (HAL_SLEEP_MODE_POWER_SAVE == sleepMode)
  { // sleep timer is started

    SMCR = (1 << SM1) | (1 << SM0) | (1 << SE); // power-save
#if defined(_SLEEP_WHEN_IDLE_)
    sei();
#endif /* _SLEEP_WHEN_IDLE_ */
    __SLEEP;
    SMCR = 0;
  }
  else
  {
    halStopSleepTimerClock();
    SMCR = (1 << SM1) | (1 << SE); // power-down
#if defined(_SLEEP_WHEN_IDLE_)
    sei();
#endif /* _SLEEP_WHEN_IDLE_ */
    __SLEEP;
    SMCR = 0;
    halStartSleepTimerClock();
    halStartingCalibrate();
  }
}

/******************************************************************************
Shutdowns system.
  NOTES:
  the application should be sure the poweroff will not be
  interrupted after the execution of the sleep().
******************************************************************************/
void halPowerOff(void)
{
  HAL_SleepMode_t sleepMode;

  if (HAL_ACTIVE_MODE == halSleepControl.wakeupStation)
    return;  // it is a too late to sleep.

  // stop application timer clock
  halStopAppClock(); // will be shutdown
  if (0ul == halTimeStartOfSleep)
  { // start of sleep procedure
    // save time of stopping of the application timer
    halTimeStartOfSleep = halGetTimeOfSleepTimer();
  }

  if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
    sleepMode = HAL_SLEEP_MODE_POWER_SAVE;
  else
    sleepMode = HAL_SLEEP_MODE_POWER_DOWN;

  if (HAL_SLEEP_STATE_CONTINUE_SLEEP != halSleepControl.sleepState)
  {
#ifdef _HAL_RF_RX_TX_INDICATOR_
    // disable front end driver if that is supported
    phyRxTxSwitcherOff();
#endif

#ifdef _HAL_ANT_DIVERSITY_
    // disable antenna diversity switcher
    phyAntennaSwitcherOff();
#endif //_HAL_ANT_DIVERSITY_

    SYS_PostEvent(HAL_EVENT_FALLING_ASLEEP, (SYS_EventData_t)&sleepMode);
  }
  else
    SYS_PostEvent(HAL_EVENT_CONTINUING_SLEEP, (SYS_EventData_t)&sleepMode);

  // wait for end of eeprom writing
  halWaitEepromReady();
  
  // making sure before sleep to put the trasnceiver to trx_off
  if(RF_TRX_OFF_STATE != (TRX_STATUS & RF_TRX_STATE_MASK))
  {
    TRX_STATE = RF_TRX_FORCE_OFF; 
  } 

  TRXPR |= (1 << SLPTR);

  halPutCpuToSleep(sleepMode);

  // wait for time about 1 TOSC1 cycle for correct re-entering from power save mode to power save mode
  // wait for time about 1 TOSC1 cycle for correct reading TCNT2 after wake up to
  OCR2B = SOME_VALUE_FOR_SYNCHRONIZATION;
  while (ASSR & HAL_ASSR_FLAGS);
}

/******************************************************************************
  Prepares system for power-save, power-down.
  Power-down the mode is possible only when internal RC is used
  Parameters:
  none.
  Returns:
  -1 there is no possibility to power-down system.
******************************************************************************/
int HAL_Sleep(void)
{
  halSleepControl.wakeupStation = HAL_SLEEP_MODE;  // the reset of sign of entry to the sleep mode.
  while (ASSR & HAL_ASSR_FLAGS);
  halPostTask(HAL_SLEEP);
  return 0;
}

/******************************************************************************
 Handler for task manager. It is executed when system has waked up.
******************************************************************************/
void halWakeupHandler(void)
{
  uint32_t timeEndOfSleep;
  HAL_SleepControl_t wakeupSource;

  timeEndOfSleep = halGetTimeOfSleepTimer();

  timeEndOfSleep -= halTimeStartOfSleep;  // time of sleep
  halTimeStartOfSleep = 0ul;
  // adjust application timer interval
  halAdjustSleepInterval(timeEndOfSleep);

  // start application timer clock
  halStartAppClock();

  // wakeup the transceiver by setting SLPTR to '0'
  TRXPR &= ~(1 << SLPTR);

  // Wait for radio to wake up.
  while (RF_TRX_OFF_STATE != (TRX_STATUS & RF_TRX_STATE_MASK));

  halSleepControl.sleepState = HAL_SLEEP_STATE_BEGIN;

  memcpy(&wakeupSource, &halSleepControl, sizeof(HAL_SleepControl_t));

  SYS_PostEvent(HAL_EVENT_WAKING_UP, (SYS_EventData_t)&wakeupSource);

#ifdef _HAL_ANT_DIVERSITY_
  // enable antenna diversity switcher
  phyAntennaSwitcherOn();
#endif

#ifdef _HAL_RF_RX_TX_INDICATOR_
  // enable front end driver if that is supported
  phyRxTxSwitcherOn();
#endif

  halSleepControl.sleepMode = HAL_SLEEP_MODE_NONE;
  if (HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE == halSleepControl.wakeupSource)
  {
    if (halSleepControl.callback)
      halSleepControl.callback(halSleepControl.startPoll);
  }
}
#else  // HAL_USE_ASYNC_TIMER

/******************************************************************************
  Prepares system for power-save, power-down.
  Power-down the mode is possible only when internal RC is used
  Parametres:
  none.
  Returns:
  -1 there is no possibility to power-down system.
******************************************************************************/
int HAL_Sleep(void)
{
  halSleepControl.wakeupStation = HAL_SLEEP_MODE;  // the reset of sign of entry to the sleep mode.
  halPostTask(HAL_SLEEP);
  return 0;
}

/**************************************************************************//**
\brief Switch on system power.

\param[in]
  wakeupSource - wake up source
******************************************************************************/
void halPowerOn(const uint8_t wakeupSource)
{
  halSleepControl.wakeupStation = HAL_ACTIVE_MODE;
  halSleepControl.wakeupSource = wakeupSource;

  TRXPR &= ~(1 << SLPTR);
  halPostTask(HAL_WAKEUP);
}

/******************************************************************************
Shutdowns system.
  NOTES:
  the application should be sure the poweroff will not be
  interrupted after the execution of the sleep().
******************************************************************************/
void halPowerOff(void)
{
  if (HAL_ACTIVE_MODE == halSleepControl.wakeupStation)
    return;  // it is a too late to sleep.

  if (HAL_SLEEP_TIMER_IS_STARTED != halSleepControl.sleepTimerState)
  {
#ifdef _HAL_RF_RX_TX_INDICATOR_
    // disable front end driver if that is supported
    phyRxTxSwitcherOff();
#endif

#ifdef _HAL_ANT_DIVERSITY_
    // disable antenna diversity switcher
    phyAntennaSwitcherOff();
#endif //_HAL_ANT_DIVERSITY_

    // wait for end of eeprom writing
    halWaitEepromReady();
    TRXPR |= (1 << SLPTR);

    SMCR = (1 << SM1) | (1 << SE); // power-down
    __SLEEP;
    SMCR = 0;
  }
}

/******************************************************************************
 Handler for task manager. It is executed when system has waked up.
******************************************************************************/
void halWakeupHandler(void)
{
  // Wait for radio to wake up.
  while (RF_TRX_OFF_STATE != (TRX_STATUS & RF_TRX_STATE_MASK));

#ifdef _HAL_ANT_DIVERSITY_
  // enable antenna diversity switcher
  phyAntennaSwitcherOn();
#endif

#ifdef _HAL_RF_RX_TX_INDICATOR_
  // enable front end driver if that is supported
  phyRxTxSwitcherOn();
#endif
}

#endif // HAL_USE_ASYNC_TIMER

/*******************************************************************************
  Makes MCU enter Idle mode.
*******************************************************************************/
#if defined(__ICCAVR__)
#pragma optimize=medium
#endif
void HAL_IdleMode(void)
{
  cli();

  if (SYS_taskFlag)
  {
    sei();
    return;
  }

  SMCR = 1 << SE;
  sei();
  __SLEEP;
  SMCR = 0;
}
#ifdef _ZGPD_SPECIFIC_
/*******************************************************************************
  Shuts down the following Peripherials to save power
*******************************************************************************/
void HAL_DisablePeripherials(void)
{
    DDRA = 0x00;
    DDRB = 0x00;
    DDRC = 0x00;
    DDRD = 0x00;
    DDRE = 0x00;
    DDRF = 0x00;
    DDRG = 0x00;
    PORTA = 0xFF;
    PORTB = 0xFF;
    PORTC = 0xFF;
    PORTD = 0xFF;
    PORTE = 0xFF;
    PORTF = 0xFF;
    PORTG = 0xFF;
    PRR0 = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRPGA) | (1<<PRTIM1) | (1<<PRSPI) | (1<<PRUSART0) | (1<<PRADC);
    PRR1 = (1<<PRTIM5) | (1<<PRTIM3) | (1<<PRUSART1);
}

/*******************************************************************************
  Enables Pull up
*******************************************************************************/
void HAL_EnablePUD(void)
{
    MCUCR = (1<<PUD);
}

/*******************************************************************************
  Disables Pull up
*******************************************************************************/
void HAL_DisablePUD(void)
{
    MCUCR &= ~(1<<PUD);
}

/*******************************************************************************
  Enables Timer/counter 2
*******************************************************************************/
void HAL_EnableSleepTimer(void)
{
    PRR0 |= (1<<PRTIM2);
}

/*******************************************************************************
  Enables USART1
*******************************************************************************/
void HAL_EnableUSART1(void)
{
    PRR1 &= ~(1<<PRUSART1);
}
/*******************************************************************************
  Enables Transceiver
*******************************************************************************/
void HAL_EnableTransceiver(void)
{
   PRR1 &= ~(1 << PRTRX24);
}
#endif
// eof halSleep.c
