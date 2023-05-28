/**************************************************************************//**
\file  halUsart.c

\brief Implementation of usart hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_USART)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <sleep.h>
#include <usart.h>
#include <halSleepTimerClock.h>
#include <halAppClock.h>
#include <halIrq.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/
void halPostUsartTask(HalUsartTaskId_t taskId);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern volatile bool halEnableDtrWakeUp;
extern void (* dtrWakeUpCallback)(void);

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Sets USART module parameters.
  \param
    usartmode - pointer to HAL_UsartDescriptor_t
  \return
    none.
******************************************************************************/
void halSetUsartConfig(HAL_UsartDescriptor_t *usartMode)
{
  UCSRnB(usartMode->tty) = 0x00; // disable usart
  UBRRn(usartMode->tty) = usartMode->baudrate; // usart speed

  if (USART_MODE_ASYNC == usartMode->mode)
  {
    UCSRnA(usartMode->tty) = (uint8_t)USART_DOUBLE_SPEED << U2X0; // Double the USART Transmition Speed
    UCSRnC(usartMode->tty) = 0x00;
  }
  else
  {
    UCSRnA(usartMode->tty) = 0;
    UCSRnC(usartMode->tty) = usartMode->edge; // edge select
  }

  UCSRnC(usartMode->tty) |= usartMode->mode;
  UCSRnC(usartMode->tty) |= usartMode->dataLength;     // character size
  UCSRnC(usartMode->tty) |= usartMode->parity;   // parity mode
  UCSRnC(usartMode->tty) |= usartMode->stopbits; // stop bit select
  UCSRnA(usartMode->tty) |= (1 << RXC0); // clear receive interrupt
  UCSRnB(usartMode->tty) |= (1 << RXEN1) | (1 << TXEN1); // usart enable
  UCSRnB(usartMode->tty) |= (1 << RXCIE0) ; // receive interrupt enable
}

/**************************************************************************//**
 \brief The interrupt handler of USART0 - data register is empty.
******************************************************************************/
ISR(USART0_UDRE_vect)
{
  BEGIN_MEASURE
  // We must disable the interrupt because we must "break" context.
  halDisableUsartDremInterrupt(USART_CHANNEL_0);
  halWakeupFromIrq();
  halPostUsartTask(HAL_USART_TASK_USART0_DRE);
  END_MEASURE(HALISR_USART0_UDR_MEASURE_CODE)
}

/**************************************************************************//**
 \brief The interrupt handler of USART0 - transmission is completed.
******************************************************************************/
ISR(USART0_TX_vect)
{
  BEGIN_MEASURE
  halDisableUsartTxcInterrupt(USART_CHANNEL_0);
  halPostUsartTask(HAL_USART_TASK_USART0_TXC);
  END_MEASURE(HALISR_USART0_TX_MEASURE_CODE)
}

/**************************************************************************//**
 \brief The interrupt handler of USART0 - reception is completed.
******************************************************************************/
ISR(USART0_RX_vect)
{
  BEGIN_MEASURE
  uint8_t  status = UCSR0A;
  uint8_t  data = UDR0;

  if (!(status & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))))
  {
    halWakeupFromIrq();
    halUsartRxBufferFiller(USART_CHANNEL_0, data);
    halPostUsartTask(HAL_USART_TASK_USART0_RXC);
  }
  #if defined(_USE_USART_ERROR_EVENT_)
    else // There is an error in the received byte.
    {
      halUsartSaveErrorReason(USART_CHANNEL_0, status);
      halPostUsartTask(HAL_USART_TASK_USART0_ERR);
    }
  #endif

  END_MEASURE(HALISR_USART0_RX_MEASURE_CODE)
}

/**************************************************************************//**
 \brief The interrupt handler of USART1 - data register is empty.
******************************************************************************/
ISR(USART1_UDRE_vect)
{
  BEGIN_MEASURE
  // We must disable the interrupt because we must "break" context.
  halDisableUsartDremInterrupt(USART_CHANNEL_1);
  halWakeupFromIrq();
  halPostUsartTask(HAL_USART_TASK_USART1_DRE);
  END_MEASURE(HALISR_USART1_UDRE_MEASURE_CODE)
}

/**************************************************************************//**
 \brief The interrupt handler of USART1 - transmission is completed.
******************************************************************************/
ISR(USART1_TX_vect)
{
  BEGIN_MEASURE
  halDisableUsartTxcInterrupt(USART_CHANNEL_1);
  halPostUsartTask(HAL_USART_TASK_USART1_TXC);
  END_MEASURE(HALISR_USART1_TX_MEASURE_CODE)
}

/**************************************************************************//**
 \brief The interrupt handler of USART1 - reception is completed.
******************************************************************************/
ISR(USART1_RX_vect)
{
  BEGIN_MEASURE
  uint8_t  status = UCSR1A;
  uint8_t  data = UDR1;

  if (!(status & ((1 << FE1) | (1 << DOR1) | (1 << UPE1))))
  {
    halWakeupFromIrq();
    halUsartRxBufferFiller(USART_CHANNEL_1, data);
    halPostUsartTask(HAL_USART_TASK_USART1_RXC);
  }
  #if defined(_USE_USART_ERROR_EVENT_)
    else // There is an error in the received byte.
    {
      halUsartSaveErrorReason(USART_CHANNEL_1, status);
      halPostUsartTask(HAL_USART_TASK_USART1_ERR);
    }
  #endif
  END_MEASURE(HALISR_USART1_RX_MEASURE_CODE)
}

#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT

#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT
#endif // defined(HAL_USE_USART)

// eof halUsart.c
