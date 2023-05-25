/***************************************************************************//**
  \file  halIrq.h

  \brief Declaration of HWD IRQ interface.

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

#ifndef _HALIRQ_H
#define _HALIRQ_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halTaskManager.h>
#include <irq.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/** \brief number valid interrupt. */
#define HAL_NUM_IRQ_LINES 8
/** \brief number valid interrupt. */
#define HAL_NUM_PC_IRQ_LINES 2
/** \brief first valid interrupt. */
#define HAL_FIRST_VALID_IRQ IRQ_0
/** \brief first valid interrupt. */
#define HAL_FIRST_VALID_PC_IRQ PCIRQ_0


/******************************************************************************
                   Types section
******************************************************************************/
/** \brief user's callback type. */
typedef void (* IrqCallback_t)(void);

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Sets configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
\param[in]
  irqMode - mode of interrupt.
******************************************************************************/
void halSetIrqConfig(uint8_t irqNumber, uint8_t irqMode);

/**************************************************************************//**
\brief Clears configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
******************************************************************************/
void halClrIrqConfig(uint8_t irqNumber);

/**************************************************************************//**
\brief Sets configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
\param[in]
  irqMode - mode of interrupt.
******************************************************************************/
void halSetPCIrqConfig(uint8_t irqNumber, uint8_t pinMask);

/**************************************************************************//**
\brief Clears configuration of pins and the registers.
\param[in]
  irqNumber - number of interrupt.
******************************************************************************/
void halClrPCIrqConfig(uint8_t irqNumber);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/**************************************************************************//**
\brief Enables external interrupt
\param[in]
  irqNumber - number of external interrupt.
******************************************************************************/
INLINE void halEnableIrqInterrupt(uint8_t irqNumber)
{
  // Enable external interrupt request
  EIMSK |= (1 << irqNumber);
}

/**************************************************************************//**
\brief Disables external interrupt
\param[in]
  irqNumber - number of external interrupt.
******************************************************************************/
INLINE void halDisableIrqInterrupt(uint8_t irqNumber)
{
  // Disable external interrupt request
  EIMSK &= ~(1 << irqNumber);
}

/**************************************************************************//**
\brief Enables external interrupt
\param[in]
  irqNumber - number of external interrupt.
******************************************************************************/
INLINE void halEnablePCIrqInterrupt(uint8_t irqNumber)
{
  // Enable external interrupt request
  PCICR |= (1 << irqNumber);
}

/**************************************************************************//**
\brief Disables external interrupt
\param[in]
  irqNumber - number of external interrupt.
******************************************************************************/
INLINE void halDisablePCIrqInterrupt(uint8_t irqNumber)
{
  // Disable external interrupt request
  PCICR &= ~(1 << irqNumber);
}

#endif /* _HALIRQ_H */

//eof halirq.h
