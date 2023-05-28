/*****************************************************************************//**
\file  halSpi.h

\brief  Declarations of USART SPI mode.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/06/07 E. Ivanov - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _HALSPI_H
#define _HALSPI_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <usart.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define SPI_CHANNEL_0 USART_CHANNEL_0  // USART0 atmega1281rfa1 start addresss
#define SPI_CHANNEL_1 USART_CHANNEL_1  // USART1 atmega1281rfa1 start addresss
#define SPI_CHANNEL_2 0x4C
#define HAL_SPI_RX_BUFFER_LENGTH      10

/******************************************************************************
                   Types section
******************************************************************************/
// spi channel
typedef UsartChannel_t SpiChannel_t;

// types of the clock mode
typedef enum
{
  // leading edge sample RX bit (rising), trailing edge setup TX bit (falling).
  SPI_CLOCK_MODE0,
  // leading edge setup TX bit (rising), trailing edge sample RX bit (falling).
  SPI_CLOCK_MODE1,
  // leading edge sample RX bit (falling), trailing edge setup TX bit (rising).
  SPI_CLOCK_MODE2,
  // leading edge setup TX bit (falling), trailing edge sample RX bit (rising).
  SPI_CLOCK_MODE3,
} SpiClockMode_t;

// clock rate
typedef enum
{
  SPI_CLOCK_RATE_62 =  ((F_CPU / (2 * 62500ul)) - 1),
  SPI_CLOCK_RATE_125 =  ((F_CPU / (2 * 125000ul)) - 1),
  SPI_CLOCK_RATE_250 =  ((F_CPU / (2 * 250000ul)) - 1),
  SPI_CLOCK_RATE_500 =  ((F_CPU / (2 * 500000ul)) - 1),
  SPI_CLOCK_RATE_1000 = ((F_CPU / (2 * 1000000ul)) - 1),
  SPI_CLOCK_RATE_2000 = ((F_CPU / (2 * 2000000ul)) - 1)
} SpiBaudRate_t;

// Data order
typedef enum
{
  SPI_DATA_MSB_FIRST, // data with MSB first
  SPI_DATA_LSB_FIRST  // data with LSB first
} SpiDataOrder_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Disables USART channel.
Parameters:
  tty  -  spi channel.
******************************************************************************/
void halClearUsartSpi(SpiChannel_t tty);

/******************************************************************************
Write a length bytes to the SPI.
Parameters:
  tty    -  spi channel
  buffer -  pointer to application data buffer;
  length -  number bytes for transfer;
Returns:
  number of written bytes
******************************************************************************/
uint16_t halSyncUsartSpiWriteData(SpiChannel_t tty, uint8_t *buffer, uint16_t length);

/******************************************************************************
Write & read a length bytes to & from the SPI.
Parameters:
  tty    -  spi channel
  buffer -  pointer to application data buffer;
  length -  number bytes for transfer;
Returns:
  number of written & read bytes
******************************************************************************/
uint16_t halSyncUsartSpiReadData(SpiChannel_t tty, uint8_t *buffer, uint16_t length);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/**************************************************************************//**
\brief Puts byte to data register of USART

\param[in] data - byte to send.
******************************************************************************/
static inline void halSendSpiByte(uint8_t data)
{
  SPDR = data;
}

/******************************************************************************
Disables SPI channel.
Parameters:
  tty  -  spi channel.
******************************************************************************/
static inline void halClearRealSpi(void)
{
  GPIO_SPI_MISO_make_in();
  SPCR = 0x00; // disable
}

/******************************************************************************
Enables data register empty interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halEnableUsartSpiDremInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) |= (1 << UDRIE0);
}

/******************************************************************************
Disables data register empty interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halDisableUsartSpiDremInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) &= ~(1 << UDRIE0);
}

/******************************************************************************
Enables transmit complete interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halEnableUsartSpiTxcInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) |=  (1 << TXCIE0);
}

/******************************************************************************
Disables transmit complete interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halDisableUsartSpiTxcInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) &=  ~(1 << TXCIE0);
}

/*****************************************************************************
Enables receive complete interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halEnableUsartSpiRxcInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) |= (1 << RXCIE0);
}

/*****************************************************************************
Disables receive complete interrupt.
Parameters:
  tty  -  spi channel.
Returns:
  none.
******************************************************************************/
static inline void halDisableUsartSpiRxcInterrupt(SpiChannel_t tty)
{
  UCSRnB(tty) &= ~(1 << RXCIE0);
}

#endif

//eof halSpi.h
