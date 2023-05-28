/*************************************************************************//**
  \file halSecurityModule.c

  \brief Prototypesof AES encryption routines.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    12.08.09 A. Taradov - Created.
******************************************************************************/

#ifndef _HALSECURITYMODULE_H
#define _HALSECURITYMODULE_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <sysTypes.h>

/******************************************************************************
                    Defines section
******************************************************************************/
#define HAL_SM_SECURITY_KEY_SIZE      16
#define HAL_SM_SECURITY_BLOCK_SIZE    16

/******************************************************************************
                    Types section
******************************************************************************/
typedef enum
{
  SM_SET_KEY_COMMAND,
  SM_SET_INIT_VECT_COMMAND,
  SM_ECB_ENCRYPT_COMMAND,
  SM_CBC_ENCRYPT_COMMAND,
  SM_ECB_DECRYPT_COMMAND,
} HAL_SmCommand_t;

typedef struct
{
  struct
  {
    void *next;
  } service;

  HAL_SmCommand_t command;
  uint8_t *text;
  uint8_t blockCount; // Block is 128-bit data.
  void (*HAL_SmEncryptConf)(void);
} HAL_SmEncryptReq_t;

/******************************************************************************
                    Prototypes section
******************************************************************************/
/******************************************************************************
  Encryption commands handler.
  Parameters:
    request - pointer to request parameters structure.
  Returns:
    none
******************************************************************************/
void HAL_SmEncryptReq(HAL_SmEncryptReq_t *request);

/******************************************************************************
  Resets encryption module
  Parameters:
    none
  Returns:
    none
******************************************************************************/
void HAL_SmReset(void);

/******************************************************************************
  Security Module task handler.
  Parameters:
    none
  Returns:
    none
******************************************************************************/
void halSmRequestHandler(void);

#endif /*_HALSECURITYMODULE_H*/

// eof halSecurityModule.h
