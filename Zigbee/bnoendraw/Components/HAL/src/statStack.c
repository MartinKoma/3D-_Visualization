/**************************************************************************//**
\file  stackStat.c

\brief Collection of stack statistics runtime data for report

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    21/02/2015 Mukesh Kumar Basrani - Created
  Last Change:
    $Id: statStack.c 27648 2015-02-24 05:42:55Z mukesh.basrani $
*******************************************************************************/
#if defined(_STATS_ENABLED_)

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <statStack.h>

/******************************************************************************
                   Define section
******************************************************************************/
/******************************************************************************
				   Extern section
******************************************************************************/

#ifdef __GNUC__
extern uint16_t __stack_start;
extern uint16_t __stack;
#endif

/******************************************************************************
                    Static functions prototypes section
******************************************************************************/

uint16_t probeStack(const volatile uint8_t *start, const volatile uint8_t *end);

// Preincluded generator for reporting stack usage. Only classic AVRs are supported for now

/**************************************************************************//**
\brief Measure stack usage by searching for first overwritten byte of
       default fill pattern

\param[in] start - start of section
\param[in] end - end of section (address of byte after the last one)
\return number of used bytes
******************************************************************************/
uint16_t probeStack(const volatile uint8_t *start, const volatile uint8_t *end)
{
  while (start < end && *start == 0xCD)
    start++;
  return end - start;
}


/**************************************************************************//**
\brief Measures stack and returns

\param[out] None
\param[in] None
\return number of actually placed bytes
******************************************************************************/
uint16_t Stat_GetCurrentStackLeft(void)
{
#if defined(__ICCAVR__)  //Only for ATMega
  #pragma section = "CSTACK"
  volatile uint8_t* pCStackEnd = __segment_begin("CSTACK");
#elif defined(__GNUC__) 
  volatile uint8_t* pCStackEnd = (uint8_t *)&__stack_start;
#else
  return STACK_CHECK_DISABLED;
#endif	

  uint8_t pLowCStack = *((uint8_t*) UTIL_CSTACK_PTR_LOADDR);
  uint8_t pHiCStack = *((uint8_t*) UTIL_CSTACK_PTR_HIADDR);    
  uint8_t* pCStack = (uint8_t*)( (uint16_t)pLowCStack | (((uint16_t)pHiCStack) << 8u) );

  return ((uint16_t) (pCStack-pCStackEnd));
}

#endif // _STATS_ENABLED_
// eof stackStat.c
