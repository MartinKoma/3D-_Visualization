/**************************************************************************//**
  \file  spi.c

  \brief Implementation of USART SPI mode, hardware-independent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/05/08 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_SPI) || defined(HAL_USE_USART)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <spi.h>
#include <halDiagnostic.h>
#include <sysAssert.h>

/******************************************************************************
                   Global functions prototypes section
******************************************************************************/
void halSetUsartSpiConfig(HAL_SpiDescriptor_t *descriptor);
void halSetSlaveSpiConfig(HAL_SpiDescriptor_t *descriptor);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_UsartDescriptor_t *halPointDescrip[NUM_USART_CHANNELS];
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
  extern HAL_SpiDescriptor_t *halRealSpiDescripPointer;
  extern volatile uint8_t rxSlaveBuffer[HAL_SPI_RX_BUFFER_LENGTH];
#endif

/******************************************************************************
                   Static functions prototypes section
******************************************************************************/
static bool isClosedPd(void *pointer);
static bool isOpenedPd(void *pointer);
static int halCheckUsartDescriptor(HAL_SpiDescriptor_t *descriptor, bool(* predicate)(void *));
static int halOpenUsartSpi(HAL_SpiDescriptor_t *descriptor);
static int halCloseUsartSpi(HAL_SpiDescriptor_t *descriptor);
static int halFillServiceInfo(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length, uint8_t transac);
static int halWriteUsartSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length);
static int halReadUsartSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length);
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
static int halCheckSpiDescriptor(HAL_SpiDescriptor_t *descriptor, bool(* predicate)(void *));
static int halOpenRealSpi(HAL_SpiDescriptor_t *descriptor);
static int halCloseRealSpi(HAL_SpiDescriptor_t *descriptor);
static int halWriteRealSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length);
static int halReadRealSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length);
#endif

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Predicate for check of closed station.
\param[in]
    pointer - pointer to some descriptor.
\return
   true - interface is closed;
   false - interface is not closed.
******************************************************************************/
static bool isClosedPd(void *pointer)
{
  return pointer ? false : true;
}

/**************************************************************************//**
\brief Predicate for check of opened station.
\param[in]
    pointer - pointer to some descriptor.
\return
   true - interface is opened;
   false - interface is not opened.
******************************************************************************/
static bool isOpenedPd(void *pointer)
{
  return pointer ? true : false;
}

/**************************************************************************//**
\brief Check correctenss of the usart spi descriptor.
\param[in]
   descriptor - pointer to the usart spi descriptor.
\param[in]
   predicate - check station predicate.
\return
   interface index - interface is opened;
   -1 - interface is not opened.
******************************************************************************/
static int halCheckUsartDescriptor(HAL_SpiDescriptor_t *descriptor, bool(* predicate)(void *))
{
  int i;

  if (NULL == descriptor)
    return -1;

  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (false == predicate((void *)halPointDescrip[i]))
    return -1;

  return i;
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Check correctenss of the real spi descriptor.
\param[in]
   descriptor - pointer to the real spi descriptor.
\param[in]
   predicate - check station predicate.
\return
    0 - interface is opened;
   -1 - interface is not opened.
******************************************************************************/
static int halCheckSpiDescriptor(HAL_SpiDescriptor_t *descriptor, bool(* predicate)(void *))
{
  if (NULL == descriptor)
    return -1;

  if (SPI_CHANNEL_2 != descriptor->tty)
    return -1;

  if (false == predicate((void *)halRealSpiDescripPointer))
    return -1;

  return 0;
}
#endif

/**************************************************************************//**
\brief Configure usart in mspi mode.
\param[in]
    descriptor - pointer to the usart spi descriptor.
\return
   -1 - there not are free resources.
    0 - SPI channel is ready.
******************************************************************************/
static int halOpenUsartSpi(HAL_SpiDescriptor_t *descriptor)
{
  int i; // Descriptor index

  i = halCheckUsartDescriptor(descriptor, isClosedPd);
  if (-1 == i)
    return -1;

  if (NULL != descriptor->callback)
  {
    descriptor->spiDescriptor.txCallback = descriptor->callback;
    descriptor->spiDescriptor.tty = descriptor->tty;
  }

  halPointDescrip[i] = &descriptor->spiDescriptor;
  halSetUsartSpiConfig(descriptor);
  return 0;
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Configure spi.
\param[in]
    descriptor - pointer to the spi descriptor.
\return
   -1 - there are no free resources.
    0 - SPI channel is ready.
******************************************************************************/
static int halOpenRealSpi(HAL_SpiDescriptor_t *descriptor)
{
  if (-1 == halCheckSpiDescriptor(descriptor, isClosedPd))
    return -1;

  halRealSpiDescripPointer = descriptor;
  halRealSpiDescripPointer->spiDescriptor.service.txPointOfRead = 0;
  halRealSpiDescripPointer->spiDescriptor.service.txPointOfWrite = 0;
  halRealSpiDescripPointer->spiDescriptor.service.rxPointOfRead = 0;
  halRealSpiDescripPointer->spiDescriptor.service.rxPointOfWrite = 0;
  halSetSlaveSpiConfig(descriptor);
  return 0;
}
#endif

/**************************************************************************//**
\brief Open the SPI interface.
\param[in]
    descriptor - pointer to the spi descriptor.
\return
   -1 - there not are free resources.
    0 - SPI channel is ready.
******************************************************************************/
int HAL_OpenSpi(HAL_SpiDescriptor_t *descriptor)
{
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
  if (SPI_CHANNEL_2 == descriptor->tty)
    return halOpenRealSpi(descriptor);
  else
#endif
    return halOpenUsartSpi(descriptor);
}

/**************************************************************************//**
\brief Clear the usart channel and pins.
\param[in]
  descriptor - pointer to the spi descriptor.
\return
   0 - success
  -1 - channel was not opened.
******************************************************************************/
static int halCloseUsartSpi(HAL_SpiDescriptor_t *descriptor)
{
  int i;

  i = halCheckUsartDescriptor(descriptor, isOpenedPd);
  if (-1 == i)
    return -1;

  halPointDescrip[i] = NULL;
  halClearUsartSpi(descriptor->tty);
  return 0;
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Clear the spi channel and pins.
\param[in]
  descriptor - pointer to the spi descriptor.
\return
  Returns 0 on success or -1 if channel was not opened.
******************************************************************************/
static int halCloseRealSpi(HAL_SpiDescriptor_t *descriptor)
{
  if (-1 == halCheckSpiDescriptor(descriptor, isClosedPd))
    return -1;

  halRealSpiDescripPointer = NULL;
  halClearRealSpi();
  return 0;
}
#endif

/**************************************************************************//**
\brief Close the SPI channel and pins.
\param[in]
  descriptor - pointer to the spi descriptor.
\return
  Returns 0 on success or -1 if channel was not opened.
******************************************************************************/
int HAL_CloseSpi(HAL_SpiDescriptor_t *descriptor)
{
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
  if (SPI_CHANNEL_2 == descriptor->tty)
    return halCloseRealSpi(descriptor);
  else
#endif
    return halCloseUsartSpi(descriptor);
}

/**************************************************************************//**
\brief Fill service structure for bus transaction.
\param[in]
   descriptor - pointer to the spi descriptor.
\param[in]
   buffer -pointer to data buffer.
\param[in]
   length - length of the data buffer.
\param[in]
   transac - bus transaction type.
\return
   -1 - interface is busy;
    0 - success.
******************************************************************************/
static int halFillServiceInfo(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length, uint8_t transac)
{
  HalUsartService_t *halBufferControl;

  halBufferControl = &descriptor->spiDescriptor.service;
  if (halBufferControl->txPointOfWrite != halBufferControl->txPointOfRead)
    return -1; // there is unsent data

  descriptor->spiDescriptor.txBuffer = buffer;
  descriptor->spiDescriptor.txBufferLength = 0;
  halBufferControl->txPointOfWrite = length;
  halBufferControl->txPointOfRead = 0;
  descriptor->spiDescriptor.rxBuffer = buffer;
  descriptor->spiDescriptor.flowControl = transac;
  return 0;
}

/**************************************************************************//**
\brief Writes a length bytes to the usart. \n
 Callback function will be used to notify about the finishing transmitting.
\param[in]
  descriptor -  pointer to spi descriptor
\param[in]
  buffer     -  pointer to application data buffer;
\param[in]
  length     -  number bytes for transfer;
\return
  -1 - spi module was not opened, there is unsent data, pointer to the data or
       the length are zero; \n
   0 - on success or a number; \n
   Number of written bytes if the synchronous method is used(callback is NULL).
******************************************************************************/
static int halWriteUsartSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  int i;

  if (!buffer || !length)
    return -1;

  i = halCheckUsartDescriptor(descriptor, isOpenedPd);
  if (-1 == i)
    return -1;

  if (&descriptor->spiDescriptor != halPointDescrip[i])
    return -1; // incorrect descriptor

  if (NULL != descriptor->callback)
  {
    if (-1 == halFillServiceInfo(descriptor, buffer, length, USART_SPI_WRITE_MODE))
      return -1;

    halEnableUsartSpiRxcInterrupt(descriptor->tty);
    halEnableUsartSpiDremInterrupt(descriptor->tty);
    return 0;
  }
  else
  {
    return halSyncUsartSpiWriteData(descriptor->tty, buffer, length);
  }
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Writes a length bytes to the spi. \n
\param[in]
  descriptor -  pointer to spi descriptor
\param[in]
  buffer     -  pointer to application data buffer;
\param[in]
  length     -  number bytes for transfer;
\return
  -1 - spi module was not opened, there is unsent data, pointer to the data or
       the length are zero; \n
   0 - on success or a number; \n
******************************************************************************/
static int halWriteRealSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  if (!buffer || !length)
    return -1;

  if (-1 == halCheckSpiDescriptor(descriptor, isOpenedPd))
    return -1;

  if (descriptor != halRealSpiDescripPointer)
    return -1; // incorrect descriptor

  if (-1 == halFillServiceInfo(descriptor, buffer, length, USART_SPI_WRITE_MODE))
    return -1;

  halSendSpiByte(*buffer);
  return 0;
}
#endif

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Put next byte to the spi.
******************************************************************************/
void halSpiTxByteComplete(void)
{
  HalUsartService_t *halBufferControl;

  halBufferControl = &halRealSpiDescripPointer->spiDescriptor.service;

  if (halBufferControl->txPointOfWrite != halBufferControl->txPointOfRead)
    halSendSpiByte(halRealSpiDescripPointer->spiDescriptor.txBuffer[++halBufferControl->txPointOfRead]);
}
#endif

/**************************************************************************//**
\brief Writes a length bytes to the SPI. \n
 Callback function will be used to notify about the finishing transmitting.
 (only for master spi)
\param[in]
  descriptor -  pointer to spi descriptor
\param[in]
  buffer     -  pointer to application data buffer;
\param[in]
  length     -  number bytes for transfer;
\return
  -1 - spi module was not opened, there is unsent data, pointer to the data or
       the length are zero; \n
   0 - on success or a number; \n
   Number of written bytes if the synchronous method is used(callback is NULL), \n
   only for master spi.
******************************************************************************/
int HAL_WriteSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
  if (SPI_CHANNEL_2 == descriptor->tty)
    return halWriteRealSpi(descriptor, buffer, length);
  else
#endif
    return halWriteUsartSpi(descriptor, buffer, length);
}

/**************************************************************************//**
\brief Reads a number of bytes from the usart.\n
 Callback function will be used to notify when the activity is finished.\n
 The read data is placed to the buffer.
\param[in]
  descriptor - pointer to HAL_SpiDescriptor_t structure
\param[in]
  buffer -  pointer to the application data buffer
\param[in]
  length -  number of bytes to transfer
\return
  -1 - spi module was not opened, or there is unsent data, or the pointer to
   data or the length are NULL; \n
   0 - success; \n
   Number of written bytes if the synchronous method is used(callback is NULL).
******************************************************************************/
static int halReadUsartSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  HAL_UsartDescriptor_t *spiDescrip;
  int i;

  if (!buffer || !length)
    return -1;

  i = halCheckUsartDescriptor(descriptor, isOpenedPd);
  if (-1 == i)
    return -1;

  spiDescrip = &descriptor->spiDescriptor;
  if (spiDescrip != halPointDescrip[i])
    return -1; // incorrect descriptor

  if (NULL != descriptor->callback)
  {
    if (-1 == halFillServiceInfo(descriptor, buffer, length, USART_SPI_READ_MODE))
      return -1;

    halEnableUsartSpiRxcInterrupt(descriptor->tty);
    halEnableUsartSpiDremInterrupt(descriptor->tty);
    return 0;
  }
  else
  {
    return halSyncUsartSpiReadData(descriptor->tty, buffer, length);
  }
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Reads a number of bytes from spi internal buffer and places them to the buffer.
\param[in]
  descriptor - pointer to HAL_SpiDescriptor_t structure
\param[in]
  buffer -  pointer to the application data buffer
\param[in]
  length -  number of bytes to transfer
\return
  -1 - spi module was not opened, or there is unsent data, or the pointer to
   data or the length are NULL; \n
   Number of read bytes from spi internal buffer.
******************************************************************************/
static int halReadRealSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  uint16_t           wasRead = 0;
  uint16_t           poW;
  uint16_t           poR;
  HalUsartService_t *halBufferControl;

  if (!buffer || !length)
    return -1;

  if (-1 == halCheckSpiDescriptor(descriptor, isOpenedPd))
    return -1;

  if (descriptor != halRealSpiDescripPointer)
    return -1; // incorrect descriptor

  halBufferControl = &halRealSpiDescripPointer->spiDescriptor.service;
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    poW = halBufferControl->rxPointOfWrite;
    poR = halBufferControl->rxPointOfRead;
  END_MEASURE(HAL_SPI_RX1_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  while ((poR != poW) && (wasRead < length))
  {
    buffer[wasRead] = rxSlaveBuffer[poR];
    if (HAL_SPI_RX_BUFFER_LENGTH == ++poR)
      poR = 0;
    wasRead++;
  }

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    halBufferControl->rxPointOfRead = poR;
    halBufferControl->rxBytesInBuffer -= wasRead;
  END_MEASURE(HAL_SPI_RX2_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  return wasRead;
}
#endif

/**************************************************************************//**
\brief For master : writes a number of bytes to the spi.\n
  Callback function will be used to notify when the activity is finished.\n
  The read data is placed to the buffer. \n
  For slave: reads a number of bytes from internal spi buffer and writes them \n
  to application buffer.
\param[in]
  descriptor - pointer to HAL_SpiDescriptor_t structure
\param[in]
  buffer -  pointer to the application data buffer
\param[in]
  length -  number of bytes to transfer
\return
  -1 - spi module was not opened, or there is unsent data, or the pointer to
   data or the length are NULL; \n
   0 - success for master; \n
   Number of written bytes if the synchronous method is used(callback is NULL) for master \n
   or number of read bytes from internal buffer to the application buffer for slave.
******************************************************************************/
int HAL_ReadSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
  if (SPI_CHANNEL_2 == descriptor->tty)
    return halReadRealSpi(descriptor, buffer, length);
  else
#endif
    return halReadUsartSpi(descriptor, buffer, length);
}

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Puts the received byte to the cyclic buffer.

\param[in]
  data - data to put.
******************************************************************************/
void halSpiRxBufferFiller(uint8_t data)
{
  uint16_t           old;
  HalUsartService_t *halBufferControl;

  if (NULL == halRealSpiDescripPointer)
  {// abnormal
    halClearRealSpi();
    return;
  }

  halBufferControl = &halRealSpiDescripPointer->spiDescriptor.service;
  old = halBufferControl->rxPointOfWrite;

  if (HAL_SPI_RX_BUFFER_LENGTH == ++halBufferControl->rxPointOfWrite)
    halBufferControl->rxPointOfWrite = 0;

  if (halBufferControl->rxPointOfWrite == halBufferControl->rxPointOfRead)
  { // Buffer full.
    halBufferControl->rxPointOfWrite = old;
    return;
  } // Buffer full.

  rxSlaveBuffer[old] = data;
  halBufferControl->rxBytesInBuffer++;
}
#endif

#if defined(ATMEGA128RFA1) || defined(ATMEGA256RFR2) || defined(ATMEGA2564RFR2)
/**************************************************************************//**
\brief Slave spi reception complete interrupt handler.
******************************************************************************/
void halSpiRxByteComplete(void)
{
  uint16_t number;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    number = halRealSpiDescripPointer->spiDescriptor.service.rxBytesInBuffer;
  END_MEASURE(HAL_SPI_RX3_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  if (number)
  {
    SYS_E_ASSERT_FATAL(halRealSpiDescripPointer->slave_callback, SPI_NULLCALLBACK_0);
    halRealSpiDescripPointer->slave_callback(number);
  }
}
#endif
#endif // defined(HAL_USE_SPI) || defined(HAL_USE_USART)

// eof spi.c

