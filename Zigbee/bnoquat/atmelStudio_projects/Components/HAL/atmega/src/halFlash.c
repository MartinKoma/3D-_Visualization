/**************************************************************************//**
  \file  halFlash.c

  \brief Implementation of flash access hardware-dependent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
   27/06/12 D. Kolmakov - Created
  Last change:
    $Id: halFlash.c 28120 2015-07-06 15:11:04Z agasthian.s $
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_FLASH_ACCESS)

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halAssert.h>
#include <halFlash.h>
#include <atomic.h>
#include <sysUtils.h>
#include <Atomic.h>

/******************************************************************************
                   define section
******************************************************************************/
#define STORE_REGISTERS()  \
    asm("push r0"); \
    asm("push r1"); \
    asm("push r2"); \
    asm("push r3"); \
    asm("push r4"); \
    asm("push r5"); \
    asm("push r6"); \
    asm("push r7"); \
    asm("push r8"); \
    asm("push r9"); \
    asm("push r10"); \
    asm("push r11"); \
    asm("push r12"); \
    asm("push r13"); \
    asm("push r14"); \
    asm("push r15"); \
    asm("push r16"); \
    asm("push r17"); \
    asm("push r18"); \
    asm("push r19"); \
    asm("push r20"); \
    asm("push r21"); \
    asm("push r22"); \
    asm("push r23"); \
    asm("push r24"); \
    asm("push r25"); \
    asm("push r26"); \
    asm("push r27"); \
    asm("push r28"); \
    asm("push r29"); \
    asm("push r30"); \
    asm("push r31"); \
    asm("ldi r26, 0x00"); \
    asm("ldi r27, 0x00"); \
    asm("ldi r28, 0x00"); \
    asm("ldi r29, 0x00"); \
    asm("ldi r30, 0x00"); \
    asm("ldi r31, 0x00");

#define RESTORE_REGISTERS() \
    asm("pop r31"); \
    asm("pop r30"); \
    asm("pop r29"); \
    asm("pop r28"); \
    asm("pop r27"); \
    asm("pop r26"); \
    asm("pop r25"); \
    asm("pop r24"); \
    asm("pop r23"); \
    asm("pop r22"); \
    asm("pop r21"); \
    asm("pop r20"); \
    asm("pop r19"); \
    asm("pop r18"); \
    asm("pop r17"); \
    asm("pop r16"); \
    asm("pop r15"); \
    asm("pop r14"); \
    asm("pop r13"); \
    asm("pop r12"); \
    asm("pop r11"); \
    asm("pop r10"); \
    asm("pop r9"); \
    asm("pop r8"); \
    asm("pop r7"); \
    asm("pop r6"); \
    asm("pop r5"); \
    asm("pop r4"); \
    asm("pop r3"); \
    asm("pop r2"); \
    asm("pop r1"); \
    asm("pop r0");

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Forms boot flash request and calls handler in boot section.
  \param[in] flashAccessReq - flash access request
******************************************************************************/
void halFlashAccessReq(HAL_FlashAccessReq_t *flashAccessReq)
{
  uint8_t gpior0;
  uint8_t gpior1;

  gpior0 = GPIOR0;
  gpior1 = GPIOR1;

  GPIOR1 = ((uint16_t)flashAccessReq >> 8);
  GPIOR0 = (uint16_t)flashAccessReq;

#if defined(__GNUC__)
  STORE_REGISTERS()
  asm("call 0x3fff0");
  RESTORE_REGISTERS()
#elif defined(__ICCAVR__)
  {
    HAL_BootSectionAccessPoint_t bootSectionAccessPoint = NULL;
    memcpy_P(&bootSectionAccessPoint, (__farflash void *)BOOT_SECTION_ACCESS_POINT, sizeof(HAL_BootSectionAccessPoint_t));

    // Call boot section access point
    bootSectionAccessPoint();
  }
#endif

  // Call the end of the flash access event handler
  HAL_FlashAccessFinishedHandler();
  
  GPIOR0 = gpior0;
  GPIOR1 = gpior1;
}

#endif  // defined(HAL_USE_BOOT_FLASH)
