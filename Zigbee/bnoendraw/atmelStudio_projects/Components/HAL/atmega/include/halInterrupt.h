/**************************************************************************//**
  \file  halInterrupt.h

  \brief Macroses to manipulate global interrupts.

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

#ifndef _HALINTERRUPT_H
#define _HALINTERRUPT_H

/******************************************************************************
Enables global interrupt.
******************************************************************************/
#define HAL_EnableInterrupts() sei()

/******************************************************************************
Disables global interrupt.
******************************************************************************/
#define HAL_DisableInterrupts() cli()

#endif /* _HALINTERRUPT_H */
// eof halInterrupt.h
