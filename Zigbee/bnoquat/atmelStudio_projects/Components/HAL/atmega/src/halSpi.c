/**************************************************************************//**
\file  halSpi.c

\brief Implementation of USART SPI mode.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/06/07 E. Ivanov - Created
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

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define UDORD0                  2
#define UCPHA0                  1
#define UCPOL0                  0
#define SPI_CLOCK_MODE_AMOUNT   4
#define SPI_DATA_ORDER_AMOUNT   2

/******************************************************************************
                   Global functions prototypes section
******************************************************************************/
void halSpiRxBufferFiller(uint8_t data);
void halSpiTxByteComplete(void);

/******************************************************************************
                   Global variables section
******************************************************************************/
HAL_SpiDescriptor_t *halRealSpiDescripPointer = NULL;
volatile uint8_t rxSlaveBuffer[HAL_SPI_RX_BUFFER_LENGTH];

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Set the parameters of USART to work at SPI mode.
Parameters:
  descriptor - pointer to the spi descriptor.
******************************************************************************/
void halSetUsartSpiConfig(HAL_SpiDescriptor_t *descriptor)
{
  uint8_t clockMode[SPI_CLOCK_MODE_AMOUNT] = {((0 << UCPOL0) | (0 << UCPHA0)),
                                              ((0 << UCPOL0) | (1 << UCPHA0)),
                                              ((1 << UCPOL0) | (0 << UCPHA0)),
                                              ((1 << UCPOL0) | (1 << UCPHA0))};
  uint8_t dataOrder[SPI_DATA_ORDER_AMOUNT] = {(0 << UDORD0),
                                              (1 << UDORD0)};

  // setting of the spi gpio direct
  if (SPI_CHANNEL_0 == descriptor->tty)
    GPIO_USART0_EXTCLK_make_out();
  else
    GPIO_USART1_EXTCLK_make_out();

  UBRRn(descriptor->tty) = 0;
  // Set MSPI mode
  UCSRnC(descriptor->tty) = (1 << UMSEL01) | (1 << UMSEL00);
  // Set clock mode and data order
  UCSRnC(descriptor->tty) |= (dataOrder[descriptor->dataOrder] | clockMode[descriptor->clockMode]);
  // Enable receiver and transmitter
  UCSRnB(descriptor->tty) = (1 << RXEN0) | (1 << TXEN0);
  // Set baud rate
  UBRRn(descriptor->tty) = descriptor->baudRate;
}

/******************************************************************************
Disables USART channel.
Parameters:
  tty  -  spi channel.
******************************************************************************/
void halClearUsartSpi(SpiChannel_t tty)
{
  if (SPI_CHANNEL_0 == tty)
    GPIO_USART0_EXTCLK_make_in();
  else
    GPIO_USART1_EXTCLK_make_in();

  UCSRnB(tty) = 0x00; // disable
}

/******************************************************************************
Set the parameters of the SPI in slave mode.
Parameters:
  descriptor - pointer to the spi descriptor.
******************************************************************************/
void halSetSlaveSpiConfig(HAL_SpiDescriptor_t *descriptor)
{
  uint8_t clockMode[SPI_CLOCK_MODE_AMOUNT] = {((0 << CPOL) | (0 << CPHA)),
                                              ((0 << CPOL) | (1 << CPHA)),
                                              ((1 << CPOL) | (0 << CPHA)),
                                              ((1 << CPOL) | (1 << CPHA))};
  uint8_t dataOrder[SPI_DATA_ORDER_AMOUNT] = {(0 << DORD),
                                              (1 << DORD)};

  // setting of the spi gpio direct
  GPIO_SPI_MISO_make_out();
  GPIO_SPI_MOSI_make_in();
  GPIO_SPI_SCK_make_in();
  GPIO_HW_SPI_CS_make_in();
  GPIO_HW_SPI_CS_make_pullup();

  // dummy read for reading from SPSR SPIF 
  if (SPSR & (1<<SPIF))
  {
   SPDR = 0x00;
  }
  SPCR = (1 << SPE) | clockMode[descriptor->clockMode] | dataOrder[descriptor->dataOrder] | (1 << SPIE);
}

/******************************************************************************
Write a length bytes to the USART SPI.
Parameters:
  tty    -  spi channel
  buffer -  pointer to application data buffer;
  length -  number bytes for transfer;
Returns:
  number of written bytes
******************************************************************************/
uint16_t halSyncUsartSpiWriteData(SpiChannel_t tty, uint8_t *buffer, uint16_t length)
{
  uint16_t i;
  uint8_t temp;

  for (i = 0; i < length; i++)
  {
    // Wait for empty transmit buffer
    while (!(UCSRnA(tty) & (1 << UDRE0)));
    // Send data
    UDRn(tty) = *(buffer + i);
    // Wait for data to be received
    while (!(UCSRnA(tty) & (1 << RXC0)));
    // receives data to clear received usart buffer
    temp = UDRn(tty);
    (void)temp;
  }
  return i;
}

/******************************************************************************
Write & read a length bytes to & from the SPI.
Parameters:
  tty    -  spi channel
  buffer -  pointer to application data buffer;
  length -  number bytes for transfer;
Returns:
  number of written & read bytes
******************************************************************************/
uint16_t halSyncUsartSpiReadData(SpiChannel_t tty, uint8_t *buffer, uint16_t length)
{
  uint16_t i;

  for (i = 0; i < length; i++)
  {
    // Wait for empty transmit buffer
    while (!(UCSRnA(tty) & (1 << UDRE0)));
    // Send data
    UDRn(tty) = *(buffer + i);
    // Wait for data to be received
    while (!(UCSRnA(tty) & (1 << RXC0)));
    // Receive data
    *(buffer + i) = UDRn(tty);
  }
  return i;
}

/**************************************************************************//**
\brief SPI interrupt handler
******************************************************************************/
ISR(SPI_STC_vect)
{
  BEGIN_MEASURE
  uint8_t data = SPDR;

  halSpiTxByteComplete();
  halSpiRxBufferFiller(data);
  halPostTask(HAL_TASK_SPI);
  END_MEASURE(HALISR_SPI_MEASURE_CODE)
}
#endif // defined(HAL_USE_SPI) || defined(HAL_USE_USART)

//end of halSpi.c
