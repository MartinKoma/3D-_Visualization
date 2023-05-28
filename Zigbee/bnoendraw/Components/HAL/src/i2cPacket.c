/**************************************************************************//**
  \file  i2cPacket.c

  \brief Provides the functionality for the writing and the reading \n
         of packets through the TWI.

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
#include <sysTypes.h>
#include <i2cPacket.h>
#include <halTaskManager.h>
#include <halDbg.h>
#include <sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/* states of the i2c transaction */
#define I2C_CLOSE                    0
#define I2C_IDLE                     1
#define I2C_WRITE_IADDR_WRITE_DATA   2
#define I2C_WRITE_IADDR_READ_DATA    3
#define I2C_WRITE_DATA               4
#define I2C_READ_DATA                5
#define I2C_TRANSAC_SUCCESS          6
#define I2C_TRANSAC_FAIL             7

/******************************************************************************
                   Global variables section
******************************************************************************/
static HAL_I2cDescriptor_t *halI2cDesc;

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Resets TWI bus and i2c HAL.
******************************************************************************/
void halI2cBusReset(void)
{
  halI2cDesc->service.state = I2C_TRANSAC_FAIL;
  halResetI2c();
  halPostTask(HAL_TWI);
}

/**************************************************************************//**
\brief Opens twi resource. Clock rate and tty must be configured. \n

\ingroup hal_i2c

\param[in]
  descriptor - pointer to HAL_I2cDescriptor_t structure

\return
  -1 - resource was already open or pointer is NULL. \n
   0 - success.
******************************************************************************/
int HAL_OpenI2cPacket(HAL_I2cDescriptor_t *descriptor)
{
  if (NULL == descriptor)
    return -1;
  if (halI2cDesc)
    return -1;

  halI2cDesc = descriptor;

  halInitI2c(halI2cDesc->clockRate);
  halI2cDesc->service.state = I2C_IDLE;
  halI2cDesc->service.index = 0;
  return 0;
}

/**************************************************************************//**
\brief Closes resource.

\ingroup hal_i2c

\param[in]
  descriptor - pointer to HAL_I2cDescriptor_t structure

\return
  -1 - resource was not open. \n
   0 - success.
******************************************************************************/
int HAL_CloseI2cPacket(HAL_I2cDescriptor_t *descriptor)
{
  if (!descriptor)
    return -1;
  if (descriptor != halI2cDesc)
    return -1;

  halI2cDesc = NULL;

  if (I2C_CLOSE != descriptor->service.state)
  {
    descriptor->service.state = I2C_CLOSE;
    return 0;
  }
  return -1;
}

/**************************************************************************//**
\brief Writes a series of bytes out to the TWI bus. Operation result will be
sent to the callback function of the HAL_I2cDescriptor_t structure.

\ingroup hal_i2c

\param[in]
  descriptor - pointer to HAL_I2cDescriptor_t structure

\return
   0 - the bus is free and the request is accepted. \n
  -1 - otherwise.
******************************************************************************/
int HAL_WriteI2cPacket(HAL_I2cDescriptor_t *descriptor)
{
  HalI2cPacketService_t *service;

  if (!descriptor)
    return -1;
  if (halI2cDesc != descriptor)
    return -1;
  if (!halI2cDesc->data && !halI2cDesc->length)
    return -1;

  service = &halI2cDesc->service;
  if (I2C_IDLE != service->state)
    return -1;

  service->index = halI2cDesc->lengthAddr;

  if (HAL_NO_INTERNAL_ADDRESS == halI2cDesc->lengthAddr)
    service->state = I2C_WRITE_DATA;
  else
    service->state = I2C_WRITE_IADDR_WRITE_DATA;
  halSendStartI2c();
  return 0;
}

/**************************************************************************//**
\brief Reads the series of bytes from the TWI bus. Operation result will be
sent to the callback function of the HAL_I2cDescriptor_t structure.

\ingroup hal_i2c

\param[in]
  descriptor - pointer to HAL_I2cDescriptor_t structure

\return
  0 - the bus is free and the request is accepted. \n
 -1 - otherwise.
******************************************************************************/
int HAL_ReadI2cPacket(HAL_I2cDescriptor_t *descriptor)
{
  HalI2cPacketService_t *service;

  if (!descriptor)
    return -1;
  if (halI2cDesc != descriptor)
    return -1;
  if (!halI2cDesc->data && !halI2cDesc->length)
    return -1;

  service = &halI2cDesc->service;
  if (I2C_IDLE != service->state)
    return -1;

  service->index = halI2cDesc->lengthAddr;

  if (HAL_NO_INTERNAL_ADDRESS == halI2cDesc->lengthAddr)
    service->state = I2C_READ_DATA;
  else
    service->state = I2C_WRITE_IADDR_READ_DATA;
  halSendStartI2c();
  return 0;
}

/******************************************************************************
Notification about the start condition was sent.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halSendStartDoneI2c(void)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if ((I2C_WRITE_IADDR_WRITE_DATA == service->state) ||
      (I2C_WRITE_IADDR_READ_DATA == service->state) ||
      (I2C_WRITE_DATA == service->state))
  {
    halWriteI2c(((halI2cDesc->id << 1) + 0));
  }
  else if (I2C_READ_DATA == service->state)
  {
    halWriteI2c(((halI2cDesc->id << 1) + 1));
  }
  else
  { // abnormal
    halI2cBusReset();
  }
}

/******************************************************************************
Sending data to i2c bus. If last byte then send stop condition and post task.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halWriteData(void)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if (service->index < halI2cDesc->length)
  {
    halWriteI2c(halI2cDesc->data[service->index++]);
  }
  else
  {
    service->state = I2C_TRANSAC_SUCCESS;
    halSendStopI2c();
    halPostTask(HAL_TWI);
  }
}

/******************************************************************************
Sending internal device address to i2c bus. If address is sent then switch i2c
hal state.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halWriteInternalAddress(void)
{
  uint8_t data;
  HalI2cPacketService_t *service = &halI2cDesc->service;

  data = (uint8_t)(halI2cDesc->internalAddr >> --service->index * 8);
  halWriteI2c(data);

  if (0 == service->index)
  {
    if (I2C_WRITE_IADDR_WRITE_DATA == service->state)
      service->state = I2C_WRITE_DATA;
    else
      service->state = I2C_READ_DATA;
  }
}

/******************************************************************************
Notification that byte was written to the TWI.
Parameters:
  result - contains result of previous operation.
Returns:
  none.
******************************************************************************/
void halWriteDoneI2c(void)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if (I2C_WRITE_DATA == service->state)
  {
    halWriteData();
  }
  else if ((I2C_WRITE_IADDR_WRITE_DATA == service->state) || (I2C_WRITE_IADDR_READ_DATA == service->state))
  {
    halWriteInternalAddress();
  }
  else if (I2C_READ_DATA == service->state)
  {
    halSendStartI2c();
  }
  else
  { // abnormal
    halI2cBusReset();
  }
}

/******************************************************************************
Notification that address byte was written to the TWI and was read ACK.
Starts reading data.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halMasterReadWriteAddressAckI2c(void)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if (I2C_READ_DATA == service->state)
  {
    if (1 == halI2cDesc->length)
      halReadI2c(false); // send nack
    else
      halReadI2c(true);  // send ack
  }
  else
  { // abnormal
    halI2cBusReset();
  }
}

/******************************************************************************
Notification that byte was read from the TWI.
Parameters:
  data - contains byte that was read.
Returns:
  none.
******************************************************************************/
void halReadDoneI2c(uint8_t data)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if (I2C_READ_DATA == service->state)
  {
    halI2cDesc->data[service->index++] = data;
    if (service->index < (halI2cDesc->length - 1))
      halReadI2c(true);  // send ACK
    else
      halReadI2c(false); // send NACK
  }
  else
  { // abnormal
    halI2cBusReset();
  }
}

/******************************************************************************
Notification that last byte was read from the TWI. Needs send STOP condition
on bus.
Parameters:
  data - contains byte that was read.
Returns:
  none.
******************************************************************************/
void halReadLastByteDoneI2c(uint8_t data)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;

  if (I2C_READ_DATA == service->state)
  {
    halI2cDesc->data[service->index++] = data;
    service->state = I2C_TRANSAC_SUCCESS;
    halSendStopI2c();
    halPostTask(HAL_TWI);
  }
  else
  { // abnormal
    halI2cBusReset();
  }
}

/******************************************************************************
Waits for end of sending and calls user's callback
******************************************************************************/
void halSig2WireSerialHandler(void)
{
  HalI2cPacketService_t *service = &halI2cDesc->service;
  uint8_t currestState;
 
  currestState = service->state;
  service->state = I2C_IDLE;
  SYS_E_ASSERT_FATAL(halI2cDesc->f, I2C_NULLCALLBACK_0);

  if (I2C_TRANSAC_SUCCESS == currestState)
  {
    halWaitEndOfStopStation();
    halI2cDesc->f(true);
  }
  else
    halI2cDesc->f(false);
}
#endif // defined(HAL_USE_TWI)

// eof i2cPacket.c
