/**************************************************************************//**
  \file eeprom.c

  \brief Implementation of the EEPROM interface.

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
#include <halEeprom.h>

/******************************************************************************
                   Types section
******************************************************************************/
typedef enum
{
  EEPROM_IDLE = 0,
  EEPROM_BUSY
} EepromState_t;

/******************************************************************************
                   Global variables section
******************************************************************************/
EepromState_t halEepromState = EEPROM_IDLE; // state
HAL_EepromParams_t halEepromParams;
void (*halEepromDone)();

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Reads some number of bytes defined by HAL_EepromParams_t from the EEPROM.
Parameters:
  params - address of HAL_EepromParams_t defined by user.
  readDone - callback method
Returns:
  0  - success.
  -1 - the EEPROM has request that was not completed,
       number of byte to read too much.
  -2 - eeprom is busy
******************************************************************************/
int HAL_ReadEeprom(HAL_EepromParams_t *params, void (*readDone)())
{
  uint16_t i;

  if (EEPROM_IDLE != halEepromState)
    return -2;
  if (NULL == params)
    return -1;
  if ((uint16_t)(params->address + params->length) > EEPROM_DATA_MEMORY_SIZE)
    return -1;

  halEepromState = EEPROM_BUSY;
  halEepromDone = readDone;
  halEepromParams = *params;
  halWaitEepromReady(); // wait for completion of previous operation
  for (i = 0; i < halEepromParams.length; i++)
  {
    halEepromParams.data[i] = halReadEeprom(halEepromParams.address++);
  }
  halEepromParams.length = 0;
  if (halEepromDone)
    halPostTask(HAL_EE_READY);
  else
    halEepromState = EEPROM_IDLE;

  return 0;
}

/******************************************************************************
Writes number of bytes defined by HAL_EepromParams_t to EEPROM.
By writeDone parameter user can control if write operation will be asynchronous
or synchronous.
Parameters:
  params - address of HAL_EepromParams_t defined by user.
  writeDone - address of callback. if writeDone is NULL write operation will be
              synchronous.
Returns:
   0 - success.
  -1 - the EEPROM has request that was not completed,
       number of byte to write too much.
  -2 - eeprom is busy
******************************************************************************/
int HAL_WriteEeprom(HAL_EepromParams_t *params, void (*writeDone)())
{
  uint16_t i;

  if (EEPROM_IDLE != halEepromState)
    return -2;
  if (NULL == params)
    return -1;
  if ((uint16_t)(params->address + params->length) > EEPROM_DATA_MEMORY_SIZE)
    return -1;

  halEepromState = EEPROM_BUSY;
  halEepromParams = *params;
  halEepromDone = writeDone;

  while (halEepromParams.length)
  {
    if (*halEepromParams.data != halReadEeprom(halEepromParams.address))
      break;
    halEepromParams.address++;
    halEepromParams.data++;
    halEepromParams.length--;
  }

  if (halEepromDone)
  {// asynchronous operation
    if (halEepromParams.length)
    {
      halEepromWrite(HAL_EEPROM_WRITE_MASK_INT, halEepromParams.address++, *halEepromParams.data++);
      halEepromParams.length--;
    }
    else
    {
      halPostTask(HAL_EE_READY);
    }
    return 0;
  }
  for (i = 0; i < halEepromParams.length; i++)
  {
    if (*halEepromParams.data == halReadEeprom(halEepromParams.address))
    {
      halEepromParams.address++;
      halEepromParams.data++;
      continue;
    }
    halEepromWrite(HAL_EEPROM_WRITE_MASK, halEepromParams.address++, *halEepromParams.data++);
  }
  halWaitEepromReady(); // wait for completion of previous write
  halEepromState = EEPROM_IDLE;

  return 0;
}

/******************************************************************************
Checks the eeprom state.

Returns:
  true  - eeprom is busy;
  false - eeprom is free;
******************************************************************************/
bool HAL_IsEepromBusy(void)
{
  if (EEPROM_BUSY == halEepromState)
    return true;
  else
    return false;
}

/**************************************************************************//**
\brief Stops current write operation.
******************************************************************************/
void HAL_StopWriteToEeprom(void)
{
  halWaitEepromReady();
  halEepromState = EEPROM_IDLE;
  halEepromParams.length = 0;
  halEepromDone = NULL;
}

/******************************************************************************
Interrupt handler about write completion to EEPROM.
******************************************************************************/
void halSigEepromReadyHandler(void)
{
  while (halEepromParams.length)
  {
    if (*halEepromParams.data != halReadEeprom(halEepromParams.address))
      break;
    halEepromParams.address++;
    halEepromParams.data++;
    halEepromParams.length--;
  }

  if (!halEepromParams.length)
  {
    halEepromState = EEPROM_IDLE;
    if (NULL != halEepromDone)
      halEepromDone();
    return;
  }
  halEepromWrite(HAL_EEPROM_WRITE_MASK_INT, halEepromParams.address++, *halEepromParams.data++);
  halEepromParams.length--;
}
#endif // defined(HAL_USE_EE_READY)
//eof eeprom.c
