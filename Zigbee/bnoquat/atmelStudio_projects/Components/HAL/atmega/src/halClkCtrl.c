/**************************************************************************//**
  \file  halClkCtrl.c

  \brief Implementation of clock control module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/05/07 E. Ivanov - Created
      16/04/09 A. Khromykh - Refactored
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>
#include <atomic.h>
#include <halDbg.h>
#include <halDiagnostic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// defines fuse mask for RC oscillator
#define HAL_RC_OSCILLATOR_CLOCK 0x02
// mask for CKSEL bits
#define HAL_CKSEL_MASK          0x0F

/******************************************************************************
                   Prototypes section
******************************************************************************/
void halStartingCalibrate(void);

/******************************************************************************
                   Global variables section
******************************************************************************/
static volatile ClkSource_t clkClockSource;

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Initialization system clock.
******************************************************************************/
void halInitFreq(void)
{
  uint8_t lowFuseByte;

  // wait for end of eeprom writing
  while (EECR & (1 << EEPE));
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    lowFuseByte = SF_GET_LOW_FUSES();
  END_MEASURE(HALATOM_SETLOWFUSES_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  if (HAL_RC_OSCILLATOR_CLOCK == (lowFuseByte & HAL_CKSEL_MASK))
    clkClockSource = INTERNAL_RC;
  else
    clkClockSource = OTHER_SOURCE;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    if (INTERNAL_RC == clkClockSource)
    {
      ASM (
        "push r21                    \n\t"

        "ldi  r21, 0x80              \n\t" /* CLKPR = 1 << CLKPCE   */
        "sts  0x0061, r21            \n\t"

#if (F_CPU == 4000000ul)
        "ldi  r21, 0x01              \n\t" /* divider = (1 << CLKPS0); // RC-Oscillator/4 */
#endif
#if (F_CPU == 8000000ul)
        "ldi  r21, 0x00              \n\t" /* divider = 0; // RC-Oscillator/2 */
#endif
        "sts  0x0061, r21            \n\t" /* CLKPR = divider       */

        "pop r21                     \n\t"
      );
    }
    else
    {
      ASM (
        "push r21                    \n\t"

        "ldi  r21, 0x80              \n\t" /* CLKPR = 1 << CLKPCE   */
        "sts  0x0061, r21            \n\t"

#if (F_CPU == 4000000ul)
        "ldi  r21, 0x02              \n\t" /* divider = (1 << CLKPS1); // Division factor 4 */
#endif
#if (F_CPU == 8000000ul)
        "ldi  r21, 0x01              \n\t" /* divider = (1 << CLKPS0); // Division factor 2 */
#endif
#if (F_CPU == 16000000ul)
        "ldi  r21, 0x00              \n\t" /* divider = 0; // Division factor 1 */
#endif
        "sts  0x0061, r21            \n\t" /* CLKPR = divider value */

        "pop r21                     \n\t"
      );
    }
  END_MEASURE(HALATOM_INITFREQ_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  if (INTERNAL_RC == clkClockSource)
    halStartingCalibrate();
}

/**************************************************************************//**
\brief Return clock source

\return
  clock source.
******************************************************************************/
ClkSource_t halGetClockSource(void)
{
  return clkClockSource;
}

/**************************************************************************//**
\brief System clock.

\return
  system clock in Hz.
******************************************************************************/
uint32_t HAL_ReadFreq(void)
{
  return (uint32_t)F_CPU;
}

// eof halClkCtrl.c
