/**************************************************************************//**
  \file  halEeprom.h

  \brief  Provides interface for the access to hardware dependent
      EEPROM module.

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

#ifndef _HALEEPROM_H
#define _HALEEPROM_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halTaskManager.h>
#include <eeprom.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/** \brief the mask to rise interrupt when operation on EEPROM was completed */
#define HAL_EEPROM_WRITE_MASK_INT (1 << EEMPE | 1 << EERIE)
#define HAL_EEPROM_WRITE_MASK (1 << EEMPE)

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Writes a byte to EEPROM.
\param[in]
  EECRMask - mask that define capability of interrupt after byte writing.
\param[in]
  address - address of byte
\param[in]
  data - data.
******************************************************************************/
void halEepromWrite(uint8_t EECRMask, uint16_t address, uint8_t data);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/**************************************************************************//**
\brief Waits completion of previous operation.
******************************************************************************/
INLINE void halWaitEepromReady(void)
{
  while (EECR & (1 << EEPE)); // wait for completion of previous write
}

/**************************************************************************//**
\brief Reads byte from EEPROM.
\param[in]
  address -address of byte.
\return
  a read byte.
******************************************************************************/
INLINE uint8_t halReadEeprom(uint16_t address)
{
  halWaitEepromReady();
  EEAR = address;
  EECR |= (1 << EERE);
  return EEDR;
}

/**************************************************************************//**
\brief Posts the task to taskhandler that "EEPROM ready"
  interrupt has occured.
******************************************************************************/
INLINE void halSigEepromReadyInterrupt(void)
{
  halPostTask(HAL_EE_READY);
}
#endif /*_HALEEPROM_H*/
//eof halEeprom.h
