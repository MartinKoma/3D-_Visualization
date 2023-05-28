/**************************************************************************//**
  \file  halTaskManager.c

  \brief Implemenattion of HAL task manager.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      10/12/07 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halTaskManager.h>
#include <atomic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HANDLERS_GET(A, I) memcpy_P(A, &halHandlers[I], sizeof(HalTask_t))

#ifdef HAL_USE_USB
  #define HAL_USB_ENDPOINT_HANDLER      halEndPointHandler
  #define HAL_USB_SUSPEND_HANDLER       halSuspendHandler
  #define HAL_USB_RESUME_HANDLER        halResumeHandler
  #define HAL_USB_BUS_RESET_HANDLER     halBusResetHandler
#else
  #define HAL_USB_ENDPOINT_HANDLER      halEmptyHandler
  #define HAL_USB_SUSPEND_HANDLER       halEmptyHandler
  #define HAL_USB_RESUME_HANDLER        halEmptyHandler
  #define HAL_USB_BUS_RESET_HANDLER     halEmptyHandler
#endif // HAL_USE_USB

#ifdef HAL_USE_USART
  #define HAL_USART_HANDLER             halSigUsartHandler
#else
  #define HAL_USART_HANDLER             halEmptyHandler
#endif // HAL_USE_USART

#define HAL_APP_TIMER_HANDLER           halAppTimerHandler

#if !defined(AT90USB1287)
  #if defined(HAL_USE_ASYNC_TIMER)
    #define HAL_ASYNC_TIMER_HANDLER     halAsyncTimerHandler
    #define HAL_SLEEP_TIME_SYNC_HANDLER halSleepSystemTimeSynchronize
  #else
    #define HAL_ASYNC_TIMER_HANDLER     halEmptyHandler
    #define HAL_SLEEP_TIME_SYNC_HANDLER halEmptyHandler
  #endif // defined(HAL_USE_ASYNC_TIMER)
  #define HAL_WAKEUP_HANDLER            halWakeupHandler
  #define HAL_SLEEP_HANDLER             halPowerOff
#else
  #define HAL_ASYNC_TIMER_HANDLER       halEmptyHandler
  #define HAL_SLEEP_TIME_SYNC_HANDLER   halEmptyHandler
  #define HAL_WAKEUP_HANDLER            halEmptyHandler
  #define HAL_SLEEP_HANDLER             halEmptyHandler
#endif // !defined(AT90USB1287)

#ifdef HAL_USE_SPI
  #define HAL_SPI_HANDLER               halSpiRxByteComplete
#else
  #define HAL_SPI_HANDLER               halEmptyHandler
#endif // HAL_USE_SPI

#ifdef HAL_USE_HW_AES
  #define HAL_HW_AES_HANDLER            halSmRequestHandler
#else
  #define HAL_HW_AES_HANDLER            halEmptyHandler
#endif // HAL_USE_HW_AES

#ifdef HAL_USE_ADC
  #define HAL_ADC_HANDLER               halSigAdcHandler
#else
  #define HAL_ADC_HANDLER               halEmptyHandler
#endif // HAL_USE_ADC

#ifdef HAL_USE_EE_READY
  #define HAL_EEPROM_HANDLER            halSigEepromReadyHandler
#else
  #define HAL_EEPROM_HANDLER            halEmptyHandler
#endif // HAL_USE_EE_READY

#ifdef HAL_USE_TWI
  #define HAL_TWI_HANDLER               halSig2WireSerialHandler
#else
  #define HAL_TWI_HANDLER               halEmptyHandler
#endif // HAL_USE_TWI

#ifdef HAL_USE_EXT_HANDLER
  #define HAL_EXTERNAL_HANDLER          halExternalHandler
#else
  #define HAL_EXTERNAL_HANDLER          halEmptyHandler
#endif // HAL_USE_EXT_HANDLER

/******************************************************************************
                   Prototypes section
******************************************************************************/
void halAppTimerHandler(void);
void halSigUsartHandler(void);
void halSig2WireSerialHandler(void);
void halSigEepromReadyHandler(void);
void halAsyncTimerHandler(void);
void halPowerOff(void);
void halSigAdcHandler(void);
void halEndPointHandler(void);
void halSuspendHandler(void);
void halResumeHandler(void);
void halBusResetHandler(void);
void halWakeupHandler(void);
void halSmRequestHandler(void);
void halSleepSystemTimeSynchronize(void);
void halSpiRxByteComplete(void);
void halExternalHandler(void);
static void halEmptyHandler(void);

/******************************************************************************
                   Global variables section
******************************************************************************/
void (* extHandler)(void);
volatile HalTaskBitMask_t halTaskFlags = 0; // HAL USART tasks' bit mask.
volatile HalTaskBitMask_t halAcceptedTasks = HAL_ALL_TASKS_ACCEPTED_MASK;
PROGMEM_DECLARE(HalTask_t halHandlers[HAL_MAX_TASKS_ID]) =
{
  HAL_USB_ENDPOINT_HANDLER,
  HAL_USB_SUSPEND_HANDLER,
  HAL_USB_RESUME_HANDLER,
  HAL_USB_BUS_RESET_HANDLER,
  HAL_USART_HANDLER,
  HAL_APP_TIMER_HANDLER,
  HAL_ASYNC_TIMER_HANDLER,
  HAL_SLEEP_TIME_SYNC_HANDLER,
  HAL_WAKEUP_HANDLER,
  HAL_SLEEP_HANDLER,
  HAL_SPI_HANDLER,
  HAL_HW_AES_HANDLER,
  HAL_ADC_HANDLER,
  HAL_EEPROM_HANDLER,
  HAL_TWI_HANDLER,
  HAL_EXTERNAL_HANDLER
};

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Empty handler to fill empty peripheries.
******************************************************************************/
static void halEmptyHandler(void)
{}

/**************************************************************************//**
\brief Execute external handler if that is present.
******************************************************************************/
void halExternalHandler(void)
{
  if (extHandler)
    extHandler();
}

/**************************************************************************//**
\brief HAL task handler.
******************************************************************************/
void HAL_TaskHandler(void)
{
  HalTask_t         handler;
  HalTaskBitMask_t  mask = 1;
  HalTaskIds_t      index = 0;
  HalTaskBitMask_t  tmpFlags;

  ATOMIC_SECTION_ENTER
  tmpFlags = halTaskFlags;
  ATOMIC_SECTION_LEAVE

  if (!(tmpFlags & halAcceptedTasks))
    return;

  for ( ; index < HAL_MAX_TASKS_ID; index++, mask <<= 1)
  {
    if (tmpFlags & mask & halAcceptedTasks)
    {
      ATOMIC_SECTION_ENTER
      halTaskFlags ^= mask;
      ATOMIC_SECTION_LEAVE
      HANDLERS_GET(&handler, index);
      handler();
    }
  }
}

// eof halTaskManager.c
