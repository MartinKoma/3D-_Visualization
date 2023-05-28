/**************************************************************************//**
  \file  w1.c

  \brief The implementation of the 1-wire interface.

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
#if defined(HAL_USE_1WIRE)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halW1.h>
#include <w1.h>
#include <atomic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_ROM_BYTE_LEN 8
#define HAL_ROM_BIT_LEN 64
#define HAL_W1_CRC_POLINOM 0x8C
#define HAL_SEARCH_ROM_CMD 0xF0
#define HAL_SEARCH_ALRM_CMD 0xEC

/******************************************************************************
                   Prototypes section
******************************************************************************/
/*******************************************************************************
 1-Wire search procedure for all devices discovering
 Parameters:
   cmd      - ROM function command.
   family   - 8-bit family code.
   data     - pointer of SRAM where are stored the 8 bytes ROM
              codes returned by the devices.
   count    - number of devices wish to find.
   actCount - number of devices have been found.
 Returns:
   W1_SUCCESS_STATUS   - if at least one device has been found.
   W1_NO_DEVICE_STATUS - if there are no any devices presented
                         on the bus with specified family code.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read and no devices with
                         spicified family code has been found.
*******************************************************************************/
W1Status_t halCommSearchW1(uint8_t cmd,
                           uint8_t family,
                           uint8_t *data,
                           uint8_t count,
                           uint8_t *actCount);

/*******************************************************************************
 1-Wire search procedure for one device discovering
 Parameters:
   cmd - ROM function command.
 Returns:
   W1_SUCCESS_STATUS   - if device has been found, ROM number
                         in ROM_NO buffer.
   W1_NO_DEVICE_STATUS - if device has not been found.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read.
*******************************************************************************/
W1Status_t halSearchW1(uint8_t cmd);

/*******************************************************************************
 Calculating 1-Wire 8-bit CRC
 Parameters:
   data   - data buffer pointer.
   length - data length.
 Returns:
   CRC value based on polynomial x^8 + x^5 + x^4 + 1
*******************************************************************************/
uint8_t halW1CRC(uint8_t *data, uint8_t length);

/******************************************************************************
                   Global variables section
******************************************************************************/
// 8-byte buffer that contains the current ROM registration
// number discovered
static uint8_t ROM_NO[HAL_ROM_BYTE_LEN];
// Bit index that identifies from which bit the next search
// discrepancy check should start
static uint8_t LastDiscrepancy;
// Flag to indicate previos search was the last device
static uint8_t LastDeviceFlag;

/******************************************************************************
                   Implementations section
******************************************************************************/
/*******************************************************************************
 1-Wire search procedure with search ROM command only
 Parameters:
   family   - 8-bit family code.
   data     - pointer of SRAM where are stored the 8 bytes ROM
              codes returned by the devices.
   count    - number of devices wish to find.
   actCount - number of devices have been found.
 Returns:
   W1_SUCCESS_STATUS   - if at least one device has been found.
   W1_NO_DEVICE_STATUS - if there are no any devices presented
                         on the bus with specified family code.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read and no devices with
                         spicified family code has been found.
*******************************************************************************/
W1Status_t HAL_SearchW1Device(uint8_t family,
                              uint8_t *data,
                              uint8_t count,
                              uint8_t *actCount)
{
  return halCommSearchW1(HAL_SEARCH_ROM_CMD, family, data, count, actCount);
}

/*******************************************************************************
 1-Wire search procedure with alarm search command only
 Parameters:
   family   - 8-bit family code.
   data     - pointer of SRAM where are stored the 8 bytes ROM
              codes returned by the devices.
   count    - number of devices wish to find.
   actCount - number of devices have been found.
 Returns:
   W1_SUCCESS_STATUS   - if at least one device has been found.
   W1_NO_DEVICE_STATUS - if there are no any devices presented
                         on the bus with specified family code.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read and no devices with
                         spicified family code has been found.
*******************************************************************************/
W1Status_t HAL_AlarmSearchW1Device(uint8_t family,
                                   uint8_t *data,
                                   uint8_t count,
                                   uint8_t *actCount)
{
  return halCommSearchW1(HAL_SEARCH_ALRM_CMD, family, data, count, actCount);
}

/*******************************************************************************
 1-Wire search procedure for all devices discovering
 Parameters:
   cmd      - ROM function command.
   family   - 8-bit family code.
   data     - pointer of SRAM where are stored the 8 bytes ROM
              codes returned by the devices.
   count    - number of devices wish to find.
   actCount - number of devices have been found.
 Returns:
   W1_SUCCESS_STATUS   - if at least one device has been found.
   W1_NO_DEVICE_STATUS - if there are no any devices presented
                         on the bus with specified family code.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read and no devices with
                         spicified family code has been found.
*******************************************************************************/
W1Status_t halCommSearchW1(uint8_t cmd,
                       uint8_t family,
                       uint8_t *data,
                       uint8_t count,
                       uint8_t *actCount)
{
  W1Status_t w1_result;
  uint8_t i;

  // Reset the searh state
  LastDiscrepancy = 0;
  LastDeviceFlag = 0;
  // Setup family code
  if (W1_ANY_FAMILY != family)
  {
    ROM_NO[0] = family;
    LastDiscrepancy = HAL_ROM_BIT_LEN + 1;
    for (i = 1; i < HAL_ROM_BYTE_LEN; i++) ROM_NO[i] = 0;
  }
  i = 0;
  // Discovering
  do
  {
    w1_result = halSearchW1(cmd);
    if (W1_SUCCESS_STATUS != w1_result)
      break;
    // There is some device with specified family code
    if ((ROM_NO[0] == family) || (W1_ANY_FAMILY == family))
    {
      memcpy((data + 8 * i), ROM_NO, sizeof(uint64_t));
      i++;
    }
    else // There are not any devices with specified family code
    {
      w1_result = W1_NO_DEVICE_STATUS;
      break;
    }
  }
  while (!LastDeviceFlag && (i < count));

  if (NULL != actCount)
    *actCount = i;

  if (i != 0)
    return W1_SUCCESS_STATUS;
  else
    return w1_result;
}

/*******************************************************************************
 1-Wire search procedure for one device discovering
 Parameters:
   cmd - ROM function command.
 Returns:
   W1_SUCCESS_STATUS   - if device has been found, ROM number
                         in ROM_NO buffer.
   W1_NO_DEVICE_STATUS - if device has not been found.
   W1_INVALID_CRC      - if during searching invalid CRC has
                         been read.
*******************************************************************************/
W1Status_t halSearchW1(uint8_t cmd)
{
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;

  // 1-Wire reset
  ATOMIC_SECTION_ENTER
    id_bit = halResetW1();
  ATOMIC_SECTION_LEAVE
  if (W1_NO_DEVICE_STATUS == id_bit)
  {
    LastDiscrepancy = 0;
    LastDeviceFlag = 0;
    return W1_NO_DEVICE_STATUS;
  }

  // issue the search command
  ATOMIC_SECTION_ENTER
    halWriteW1(cmd);
  ATOMIC_SECTION_LEAVE
  // search 64-bit uniqued registration number
  do
  {
    // read a bit and its complement
    ATOMIC_SECTION_ENTER
      id_bit = halReadW1Bit();
      cmp_id_bit = halReadW1Bit();
    ATOMIC_SECTION_LEAVE
    // check for no devices on 1-wire
    if ((1 == id_bit) && (1 == cmp_id_bit))
      return W1_NO_DEVICE_STATUS;

    // all devices coupled have 0 or 1
    if (id_bit != cmp_id_bit)
    {
      search_direction = id_bit;
      // there is descepancy
    }
    else
    {
      // if this discrepancy if before the Last Discrepancy
      // on a previous next then pick the same as last time (old branch)
      if (id_bit_number < LastDiscrepancy)
        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
      // if equal to last pick 1, if not then pick 0 (new branch)
      else
        search_direction = (id_bit_number == LastDiscrepancy);
      // if 0 was picked then record its position in LastZero
      if (0 == search_direction)
        last_zero = id_bit_number;
    }

    if (1 == search_direction)
      ROM_NO[rom_byte_number] |= rom_byte_mask;
    else
      ROM_NO[rom_byte_number] &= ~rom_byte_mask;
    ATOMIC_SECTION_ENTER
      halWriteW1bit(search_direction);
    ATOMIC_SECTION_LEAVE
    id_bit_number++;
    rom_byte_mask <<= 1;

    if (rom_byte_mask == 0)
    {
      rom_byte_number++;
      rom_byte_mask = 1;
    }
  } while (rom_byte_number < HAL_ROM_BYTE_LEN);

  // Invalid CRC
  if (halW1CRC(ROM_NO, HAL_ROM_BYTE_LEN))
    return W1_INVALID_CRC;

  LastDiscrepancy = last_zero;
  // check for last device
  if (0 == LastDiscrepancy)
    LastDeviceFlag = 1;

  return W1_SUCCESS_STATUS;
}

/*******************************************************************************
 Calculating 1-Wire 8-bit CRC
 Parameters:
   data   - data buffer pointer.
   length - data length.
 Returns:
   CRC value based on polynomial x^8 + x^5 + x^4 + 1
*******************************************************************************/
uint8_t halW1CRC(uint8_t *data, uint8_t length)
{
  uint8_t reg;
  uint8_t b;

  for (reg = 0; length > 0; length--, data++)
  {
    for (b = 0; b < 8; b++)
    {
      if (((*data >> b) & 1) ^ (reg & 1))
      {
        reg >>= 1;
        reg ^= HAL_W1_CRC_POLINOM;
      }
      else
        reg >>= 1;
    }
  }
  return reg;
}

/*******************************************************************************
Resets all devices connected to the bus.
Parameters:
  none.
Returns:
  0 - there are some devices at the bus.
  1 - there are not any devices at the bus.
*******************************************************************************/
uint8_t HAL_ResetW1(void)
{
  return halResetW1();
};

/*******************************************************************************
Writes byte to the bus
Parameters:
    value - byte to write.
Returns:
    none.
*******************************************************************************/
void HAL_WriteW1(uint8_t value)
{
  halWriteW1(value);
}

/*******************************************************************************
Reads byte from the bus.
Parameters:
    none.
Returns:
  byte read from the bus.
*******************************************************************************/
uint8_t HAL_ReadW1(void)
{
  return halReadW1();
}
#endif // defined(HAL_USE_1WIRE)

// eof halW1.c
