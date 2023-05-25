/*****************************************************************************//**
\file  halFlash.h

\brief  Declarations of flash access module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    27/06/12 D. Kolmakov - Created
  Last change:
    $Id: halFlash.h 27584 2015-01-09 14:45:42Z unithra.c $
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _HALFLASH_H
#define _HALFLASH_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <flash.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
  \brief Forms boot flash request and calls handler in boot section.
  \param[in] flashAccessReq - flash access request
******************************************************************************/
void halFlashAccessReq(HAL_FlashAccessReq_t *flashAccessReq);

#endif // _HALFLASH_H

//eof halFlash.h
