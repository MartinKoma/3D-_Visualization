/**************************************************************************//**
  \file  halPwm.h

  \brief Declaration of hardware depended PWM interface.

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

#ifndef _HALPWM_H
#define _HALPWM_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <gpio.h>
#include <pwm.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Definitions section
******************************************************************************/
/* Waveform Generation Mode bits position. PWM module independent. */
#define WGMn0 0
#define WGMn1 1
#define WGMn2 3
#define WGMn3 4

/* Force Output Compare bits position. PWM module independent. */
#define FOCnA 7
#define FOCnB 6
#define FOCnC 5

/* Clock Select bits position. PWM module independent. */
#define CSn0 0
#define CSn1 1
#define CSn2 2

/* Compare Output Mode bits position. PWM module independent. */
#define COMnA0 6
#define COMnA1 7
#define COMnB0 4
#define COMnB1 5
#define COMnC0 2
#define COMnC1 3

/* PWN unit base channel pins position. PWM module dependent. */
#define PWM_UNIT_1_BASE_CHANNEL_PIN PB5
#define PWM_UNIT_3_BASE_CHANNEL_PIN PE3

/* Force Output Compare base bit. PWM module independent. */
#define FOCNX_BASE_BIT FOCnA
/* Compare Output Mode base bit number. PWM module independent. */
#define COMNX0_BASE_BIT COMnA0
/* Compare Output Mode bitfield size. PWM module and channel independent. */
#define COMNX_BITFIELD_SIZE 2
/* Compare Output Mode low bit number. PWM module independent. */
#define COMnx0(descriptor) ((descriptor)->service.COMnx0)
/* Compare Output Mode high bit number. PWM module independent. */
#define COMnx1(descriptor) ((descriptor)->service.COMnx0 + 1)
/* Output Compare Register. PWM module and channel dependent. */
#define OCRnx(descriptor) (*((descriptor)->service.OCRnx))
/* Data Direction Rregister. PWM module dependent. */
#define DDRn(descriptor) (*((descriptor)->service.DDRn))

/******************************************************************************
                   Types section
******************************************************************************/

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initializes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.
******************************************************************************/
void halOpenPwm(HAL_PwmUnit_t pwmUnit);

/**************************************************************************//**
\brief Starts PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halStartPwm(HAL_PwmDescriptor_t *descriptor);

/**************************************************************************//**
\brief Stops PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halStopPwm(HAL_PwmDescriptor_t *descriptor);

/**************************************************************************//**
\brief Sets base frequency of module. Common for all module channels.

\param [in] pwmUnit - PWM unit number. Equal to corresponding Timer/Counter ID.
\param [in] top - value for the TOP register.
\param [in] prescaler - clock prescaler.
******************************************************************************/
void halSetPwmFrequency(HAL_PwmUnit_t pwmUnit, uint16_t top, HAL_PwmPrescaler_t prescaler);

/**************************************************************************//**
\brief Sets compare value for the PWM channel.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halSetPwmCompareValue(HAL_PwmDescriptor_t *descriptor, uint16_t cmpValue);

/**************************************************************************//**
\brief Closes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.
******************************************************************************/
void halClosePwm(HAL_PwmUnit_t pwmUnit);

/**************************************************************************//**
\brief Prepare PWM channel access. Determine control registers, ports, pins etc.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
void halPreparePwmChannelAccess(HAL_PwmDescriptor_t *descriptor);

/**************************************************************************//**
\brief Configure corresponding pin as PWM out.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
static inline void halMakeOutPwmPin(HAL_PwmDescriptor_t *descriptor)
{
  DDRn(descriptor) |=
    (1 << (descriptor->service.pwmBaseChannelPin + descriptor->channel));
}

/**************************************************************************//**
\brief Configure corresponding PWM output pin as in.

\param [in] descriptor - PWM channel descriptor.
******************************************************************************/
static inline void halMakeInPwmPin(HAL_PwmDescriptor_t *descriptor)
{
  DDRn(descriptor) &=
    ~(1 << (descriptor->service.pwmBaseChannelPin + descriptor->channel));
}

/**************************************************************************//**
\brief Perform two-step writing to 16-bit registers with special access rule:
       TCNTn, OCRnA/B/C, ICRn.

\param [in] reg - register address.
\param [in] word - word to move.
******************************************************************************/
static inline void halMoveWordToRegister(volatile uint16_t *reg, uint16_t word)
{
ATOMIC_SECTION_ENTER
BEGIN_MEASURE
  /* High byte writing */
  *((volatile uint8_t*)(reg) + 1) = (uint8_t)(word >> 8);
  /* Low byte writing */
  *(volatile uint8_t*)(reg) = (uint8_t)(word);
END_MEASURE(HAL_PWM1_MEASURE_CODE)
ATOMIC_SECTION_LEAVE
}

/**************************************************************************//**
\brief Perform two-step reading of 16-bit registers with special access rule:
       TCNTn, OCRnA/B/C, ICRn.

\param [in] reg - register address.

\return register value
******************************************************************************/
static inline uint16_t halReadWordFromRegister(volatile uint16_t *reg)
{
  uint16_t word;
ATOMIC_SECTION_ENTER
BEGIN_MEASURE
  /* Low byte reading */
  word = *(volatile uint8_t*)(reg);
  /* High byte reading */
  word |= ((uint16_t)(*((volatile uint8_t*)(reg) + 1)) << 8);
END_MEASURE(HAL_PWM2_MEASURE_CODE)
ATOMIC_SECTION_LEAVE
  return word;
}

#endif /* _HALPWM_H */

// eof halPwm.h
