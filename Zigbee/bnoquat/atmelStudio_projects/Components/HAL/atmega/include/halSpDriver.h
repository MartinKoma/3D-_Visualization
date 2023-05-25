/****************************************************************************//**
  \file halSpDriver.h

  \brief AVR Self-programming driver header file.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    13/05/09 A. Khromykh - Created
    27/06/12 D. Kolmakov - Copied and modified
  Last change:
    $Id: halSpDriver.h 27584 2015-01-09 14:45:42Z unithra.c $
 ******************************************************************************/
#ifndef HAL_SP_DRIVER_H
#define HAL_SP_DRIVER_H

/******************************************************************************
                   Definitions section
******************************************************************************/
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 256
#endif

/******************************************************************************
                              Prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief This routine enables RWW section.
******************************************************************************/
void halSpRWWSectionEnable(void);

/**************************************************************************//**
  \brief Erase page buffer to application or application table section at byte address.
  \param[in] address - byte address for flash page.
******************************************************************************/
void halSpEraseApplicationPage(uint32_t address);

/**************************************************************************//**
  \brief Write page buffer to application or application table section at byte address.
  \param[in] address - byte address for flash page.
******************************************************************************/
void halSpWriteApplicationPage(uint32_t address);

/**************************************************************************//**
  \brief Load one word into Flash page buffer.
  \param[in] address - position in inside the flash page buffer.
  \param[in] data - value to be put into the buffer.
******************************************************************************/
void halSpLoadFlashWord(uint16_t address, uint16_t data);

/**************************************************************************//**
  \brief Read byte from internal flash.
  \param[in] address - byte address into flash.
  \return byte from flash
******************************************************************************/
uint8_t halSpReadInternalFlash(uint32_t address);

#endif /* HAL_SP_DRIVER_H */
