/**************************************************************************//**
  \file  halWdtInit.c

  \brief Implementation of the reset reason interface.

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
#include <resetReason.h>
#include <sysTypes.h>
#include <wdtCtrl.h>

/******************************************************************************
                   External variables section
******************************************************************************/
extern uint8_t halResetReason; // contains the reset reason

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Returns the reset reason.
Parameters:
  none.
Returns:
  The reason of reset.
******************************************************************************/
HAL_ResetReason_t HAL_ReadResetReason(void)
{
  return (HAL_ResetReason_t)halResetReason;
}

/******************************************************************************
Software reset.
******************************************************************************/
#if defined(__ICCAVR__)
#pragma optimize=medium
#endif
void HAL_WarmReset(void)
{
  halResetReason = TEMP_WARM_RESET;
  wdt_enable(0);
  while(1);
}
//eof resetReason.c
