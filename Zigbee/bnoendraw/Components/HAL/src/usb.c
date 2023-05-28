/**************************************************************************//**
  \file  usb.c

  \brief Implementation of usb hardware independent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      17/07/08 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_USB)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halUsb.h>
#include <usb.h>
#include <sysAssert.h>

/******************************************************************************
                   External global variables section
******************************************************************************/
// user's request handler
extern void (* sendReqToUpper)(uint8_t *req);
// Holds the internal state for each endpoint of the UDP
extern UsbEndpoint_t endpoints[USB_END_POINTS_NUMBER];
// Device current state
extern volatile uint8_t deviceState;
// Previous device current state
extern volatile uint8_t previousDeviceState;
// pointer to request memory. Memory allocate by user.
extern uint8_t *requestMemory;

/******************************************************************************
                   Global variables section
******************************************************************************/
void (* resumeCallback)(void) = NULL;
void (* suspendCallback)(void) = NULL;
void (* endOfBusResetCallback)(void) = NULL;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Registers user's end of bus reset handler

Parameters:
  f - pointer to user's callback
******************************************************************************/
void HAL_RegisterEndOfBusResetHandler(void (* f)(void))
{
  endOfBusResetCallback = f;
}

/******************************************************************************
 Registers user's resume handler

Parameters:
  f - pointer to user's callback
******************************************************************************/
void HAL_RegisterResumeHandler(void (* f)(void))
{
  resumeCallback = f;
}

/******************************************************************************
 Registers user's suspend handler

Parameters:
  f - pointer to user's callback
******************************************************************************/
void HAL_RegisterSuspendHandler(void (* f)(void))
{
  suspendCallback = f;
}

/******************************************************************************
 Registers user's request handler

Parameters:
  f - pointer to user's callback
******************************************************************************/
void HAL_RegisterRequestHandler(void (* f)(uint8_t *req))
{
  sendReqToUpper = f;
}

/******************************************************************************
Configures an endpoint according to its Endpoint Descriptor.

Parameters:
  descriptor - Pointer to an Endpoint descriptor.
******************************************************************************/
void HAL_ConfigureEndpoint(HAL_UsbEndPointDescptr_t *descriptor)
{
  UsbEndpoint_t *endpoint;
  uint8_t eptnum;
  uint8_t type;
  HAL_UsbEndPointDirect_t direction;

  // NULL descriptor -> Control endpoint 0
  if (NULL == descriptor)
  {
    eptnum = USB_END_POINT_0;
    endpoint = &(endpoints[USB_END_POINT_0]);
    type = EP_CONTROL;
    direction = EP_OUT;
    endpoint->size = UDP_ENDPOINTS_MAXPACKETSIZE(USB_END_POINT_0);
  }
  else
  {
    eptnum = descriptor->bEndpointAddress & 0x0F;
    endpoint = &(endpoints[eptnum]);
    type = descriptor->bmAttributes & 0x03;
    if (descriptor->bEndpointAddress & 0x80)
      direction = EP_IN;
    else
      direction = EP_OUT;
    endpoint->size = descriptor->wMaxPacketSize;
  }

  // Abort the current transfer is the endpoint was configured and in
  // Write or Read state
  if ((UDP_ENDPOINT_RECEIVING == endpoint->state) || (UDP_ENDPOINT_SENDING == endpoint->state))
    halEndOfTransfer(eptnum, STATUS_RESET);

  endpoint->state = UDP_ENDPOINT_IDLE;
  halConfigureEndpoint(eptnum, type, direction);
}

/******************************************************************************
Sends data through a USB endpoint. Sets up the transfer descriptor,
writes one or two data payloads (depending on the number of FIFO bank
for the endpoint) and then starts the actual transfer. The operation is
complete when all the data has been sent.

*If the size of the buffer is greater than the size of the endpoint
(or twice the size if the endpoint has two FIFO banks), then the buffer
must be kept allocated until the transfer is finished*. This means that
it is not possible to declare it on the stack (i.e. as a local variable
of a function which returns after starting a transfer).

Parameters:
  eptnum - Endpoint number.
  data - Pointer to a buffer with the data to send.
  size - Size of the data buffer.
  callback - Optional callback function to invoke when the transfer is complete.
  argument - Optional argument to the callback function.

Returns:
  STATUS_SUCCESS if the transfer has been started; otherwise, the
  corresponding error status code.
******************************************************************************/
uint8_t HAL_UsbWrite(uint8_t eptnum, void *data, uint32_t size, TransferCallback_t callback, void *argument)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);

  // Check that the endpoint is in Idle state
  if (UDP_ENDPOINT_IDLE != endpoint->state)
    return STATUS_BUSY;

  // Setup the transfer descriptor
  transfer->data = (void *) data;
  transfer->remaining = size;
  transfer->buffered = 0;
  transfer->transferred = 0;
  transfer->callback = callback;
  transfer->argument = argument;

  // Send the first packet
  endpoint->state = UDP_ENDPOINT_SENDING;
  halStartUsbWrite(eptnum);

  // Enable interrupt on endpoint
  halEnableEndPointTxInterrupt(eptnum);

  return STATUS_SUCCESS;
}

/******************************************************************************
Reads incoming data on an USB endpoint This methods sets the transfer
descriptor and activate the endpoint interrupt. The actual transfer is
then carried out by the endpoint interrupt handler. The Read operation
finishes either when the buffer is full, or a short packet (inferior to
endpoint maximum  size) is received.

*The buffer must be kept allocated until the transfer is finished*.

Parameters:
  eptnum - Endpoint number.
  data - Pointer to a data buffer.
  size - Size of the data buffer in bytes.
  callback - Optional end-of-transfer callback function.
  argument - Optional argument to the callback function.

Returns:
  STATUS_SUCCESS if the read operation has been started; otherwise,
  the corresponding error code.
******************************************************************************/
uint8_t HAL_UsbRead(uint8_t eptnum, void *data, uint32_t size, TransferCallback_t callback, void *argument)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);

  if (NULL == data)
    return STATUS_ABORTED;

  // Return if the endpoint is not in IDLE state
  if (UDP_ENDPOINT_IDLE != endpoint->state)
    return STATUS_BUSY;

  // Endpoint enters Receiving state
  endpoint->state = UDP_ENDPOINT_RECEIVING;

  // Set the transfer descriptor
  transfer->data = data;
  transfer->remaining = size;
  transfer->buffered = 0;
  transfer->transferred = 0;
  transfer->callback = callback;
  transfer->argument = argument;

  // Enable interrupt on endpoint
  halEnableEndPointRxInterrupt(eptnum);

  return STATUS_SUCCESS;
}

/******************************************************************************
Sets the HALT feature on the given endpoint (if not already in this state).

Parameters:
  eptnum - Endpoint number.
******************************************************************************/
void HAL_Halt(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);

  // Check that endpoint is enabled and not already in Halt state
  if ((UDP_ENDPOINT_DISABLED != endpoint->state) && (UDP_ENDPOINT_HALTED != endpoint->state))
  {
    // Abort the current transfer if necessary
    halEndOfTransfer(eptnum, STATUS_ABORTED);

    halEndpointHaltState(eptnum);

    // Enable the endpoint interrupt
    halEnableEndPointStallInterrupt(eptnum);
  }
}

/******************************************************************************
Clears the Halt feature on the given endpoint.

Parameters:
  eptnum - Endpoint number.
******************************************************************************/
void HAL_Unhalt(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);

  // Check if the endpoint is enabled
  if (UDP_ENDPOINT_DISABLED != endpoint->state)
  {
    // Return endpoint to Idle state
    endpoint->state = UDP_ENDPOINT_IDLE;

    halEndpointUnHaltState(eptnum);
  }
}

/******************************************************************************
Returns the current Halt status of an endpoint.

Parameters:
  eptnum - Endpoint number.

Returns:
  1 - if the endpoint is currently halted;
  0 - otherwise.
******************************************************************************/
uint8_t HAL_IsHalted(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  if (UDP_ENDPOINT_HALTED == endpoint->state)
    return 1;
  else
    return 0;
}

/******************************************************************************
Causes the given endpoint to acknowledge the next packet it receives with
a STALL handshake.

Parameters:
  eptnum - Endpoint number.

Returns:
  STATUS_SUCCESS or STATUS_BUSY.
******************************************************************************/
uint8_t HAL_Stall(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);

  // Check that endpoint is in Idle state
  if (UDP_ENDPOINT_IDLE != endpoint->state)
  {
    return STATUS_BUSY;
  }

  halSendStallToHost(eptnum);
  // Enable the endpoint interrupt
  halEnableEndPointStallInterrupt(eptnum);

  return STATUS_SUCCESS;
}

/******************************************************************************
Sets the device address to the given value.

Parameters:
  address - New device address.
******************************************************************************/
void HAL_SetAddress(uint8_t *address)
{
  halSetUsbAddress(*address);
  // If the address is 0, the device returns to the Default state
  if (*address)
    deviceState = DEVICE_PREADDRESSED;
  // If the address is non-zero, the device enters the Address state
  else
    deviceState = DEVICE_DEFAULT;
}

/******************************************************************************
Sets the current device configuration.

Parameters:
  cfgnum - Configuration number to set.
******************************************************************************/
void HAL_SetConfiguration(uint8_t cfgnum)
{
  // If the configuration number if non-zero, the device enters the
  // Configured state
  if (cfgnum)
  {
    deviceState = DEVICE_CONFIGURED;
  }
  // If the configuration number is zero, the device goes back to the Address
  // state
  else
  {
    deviceState = DEVICE_ADDRESS;
    // Abort all transfers
    halDisableEndpoints();
  }
}

/******************************************************************************
Initializes the USB driver.

Parameters:
 reqMem - Memory for usb request. Memory allocate by user.
******************************************************************************/
void HAL_UsbInit(uint8_t *reqMem)
{
  requestMemory = reqMem;

  // Reset endpoint structures
  halResetEndpoints();

  // Device is in the Attached state
  deviceState = DEVICE_SUSPENDED;
  previousDeviceState = DEVICE_POWERED;
  halInitUsbDevice();
  halUsbInterrupt();
}

/******************************************************************************
Returns the current state of the USB device.

Returns:
  Device current state.
******************************************************************************/
uint8_t HAL_GetState(void)
{
  return deviceState;
}

/******************************************************************************
Endpoint interrupt handler.
******************************************************************************/
void halEndPointHandler(void)
{
  halCommonEndpointHandler();
}

/******************************************************************************
usb suspend interrupt handler.
******************************************************************************/
void halSuspendHandler(void)
{
  if (NULL != suspendCallback)
    suspendCallback();
}

/******************************************************************************
usb resume interrupt handler.
******************************************************************************/
void halResumeHandler(void)
{
  if (NULL != resumeCallback)
    resumeCallback();
}

/******************************************************************************
usb bus reset interrupt handler.
******************************************************************************/
void halBusResetHandler(void)
{
  halEndOfBusResetHandler();
  SYS_E_ASSERT_FATAL(endOfBusResetCallback, USB_NULLCALLBACK_1);
  endOfBusResetCallback();
}
#endif // defined(HAL_USE_USB)

// eof usb.c
