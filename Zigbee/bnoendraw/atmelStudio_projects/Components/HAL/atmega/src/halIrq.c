/**************************************************************************//**
  \file  halIrq.c

  \brief Implementation of HWD IRQ interface.

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

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halIrq.h>
#include <sleep.h>
#include <halSleepTimerClock.h>
#include <halAppClock.h>
#include <halDbg.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Global variables section
******************************************************************************/
#if defined(HAL_USE_EXT_IRQ)
IrqCallback_t IrqCallbackList[HAL_NUM_IRQ_LINES] =
  {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#endif // HAL_USE_EXT_IRQ

#if defined(HAL_USE_PC_IRQ)
static uint8_t pinChangeMask;
IrqCallback_t PCIrqCallbackList[HAL_NUM_PC_IRQ_LINES] =
  {NULL, NULL};
#endif // HAL_USE_PC_IRQ

/******************************************************************************
                   Implementations section
******************************************************************************/
#if defined(HAL_USE_EXT_IRQ)
/******************************************************************************
Sets configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
  irqMode - mode of interrupt
Returns:
  none.
******************************************************************************/
void halSetIrqConfig(uint8_t irqNumber, uint8_t irqMode)
{
  if (irqNumber < IRQ_4)
  {
    uint8_t shift = irqNumber << 1;

    DDRD &= ~(1 << irqNumber); // IRQ pin is input
    PORTD |= (1 << irqNumber); // Enable pullup
    // Setup corresponding interrupt sence control
    EICRA &= ~(3 << shift);
    EICRA |= (irqMode & 0x03) << shift;
  }
  else
  {
    uint8_t shift = (irqNumber - IRQ_4) << 1;

    DDRE &= ~(1 << irqNumber); // IRQ pin is input
    PORTE |= (1 << irqNumber); // Enable pullup
    // Setup interrupt sence control
    EICRB &= ~(3 << shift);
    EICRB |= (irqMode & 0x03) << shift;
  }

  // Clear the INTn interrupt flag
  EIFR |= (1 << irqNumber);
}

/**************************************************************************//**
\brief Clears configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
******************************************************************************/
void halClrIrqConfig(uint8_t irqNumber)
{
  if (irqNumber < IRQ_4)
  {
    uint8_t shift = irqNumber << 1;

    DDRD &= ~(1 << irqNumber);  // IRQ pin is input
    PORTD &= ~(1 << irqNumber); // Disable pullup
    EICRA &= ~(3 << shift);     // Clear interrupt sence control
  }
  else
  {
    uint8_t shift = (irqNumber - IRQ_4) << 1;

    DDRE &= ~(1 << irqNumber);  // IRQ pin is input
    PORTE &= ~(1 << irqNumber); // Disable pullup
    EICRB &= ~(3 << shift);     // Clear interrupt sence control
  }
}

/******************************************************************************
 External interrupt 0 handler
******************************************************************************/
ISR(INT0_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_0 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT0_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 1 handler
******************************************************************************/
ISR(INT1_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_1 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT1_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 2 handler
******************************************************************************/
ISR(INT2_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_2 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT2_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 3 handler
******************************************************************************/
ISR(INT3_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_3 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT3_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 4 handler
******************************************************************************/
ISR(INT4_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_4 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT4_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 5 handler
******************************************************************************/
ISR(INT5_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_5 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT5_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 6 handler
******************************************************************************/
ISR(INT6_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_6 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT6_VECT_MEASURE_CODE)
}

/******************************************************************************
 External interrupt 7 handler
******************************************************************************/
ISR(INT7_vect)
{
  BEGIN_MEASURE
  halWakeupFromIrq();
  IrqCallbackList[IRQ_7 - HAL_FIRST_VALID_IRQ]();
  END_MEASURE(HALISR_INT7_VECT_MEASURE_CODE)
}
#endif // defined(HAL_USE_EXT_IRQ)

#if defined (HAL_USE_PC_IRQ)
/******************************************************************************
Sets configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
Returns:
  none.
******************************************************************************/
void halSetPCIrqConfig(uint8_t irqNumber, uint8_t pinMask)
{
  if (irqNumber < PCIRQ_1)
  {
    uint8_t index = 0;
    pinChangeMask = pinMask;
    while (pinMask)
    {
      if (pinMask & 0x01)
      {
        DDRB &= ~(1 << index); // IRQ pin is input
        PORTB |= (1 << index); // Enable pullup
        PCMSK0 |= (1 << index); // Enable pin change
      }
      pinMask >>= 1;
      index++;
    }
  }
  else
  {
    DDRE &= ~0x01; // IRQ pin is input
    PORTE |= 0x01; // Enable pullup
    PCMSK1 |= 0x01; // Enable pin change
  }

  // Clear the INTn interrupt flag
  PCIFR |= (1 << irqNumber);
}

/**************************************************************************//**
\brief Clears configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
******************************************************************************/
void halClrPCIrqConfig(uint8_t irqNumber)
{
  if (irqNumber < PCIRQ_1)
  {
    uint8_t index = 0;
    while (pinChangeMask)
    {
      if (pinChangeMask & 0x01)
      {
        DDRB &= ~(1 << index);  // IRQ pin is input
        PORTB &= ~(1 << index); // Disable pullup
        PCMSK0 &= ~(1 << index); // Disable pin change

      }
      pinChangeMask >>= 1;
      index++;
    }
  }
  else
  {
    DDRE &= ~1;  // IRQ pin is input
    PORTE &= ~1; // Disable pullup
    PCMSK1 &= ~1; // Disable pin change

  }
}
/******************************************************************************
 Pin change interrupt 0 handler
******************************************************************************/
ISR(PCINT0_vect)
{
  halWakeupFromIrq();
  PCIrqCallbackList[PCIRQ_0 - HAL_FIRST_VALID_PC_IRQ]();
}
/******************************************************************************
 Pin change interrupt 1 handler
******************************************************************************/
ISR(PCINT1_vect)
{
  halWakeupFromIrq();
  PCIrqCallbackList[PCIRQ_1 - HAL_FIRST_VALID_PC_IRQ]();
}
#endif // HAL_USE_PC_IRQ

// eof irq.c
