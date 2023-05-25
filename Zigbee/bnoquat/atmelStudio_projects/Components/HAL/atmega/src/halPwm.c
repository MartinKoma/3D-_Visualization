/**************************************************************************//**
  \file  halPwm.c

  \brief Implementation of hardware depended PWM interface.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
     10/11/08 A. Taradov - Created
      5/04/11 A.Razinkov - Refactored
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_PWM)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halPwm.h>
#include <atomic.h>

/******************************************************************************
                   Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Prepare PWM channel access. Determine control registers, ports, pins etc.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halPreparePwmChannelAccess(HAL_PwmDescriptor_t *descriptor)
{
  /* PWM output port and pin determination */
  switch (descriptor->unit)
  {
    case PWM_UNIT_1:
      descriptor->service.DDRn = &DDRB;
      descriptor->service.pwmBaseChannelPin = PWM_UNIT_1_BASE_CHANNEL_PIN;
      break;

    case PWM_UNIT_3:
      descriptor->service.DDRn = &DDRE;
      descriptor->service.pwmBaseChannelPin = PWM_UNIT_3_BASE_CHANNEL_PIN;
      break;

    /* Invalid PWM unit identifier */
    default:
      break;
  }
  /* PWM control registers determination */
  descriptor->service.COMnx0 = COMNX0_BASE_BIT - (descriptor->channel * COMNX_BITFIELD_SIZE);
  descriptor->service.OCRnx = (&OCRnA(descriptor->unit) + (descriptor->channel));
}

/**************************************************************************//**
\brief Initializes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.
******************************************************************************/
void halOpenPwm(HAL_PwmUnit_t pwmUnit)
{
  /* Clear Timer/Counter */
  halMoveWordToRegister(&TCNTn(pwmUnit), 0x0000);
  /* Clear Timer/Counter Input Capture register */
  halMoveWordToRegister(&ICRn(pwmUnit), 0x0000);
  /* Configure port for normal operation */
  TCCRnA(pwmUnit) = 0x00;
  /* Configure PWM mode: phase and frequency correct, TOP = ICRn */
  TCCRnB(pwmUnit) = (1 << WGMn3);
}

/**************************************************************************//**
\brief Starts PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halStartPwm(HAL_PwmDescriptor_t *descriptor)
{
  /* Force compare on channel to clear output */
  TCCRnC(descriptor->unit) |= (1 << (FOCNX_BASE_BIT - descriptor->channel));
  /* Configure PWM pin as output */
  halMakeOutPwmPin(descriptor);
  /* Configure Compare Output Mode for PWM channel */
  TCCRnA(descriptor->unit) |=
    ((1 << COMnx1(descriptor)) | (1 << COMnx0(descriptor)));
  if (PWM_POLARITY_INVERTED == descriptor->polarity)
    TCCRnA(descriptor->unit) &= ~(1 << COMnx0(descriptor));
}

/**************************************************************************//**
\brief Stops PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halStopPwm(HAL_PwmDescriptor_t *descriptor)
{
  /* Clean compare register and stop Timer/Counter */
  halMoveWordToRegister(&OCRnx(descriptor), 0x0000);
  TCCRnA(descriptor->unit) &= ~((1 << COMnx1(descriptor)) | (1 << COMnx0(descriptor)));
  /* Configure PWM pin as intput */
  halMakeInPwmPin(descriptor);
}

/**************************************************************************//**
\brief Sets base frequency of module. Common for all module channels.

\param [in] pwmUnit - PWM unit number. Equal to corresponding Timer/Counter ID.
\param [in] top - value for the TOP register.
\param [in] prescaler - clock prescaler.
******************************************************************************/
void halSetPwmFrequency(HAL_PwmUnit_t pwmUnit, uint16_t top, HAL_PwmPrescaler_t prescaler)
{
  /* Stop Timer/Counter */
  TCCRnB(pwmUnit) &= ~((1 << CSn2) | (1 << CSn1) | (1 << CSn0));
  /* Set new TOP register (ICRn in our case) */
  halMoveWordToRegister(&ICRn(pwmUnit), top);
  /* Initialize Timer/Counter with new TOP value */
  halMoveWordToRegister(&TCNTn(pwmUnit), top);
  /* Clear all PWM outputs */
  halMoveWordToRegister(&OCRnA(pwmUnit), 0x0000);
  halMoveWordToRegister(&OCRnB(pwmUnit), 0x0000);
  halMoveWordToRegister(&OCRnC(pwmUnit), 0x0000);

  /* Run Timer/Counter */
  TCCRnB(pwmUnit) |= prescaler;
}

/**************************************************************************//**
\brief Sets compare value for the PWM channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halSetPwmCompareValue(HAL_PwmDescriptor_t *descriptor, uint16_t cmpValue)
{
  halMoveWordToRegister(&OCRnx(descriptor), cmpValue);
}

/**************************************************************************//**
\brief Closes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.
******************************************************************************/
void halClosePwm(HAL_PwmUnit_t pwmUnit)
{
  /* Configure port for normal operation */
  TCCRnA(pwmUnit) = 0x00;
  /* Disable PWM and stop timer */
  TCCRnB(pwmUnit) = 0x00;
}
#endif // defined(HAL_USE_PWM)

// eof halPwm.c
