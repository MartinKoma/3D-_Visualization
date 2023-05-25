/**************************************************************************//**
  \file  halAdc.h

  \brief Declaration of hardware depended ADC interface.

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

#ifndef _HALADC_H
#define _HALADC_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <halTaskManager.h>
#include <halFCPU.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define ADC_0  0x78

/******************************************************************************
                   Types section
******************************************************************************/
typedef uint8_t AdcHwChannel_t;

/**************************************************************************//**
\brief channel number. \n
Note for avr:
If 10x gain is used, 8 bit resolution can be expected. \n
If 200x gain is used, 7 bit resolution can be expected. \n
If the user wants to perform a quick polarity check of the result, \n
it is sufficient to read the MSB of the result. If the bit is one, \n
the result is negative, and if this bit is zero, the result is positive. \n
To reach the given accuracy, 10x or 200x Gain should not be used \n
for operating voltage below 2.7V.
******************************************************************************/
typedef enum
{
  /** \brief channel ADC0 */
  HAL_ADC_CHANNEL0 = 0,
  /** \brief channel ADC1 */
  HAL_ADC_CHANNEL1 = 1,
  /** \brief channel ADC2 */
  HAL_ADC_CHANNEL2 = 2,
  /** \brief channel ADC3 */
  HAL_ADC_CHANNEL3 = 3,
  /** \brief ADC0 - ADC0 with gain 10x */
  HAL_ADC_DIFF_CHANNEL0 = 8,
  /** \brief ADC1 - ADC0 with gain 10x */
  HAL_ADC_DIFF_CHANNEL1 = 9,
  /** \brief ADC0 - ADC0 with gain 200x */
  HAL_ADC_DIFF_CHANNEL2 = 10,
  /** \brief ADC1 - ADC0 with gain 200x */
  HAL_ADC_DIFF_CHANNEL3 = 11,
  /** \brief ADC2 - ADC2 with gain 10x */
  HAL_ADC_DIFF_CHANNEL4 = 12,
  /** \brief ADC3 - ADC2 with gain 10x */
  HAL_ADC_DIFF_CHANNEL5 = 13,
  /** \brief ADC2 - ADC2 with gain 200x */
  HAL_ADC_DIFF_CHANNEL6 = 14,
  /** \brief ADC3 - ADC2 with gain 200x */
  HAL_ADC_DIFF_CHANNEL7 = 15
} HAL_AdcChannelNumber_t;

/**************************************************************************//**
\brief adc sample rate
******************************************************************************/
typedef enum
{
  ADC_77KSPS,
  ADC_39KSPS,
  ADC_19200SPS,
  ADC_9600SPS,
  ADC_4800SPS
} HAL_AdcSampleRate_t;

/**************************************************************************//**
\brief adc resolution
******************************************************************************/
typedef enum
{
  RESOLUTION_8_BIT,
  RESOLUTION_10_BIT
} HAL_AdcResolution_t;

/**************************************************************************//**
\brief adc voltage reference
******************************************************************************/
typedef enum
{
  /** \brief AREF, Internal Vref turned off. \n
  If 10x or 200x gain is selected, only 2.56 V should be used as Internal Voltage Reference. \n
  For differential conversion, only 1.1V cannot be used as internal voltage reference. */
  AREF = (0 << 6),
  /** \brief AVCC with external capacitor at AREF pin */
  AVCC = (1 << 6),
  /** \brief Internal 1.1V Voltage Reference with external capacitor at AREF pin */
  INTERNAL_1d1V = (2 << 6),
  /** \brief Internal 2.56V Voltage Reference with external capacitor at AREF pin */
  INTERNAL_2d56V = (3 << 6)
} HAL_AdcVoltageReference_t;

typedef struct
{
  volatile uint16_t halAdcCurCount;
} HalAdcService_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Set up parametres to ADC.
******************************************************************************/
void halOpenAdc(void);

/**************************************************************************//**
\brief starts convertion on the ADC channel.
\param[in]
  channel - channel number.
******************************************************************************/
void halStartAdc(HAL_AdcChannelNumber_t channel);

/**************************************************************************//**
\brief Closes the ADC.
******************************************************************************/
void halCloseAdc(void);

#endif /* _HALADC_H */

// eof halAdc.h
