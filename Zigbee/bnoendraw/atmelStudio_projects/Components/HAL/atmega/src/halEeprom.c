/**************************************************************************//**
  \file  halEeprom.c

  \brief  Implementation of the hardware dependent the EEPROM module.

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
#if defined(HAL_USE_EE_READY)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <atomic.h>
#include <halEeprom.h>
#include <halDbg.h>
#include <halDiagnostic.h>
/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Writes a byte to EEPROM.
Parameters:
  EECRMask - mask that define capability of interrupt after byte writing.
  address - address of byte
  data - data.
Returns:
  none.
******************************************************************************/
void halEepromWrite(uint8_t EECRMask, uint16_t address, uint8_t data)
{
  while (EECR & (1 << EEPE)); // wait for completion of previous eeprom write
  while (SPMCSR & (1 << SPMEN)); // wait for completion of previous program memory write
  EEAR = address;
  EEDR = data;
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    EECR  = EECRMask;
    EECR |= (1 << EEPE);
  END_MEASURE(HALATOM_EEPROM_WRITE_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE
}

/******************************************************************************
Interrupt handler.
******************************************************************************/
ISR(EE_READY_vect)
{
  BEGIN_MEASURE
  EECR &= ~(1 << EERIE); //disable interrupt
  halSigEepromReadyInterrupt();
  END_MEASURE(HALISR_EEPROM_READY_MEASURE_CODE)
}
#endif // defined(HAL_USE_EE_READY)

// eof helEeprom.c
