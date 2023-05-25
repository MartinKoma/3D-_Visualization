/**************************************************************************//**
  \file  halWdt.h

  \brief  Declarations of wdt hardware-dependent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      1/10/08 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALWDT_H
#define _HALWDT_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(__ICCAVR__)

/** Enable the watch dog timer with a specific timeout value */
#define wdt_enable(timeout) do {        \
    uint8_t volatile sreg_temp = SREG;  \
    cli();                              \
    __watchdog_reset();                 \
    WDTCSR |= (1 << WDCE) | (1 << WDE); \
    WDTCSR = (1 << WDE) | timeout;      \
    SREG = sreg_temp;                   \
} while (0)

#define wdt_disable()   MCUSR = 0;      \
  WDTCSR |= (1 << WDCE) | (1 << WDE);   \
  WDTCSR = 0x00;

#endif

#endif /* _HALWDT_H */

//eof halWdt.h
