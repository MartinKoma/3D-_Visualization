/***************************************************************************//**
  \file  halW1.h

  \brief Declarations of 1-wire hardware-dependent module.

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

#ifndef _HALW1_H
#define _HALW1_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <inttypes.h>

/******************************************************************************
                   Types section
******************************************************************************/
/** \brief i2c 1-wire status */
typedef enum
{
  /** \brief There is no device on the bus */
  W1_NO_DEVICE_STATUS,
  /** \brief At least one device is on the bus */
  W1_SUCCESS_STATUS,
  /** \brief Invalid CRC was read during the device search operation */
  W1_INVALID_CRC
} W1Status_t;

/***************************************************************************//**
\brief Reads byte from the bus.

\return
  byte read from the bus.
*******************************************************************************/
uint8_t halReadW1(void);

/***************************************************************************//**
\brief Reads bit from the bus.

\return
  Read bit is placed to position of last significant bit.
*******************************************************************************/
uint8_t halReadW1Bit(void);

/***************************************************************************//**
\brief Writes bit to the bus.

\param[in]
  value - to write. The bit is placed to position of last significant bit.
*******************************************************************************/
void halWriteW1bit(uint8_t value);

/***************************************************************************//**
\brief Writes byte to the bus

\param[in]
    value - byte to write.
*******************************************************************************/
void halWriteW1(uint8_t value);

/***************************************************************************//**
\brief Resets all devices connected to the bus.

\return
  0 - there are some devices at the bus. \n
  1 - there are not any devices at the bus.
*******************************************************************************/
uint8_t halResetW1(void);

/**************************************************************************//**
\brief Performs delay in microseconds

\param[in]
  delay - number of microseconds to be delay
******************************************************************************/
void __delay_us(uint8_t delay);

#endif /* _HALW1_H */
