/**************************************************************************//**
  \file  halDelay.c

  \brief HAL_Delay() routine implementation

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    17.08.09 A. Taradov - Created.
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALDELAY_H
#define _HALDELAY_H

/******************************************************************************
                        Includes section.
******************************************************************************/
#include <inttypes.h>
#include <halW1.h>

/******************************************************************************
                    Types section
******************************************************************************/

/******************************************************************************
                    Implementation section
******************************************************************************/
/******************************************************************************
  Delay in us
******************************************************************************/
void HAL_Delay(uint8_t us)
{
  __delay_us(us);
}

#endif /* _HALDELAY_H */

// eof halDelay.h
