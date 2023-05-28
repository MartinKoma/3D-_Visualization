/**************************************************************************//**
  \file  halWdtInit.c

  \brief Implementation of WDT start up procedure.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      29/05/07 E. Ivanov - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <resetReason.h>
#include <halAppClock.h>

/******************************************************************************
                   Defines section
******************************************************************************/
#define PIN_OUT 62500
#define MEANING_BITS   0x1F
#define DELAY_VALUE 50000

/******************************************************************************
                        Prototypes section
******************************************************************************/
#ifdef _SYS_ASSERT_ON_
  INLINE void halJumpNullHandler(void);
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
#if defined(__GNUC__)
  uint8_t halResetReason __attribute__ ((section (".noinit")));
#elif defined(__ICCAVR__)
  __no_init uint8_t halResetReason;
#endif

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Resets and stops wdt. Saves the reason of reset.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
#if defined(__GNUC__)

  void halWdtInit(void) __attribute__ ((naked)) \
    __attribute__ ((section (".init0")));

  void halWdtInit(void)
  {
    ASM("clr r1");
#elif defined(__ICCAVR__)

/* Since standard c doesn’t have this feature, so the coverity tool picked up this as error
   In IAR __task will relaxes the rules for preserving the register. The function which described 
   with __task do not save all registers on entry and exit(similar to RTOS), it may possible to corrupt 
   the calling function register. So the care must be taken to avoid this corruption. “__task” not an return type. 
   Its IAR compiler keyword. */
#if defined(__COVERITY__)
void halWdtInit(void)
#elif defined(__ICCAVR__)
  __task void halWdtInit(void)
#endif
  {
    ASM("clr r15");
#else
  #error 'Compiler not supported.'
#endif

    if (TEMP_WARM_RESET != halResetReason)
    {
      halResetReason = MCUSR & MEANING_BITS;

      if (halResetReason & POWER_ON_RESET)
        halResetReason = POWER_ON_RESET;
    }
    else
    {
      halResetReason = WARM_RESET;
    }
    MCUSR = 0;
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = 0x00;

  #ifdef _SYS_ASSERT_ON_
    halJumpNullHandler();
  #endif
  }

#ifdef _SYS_ASSERT_ON_
/******************************************************************************
Jump to NULL handler.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halJumpNullHandler(void)
{
  if (0 == halResetReason) // was jump on NULL
  {
    register volatile uint16_t tmp = SP;
    uint32_t delay;

    ASM ("cli");
    DDRE |= (1 << 2) | (1 << 3) | (1 << 4);
    PORTE &= ~((1 << 2) | (1 << 3) | (1 << 4));

    /* Init UART*/
    UBRR1H = 0;
    #if (F_CPU == 4000000ul)
      UBRR1L = 12;
    #elif (F_CPU == 8000000ul)
      UBRR1L = 25;
    #elif (F_CPU == 16000000ul)
      UBRR1L = 51;
    #endif
    UCSR1A = (1 << U2X1);
    UCSR1B = (1 << TXEN1);
    UCSR1C = (3 << UCSZ10);  // 8-bit data

    /* Init timer counter 4.*/
    OCR4A = 0;
    /* Disable TC4 interrupt */
    TIMSK4 &= ~(1 << OCIE4A);
    /* main clk / 8 */
    TCCR4B = (1 << WGM12) | (1 << CS11);

    while (1)
    {
      do
      { /* Send byte to UART */
        while (!(UCSR1A & (1 << UDRE1)));
        UDR1 = *((uint8_t *)SP);
        SP++;
      } while (RAMEND >= SP);
      SP = tmp;


      PORTE ^= (1 << 2);
      for (delay = 0; delay < DELAY_VALUE; delay++)
        asm("nop");

      PORTE ^= ((1 << 2) | (1 << 3));
      for (delay = 0; delay < DELAY_VALUE; delay++)
        asm("nop");

      PORTE ^= ((1 << 3) | (1 << 4));
      for (delay = 0; delay < DELAY_VALUE; delay++)
        asm("nop");

      PORTE ^= (1 << 4);
    }
  }
}
#endif

#if defined(__GNUC__) && defined(_STATS_ENABLED_)
void halFillStack(void) __attribute__ ((naked, section (".init1")));
/**************************************************************************//**
\brief Fill cstack with repeated pattern 0xCD
******************************************************************************/
void halFillStack(void)
{
  extern uint16_t __stack_start;
  extern uint16_t __stack;

  for (uint8_t *start = (uint8_t *)&__stack_start; start <= (uint8_t *)&__stack; start++)
    *start = 0xCD;
}
#endif
// eof halWdtInit.c
