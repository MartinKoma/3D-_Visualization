/**************************************************************************//**
  \file  halAtomic.h

  \brief Implementation of atomic sections.

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

#ifndef _HALATOMIC_H
#define _HALATOMIC_H

#include <sysTypes.h>

typedef  uint8_t atomic_t;

/******************************************************************************
Saves global interrupt bit. Disables global interrupt.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
INLINE atomic_t halStartAtomic(void)
{
  atomic_t result = SREG;
  cli();
  return result;
}

/******************************************************************************
Restores global interrupt.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
INLINE void halEndAtomic(atomic_t sreg)
{
  SREG = sreg;
}

#endif /* _HALATOMIC_H */
// eof atomic.h

