/*************************************************************************//**
  \file halSecurityModule.c

  \brief Implementation of AES encryption routines.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    12.08.09 A. Taradov - Created.
*****************************************************************************/

#ifdef HAL_USE_HW_AES
/******************************************************************************
                    Includes section
******************************************************************************/
#include <dbg.h>
#include <halDbg.h>
#include <sysTypes.h>
#include <sysQueue.h>
#include <halSecurityModule.h>
#include <halTaskManager.h>
#include <sysAssert.h>

/******************************************************************************
                    Defines section
******************************************************************************/
#define AES_CTRL_IM             2
#define AES_CTRL_DIR            3
#define AES_CTRL_MODE           5
#define AES_CTRL_REQUEST        7

#define AES_STATUS_RY           0
#define AES_STATUS_ER           7

/******************************************************************************
                    Local variables
******************************************************************************/
static DECLARE_QUEUE(halSmRequestQueue);

/******************************************************************************
                    Implementations section
******************************************************************************/
/******************************************************************************
  Encryption commands handler.
  Parameters:
    request - pointer to request parameters structure.
  Returns:
    none
******************************************************************************/
void HAL_SmEncryptReq(HAL_SmEncryptReq_t *request)
{
  putQueueElem(&halSmRequestQueue, request);
  halPostTask(HAL_SM_REQ);
}

/******************************************************************************
  Resets encryption module
  Parameters:
    none
  Returns:
    none
******************************************************************************/
void HAL_SmReset(void)
{
  resetQueue(&halSmRequestQueue);
  halClearTask(HAL_SM_REQ);
}

/******************************************************************************
  Security Module task handler.
  Parameters:
    none
  Returns:
    none
******************************************************************************/
void halSmRequestHandler(void)
{
  uint8_t i, j;
  uint8_t blockOffset = 0;
  HAL_SmEncryptReq_t *request;

  request = getQueueElem(&halSmRequestQueue);

  switch (request->command)
  {
    case SM_SET_KEY_COMMAND:
    {
      for (i = 0; i < HAL_SM_SECURITY_KEY_SIZE; i++)
        AES_KEY = request->text[i];
    } break;

    case SM_ECB_DECRYPT_COMMAND:
    {
      // Derive decryption key by performing one dummy encryption

      AES_CTRL = (0 << AES_CTRL_DIR) | (0 << AES_CTRL_MODE); // ECB encryption

      // Load data to process
      for (j = 0; j < HAL_SM_SECURITY_BLOCK_SIZE; j++)
        AES_STATE = 0; // Dummy

      // Start processing
      AES_CTRL |= (1 << AES_CTRL_REQUEST);

      // Wait for operation to complete
      while (0 == (AES_STATUS & (1 << AES_STATUS_RY)));

      { // read decryption key and set it back
        uint8_t key[HAL_SM_SECURITY_KEY_SIZE];

        for (i = 0; i < HAL_SM_SECURITY_KEY_SIZE; i++)
          key[i] =  AES_KEY;

        for (i = 0; i < HAL_SM_SECURITY_KEY_SIZE; i++)
          AES_KEY = key[i];
      }
    }; // fall through for further processing

    case SM_ECB_ENCRYPT_COMMAND:
    {
      for (i = 0; i < request->blockCount; i++)
      {
        if (SM_ECB_ENCRYPT_COMMAND == request->command)
          AES_CTRL = (0 << AES_CTRL_DIR) | (0 << AES_CTRL_MODE); // ECB encryption
        else
          AES_CTRL = (1 << AES_CTRL_DIR) | (0 << AES_CTRL_MODE); // ECB decryption

        // Load data to process
        for (j = 0; j < HAL_SM_SECURITY_BLOCK_SIZE; j++)
          AES_STATE = request->text[blockOffset + j];

        // Start processing
        AES_CTRL |= (1 << AES_CTRL_REQUEST);

        // Wait for operation to complete
        while (0 == (AES_STATUS & (1 << AES_STATUS_RY)));

        // Store resulting data
        for (j = 0; j < HAL_SM_SECURITY_BLOCK_SIZE; j++)
          request->text[blockOffset + j] = AES_STATE;

        // Process the next block
        blockOffset += HAL_SM_SECURITY_BLOCK_SIZE;
      }
    } break;

    default:
    {
      SYS_E_ASSERT_FATAL(false, SECURITY_MODULE_INVALID_COMMAND);
    } break;
  }

  deleteHeadQueueElem(&halSmRequestQueue);
  if (getQueueElem(&halSmRequestQueue))
    halPostTask(HAL_SM_REQ);

  SYS_E_ASSERT_FATAL(request->HAL_SmEncryptConf, SECURITY_NULLCALLBACK0);
  request->HAL_SmEncryptConf();
}
#endif // HAL_USE_HW_AES

// eof halSecurityModule.c
