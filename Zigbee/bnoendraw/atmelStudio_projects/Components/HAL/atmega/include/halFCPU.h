/**************************************************************************//**
  \file  halFCPU.h

  \brief Declaration F_CPU for C code.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      6/10/08 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Define(s) section
******************************************************************************/
/* Main clock of CPU in Hz. */
#if defined(HAL_3d6864MHz)
  #define F_CPU 3686400
#elif defined(HAL_4MHz)
  #define F_CPU 4000000
#elif defined(HAL_7d3728MHz)
  #define F_CPU 7372800
#elif defined(HAL_8MHz)
  #define F_CPU 8000000
#elif defined(HAL_16MHz)
  #define F_CPU 16000000
#endif

// eof halFCPU.h
