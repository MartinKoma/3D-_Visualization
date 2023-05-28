/**************************************************************************//**
  \file  pwm.c

  \brief Implementation of PWM interface.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
     10/11/08 A. Taradov - Created
     5/04/11 A.Razinkov - Refactored
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_PWM)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halPwm.h>
#include <pwm.h>

/******************************************************************************
                   Defines section
******************************************************************************/

/******************************************************************************
                   Constants section
******************************************************************************/

/******************************************************************************
                   Global variables section
******************************************************************************/

/******************************************************************************
                   Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Initializes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.

\return operation status
******************************************************************************/
int HAL_OpenPwm(HAL_PwmUnit_t pwmUnit)
{
  /* Check PWM unit */
  if ((PWM_UNIT_1 == pwmUnit) || (PWM_UNIT_3 == pwmUnit))
    halOpenPwm(pwmUnit);
  else
    return PWM_INVALID_UNIT_STATUS;

  return PWM_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Starts PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.

\return operation status
******************************************************************************/
int HAL_StartPwm(HAL_PwmDescriptor_t *descriptor)
{
  /* Invalid PWM channel specified */
  if (PWM_INVALID_CHANNEL <= descriptor->channel)
    return PWM_INVALID_CHANNEL_STATUS;
  /* Check PWM unit */
  if ((PWM_UNIT_1 == descriptor->unit) || (PWM_UNIT_3 == descriptor->unit))
  {
    halPreparePwmChannelAccess(descriptor);
    halStartPwm(descriptor);
  }
  else
    return PWM_INVALID_UNIT_STATUS;

  return PWM_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Stops PWM on specified channel.

\param [in] descriptor - PWM channel descriptor.

\return operation status
******************************************************************************/
int HAL_StopPwm(HAL_PwmDescriptor_t *descriptor)
{
  /* Invalid PWM channel specified */
  if (PWM_INVALID_CHANNEL <= descriptor->channel)
    return PWM_INVALID_CHANNEL_STATUS;
  else
    halStopPwm(descriptor);

  return PWM_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Sets base frequency of module. Common for all module channels.

\param [in] pwmUnit - PWM unit number. Equal to corresponding Timer/Counter ID.
\param [in] top - value for the TOP register.
\param [in] prescaler - clock prescaler.

\return operation status
******************************************************************************/
int HAL_SetPwmFrequency(HAL_PwmUnit_t pwmUnit, uint16_t top, HAL_PwmPrescaler_t prescaler)
{
  /* Check prescaler value */
  if (PWM_PRESCALER_INVALID <= prescaler)
    return PWM_INVALID_PRESCALER_STATUS;
  else halSetPwmFrequency(pwmUnit, top, prescaler);

  return PWM_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Sets compare value for the PWM channel.

\param [in] descriptor - PWM channel descriptor.

\return operation status
******************************************************************************/
int HAL_SetPwmCompareValue(HAL_PwmDescriptor_t *descriptor, uint16_t cmpValue)
{
  /* Invalid PWM channel specified */
  if (PWM_INVALID_CHANNEL <= descriptor->channel)
    return PWM_INVALID_CHANNEL_STATUS;
  /* Check PWM unit */
  if ((PWM_UNIT_1 == descriptor->unit) || (PWM_UNIT_3 == descriptor->unit))
    halSetPwmCompareValue(descriptor, cmpValue);
  else
    return PWM_INVALID_UNIT_STATUS;

  return PWM_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Closes the PWM.

\param [in] pwmUnit - PWM unit number.
                      Equal to ID of Timer/Counter witch serves PWM module.

\return operation status
******************************************************************************/
int HAL_ClosePwm(HAL_PwmUnit_t pwmUnit)
{
  /* Check PWM unit */
  if ((PWM_UNIT_1 == pwmUnit) || (PWM_UNIT_3 == pwmUnit))
    halClosePwm(pwmUnit);
  else
    return PWM_INVALID_UNIT_STATUS;

  return PWM_SUCCESS_STATUS;
}
#endif // defined(HAL_USE_PWM)

// eof pwm.c
