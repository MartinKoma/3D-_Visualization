/**************************************************************************//**
  \file  halClkCtrl.h

  \brief Declarations of clock control hardware-dependent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/05/07 E. Ivanov - Created
      16/04/09 A. Khromykh - Refactored
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALCLKCTRL_H
#define _HALCLKCTRL_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halFCPU.h>

/******************************************************************************
                   Types section
******************************************************************************/
/**************************************************************************//**
\brief Possible clock source
******************************************************************************/
typedef enum
{
  INTERNAL_RC,
  OTHER_SOURCE
} ClkSource_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initialization system clock.
******************************************************************************/
void halInitFreq(void);

/**************************************************************************//**
\brief Return clock source

\return
  clock source.
******************************************************************************/
ClkSource_t halGetClockSource(void);

/**************************************************************************//**
\brief System clock.

\return
  system clock in Hz.
******************************************************************************/
uint32_t HAL_ReadFreq(void);

#endif /* _HALCLKCTRL_H */

// eof halClkCtrl.h
