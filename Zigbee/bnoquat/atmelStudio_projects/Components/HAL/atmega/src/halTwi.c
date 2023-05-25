/**************************************************************************//**
  \file  halTwi.c

  \brief Provides the functionality of TWI.

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
#if defined(HAL_USE_TWI)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halTwi.h>
#include <halTaskManager.h>
#include <halDbg.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Inits TWI module. Setup teh speed of TWI.

\param[in]
  rate - the speed of TWI.
******************************************************************************/
void halInitI2c(I2cClockRate_t rate)
{
  TWCR = 0x00;
  TWSR = HAL_I2C_PRESCALER; // prescaler
  // Set bit rate
  TWBR = rate;
}

/**************************************************************************//**
\brief Interrupt handler.
******************************************************************************/
ISR(TWI_vect)
{
  BEGIN_MEASURE
  switch (TWSR & 0xF8)
  {
    case TWS_START:
    case TWS_RSTART:
      halSendStartDoneI2c();
      break;

    case TWS_MT_SLA_ACK:
    case TWS_MT_DATA_ACK:
      halWriteDoneI2c();
      break;

    case TWS_BUSERROR:
    case TWS_MT_SLA_NACK:
    case TWS_MT_DATA_NACK:
    case TWS_MR_SLA_NACK:
      halI2cBusReset();
      break;

    case TWS_MR_SLA_ACK:
      halMasterReadWriteAddressAckI2c();
      break;

    case TWS_MR_DATA_ACK:
      halReadDoneI2c(halReadByteI2c());
      break;

    case TWS_MR_DATA_NACK:
      halReadLastByteDoneI2c(halReadByteI2c());
      break;

    default:
      break;
  }
  END_MEASURE(HALISR_TWI_MEASURE_CODE)
}
#endif // defined(HAL_USE_TWI)

// eof halTwi.c

