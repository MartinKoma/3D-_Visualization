/**************************************************************************//**
  \file  halAdc.c

  \brief Implementation of hardware depended ADC interface.

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
#if defined(HAL_USE_ADC)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <adc.h>
#include <halW1.h>
#include <halDbg.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Defines section
******************************************************************************/
#define ALL_CHANNEL_MASK          0x1F
#define CHANNEL_MASK_1            0x01
#define CHANNEL_MASK_2            0x03
#define CHANNEL_MASK_3            0x04
#define CHANNEL_MASK_4            0x0C
#define DELAY_FOR_STABILIZE       125

/******************************************************************************
                   Constants section
******************************************************************************/
#if F_CPU == 4000000
  PROGMEM_DECLARE(uint8_t halAdcDivider[5]) = {2, 3, 4, 5, 6};
#elif F_CPU == 8000000
  PROGMEM_DECLARE(uint8_t halAdcDivider[5]) = {3, 4, 5, 6, 7};
#elif F_CPU == 16000000
  PROGMEM_DECLARE(uint8_t halAdcDivider[5]) = {4, 5, 6, 7, 8};
#endif

/******************************************************************************
                   External variables section
******************************************************************************/
extern HAL_AdcDescriptor_t *adcDesc;

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Set up parametres to ADC.
******************************************************************************/
void  halOpenAdc(void)
{
  /* sets voltage reference */
  ADMUX = adcDesc->voltageReference;
  /* Enable left adjust result */
  if (RESOLUTION_8_BIT == adcDesc->resolution)
    ADMUX |= (1 << ADLAR);

  uint8_t tmp;
  memcpy_P(&tmp, &(halAdcDivider[adcDesc->sampleRate]), 1);
  ADCSRA = tmp | (1 << ADEN);
}

/**************************************************************************//**
\brief starts convertion on the ADC channel.
\param[in]
  channel - channel number.
******************************************************************************/
void halStartAdc(HAL_AdcChannelNumber_t channel)
{
  adcDesc->service.halAdcCurCount = 0;
  /* disable digital buffers */
  if (HAL_ADC_CHANNEL3 >= channel)
  {
    DIDR0 = (1 << channel);
  }
  else
  {
    if ((HAL_ADC_DIFF_CHANNEL0 == channel) || (HAL_ADC_DIFF_CHANNEL2 == channel))
      DIDR0 = CHANNEL_MASK_1;
    else if ((HAL_ADC_DIFF_CHANNEL1 == channel) || (HAL_ADC_DIFF_CHANNEL3 == channel))
      DIDR0 = CHANNEL_MASK_2;
    else if ((HAL_ADC_DIFF_CHANNEL4 == channel) || (HAL_ADC_DIFF_CHANNEL6 == channel))
      DIDR0 = CHANNEL_MASK_3;
    else if ((HAL_ADC_DIFF_CHANNEL5 == channel) || (HAL_ADC_DIFF_CHANNEL7 == channel))
      DIDR0 = CHANNEL_MASK_4;
  }

  uint8_t tmp = ADMUX & ALL_CHANNEL_MASK;

  /* clear previous channel number */
  ADMUX &= ~ALL_CHANNEL_MASK;
  /* set current channel number */
  ADMUX |= channel;

  /* if new differential channel is settled then must make 125 us delay for gain stabilize. */
  if ((tmp != channel) && (HAL_ADC_CHANNEL3 < channel))
    __delay_us(DELAY_FOR_STABILIZE);

  if (adcDesc->selectionsAmount > 1)
    ADCSRA |= ((1 << ADIE)  | (1 << ADATE) | (1 << ADSC));  // Starts running mode
  else
    ADCSRA |= ((1 << ADIE) | (1 << ADSC)); // Starts one conversion
}

/**************************************************************************//**
\brief Closes the ADC.
******************************************************************************/
void halCloseAdc(void)
{
  ADMUX  = 0;
  ADCSRA = 0;
  // Digital input enable
  DIDR0 = 0;
}

/**************************************************************************//**
\brief ADC conversion complete interrupt handler.
******************************************************************************/
ISR(ADC_vect)
{
  BEGIN_MEASURE
  // Read ADC conversion result
  if (RESOLUTION_8_BIT == adcDesc->resolution)
    ((uint8_t *)adcDesc->bufferPointer)[adcDesc->service.halAdcCurCount++] = ADCH;
  else
    ((uint16_t *)adcDesc->bufferPointer)[adcDesc->service.halAdcCurCount++] = ADC;

  if (adcDesc->service.halAdcCurCount == adcDesc->selectionsAmount)
  {
    // Disable ADC Interrupt
    ADCSRA &= ~(1 << ADIE);
    halPostTask(HAL_TASK_ADC);
  }
  END_MEASURE(HALISR_ADC_MEASURE_CODE)
}
#endif // defined(HAL_USE_ADC)

// eof halAdc.c
