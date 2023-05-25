/***************************************************************************//**
  \file  halDbg.h

  \brief Declarations of hal , bsb mistake interface.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      09/11/07 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALDBG_H
#define _HALDBG_H

#include <dbg.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
enum
{
  APPTIMER_MISTAKE                         = 0x2000,
  INCORRECT_EEPROM_ADDRESS                 = 0x2001,
  MEMORY_CANNOT_WRITE                      = 0x2002,
  USARTC_HALUSARTRXBUFFERFILLER_0          = 0x2003,
  USARTC_HALSIGUSARTTRANSMISSIONCOMPLETE_0 = 0x2004,
  USARTC_HALSIGUSARTRECEPTIONCOMPLETE_0    = 0x2005,
  HALUSARTH_HALCLOSEUSART_0                = 0X2006,
  HALUSARTH_HALENABLEUSARTDREMINTERRUPT_0  = 0X2007,
  HALUSARTH_HALDISABLEUSARTDREMINTERRUPT_0 = 0X2008,
  HALUSARTH_HALENABLEUSARTTXCINTERRUPT_0   = 0X2009,
  HALUSARTH_HALDISABLEUSARTTXCINTERRUPT_0  = 0X200A,
  HALUSARTH_HALENABLEUSARTRXCINTERRUPT_0   = 0X200B,
  HALUSARTH_HALDISABLEUSARTRXCINTERRUPT_0  = 0X200C,
  HALUSARTH_HALSENDUSARTBYTE_0             = 0X200D,
  USARTC_HALUSARTSAVEERRORREASON           = 0x200E,
  USARTC_HALSIGUSARTERROROCCURED_0         = 0x200F,
  USARTC_HALUNKNOWNERRORREASON_0           = 0x2010,
  ADC_NULLCALLBACK_0                       = 0x2011,
  SECURITY_MODULE_INVALID_COMMAND          = 0x2012,
  APPTIMER_HANDLER_0                       = 0x2013,
  I2C_NULLCALLBACK_0                       = 0x2014,
  IRQ_NULLCALLBACK_0                       = 0x2015,
  PC_IRQ_NULLCALLBACK_0                    = 0x2016,
  SLEEPTIMER_NULLCALLBACK_0                = 0x2017,
  SPI_NULLCALLBACK_0                       = 0x2018,
  SECURITY_NULLCALLBACK0                   = 0x2019,
  FLASH_HALFILLFLASHPAGEBUFFER_0           = 0x2100,
  TIMER_REMOVETIMER_0                      = 0x2101,

  HAL_USART_TX_EMPTY_LIMIT                 = 0x21CA,
  HAL_USART_TRANS_COMPLETE_LIMIT           = 0x21CB,
  HAL_USART_HW_CONTROLLER_LIMIT            = 0x21CC,
  HALISR_INT0_VECT_TIME_LIMIT              = 0x21CD,
  HALISR_INT1_VECT_TIME_LIMIT              = 0x21CE,
  HALISR_INT2_VECT_TIME_LIMIT              = 0x21CF,
  HALISR_INT3_VECT_TIME_LIMIT              = 0x21D0,
  HALISR_INT4_VECT_TIME_LIMIT              = 0x21D1,
  HAL_SLEEP_TIMER_SYNCHRONIZE_LIMIT        = 0x21D2,
  HAL_GET_SLEEP_TIME_LIMIT                 = 0x21D3,
  HAL_WDT_START_TIME_LIMIT                 = 0x21D4,
  HAL_EEPROM_WRITE_TIME_LIMIT              = 0x21D5,
  HAL_APP_TIMER_SYNC_TIME_LIMIT            = 0x21D6,
  HAL_PWM1_TIME_LIMIT                      = 0x21D7,
  HAL_PWM2_TIME_LIMIT                      = 0x21D8,
  HAL_SPI_RX1_TIME_LIMIT                   = 0x21D9,
  HAL_SPI_RX2_TIME_LIMIT                   = 0x21DA,
  HAL_SPI_RX3_TIME_LIMIT                   = 0x21DB,
  HAL_USART_CLEAR_TASK_TIME_LIMIT          = 0x21DC,
  HAL_USART_CHECK_TASK_TIME_LIMIT          = 0x21DD,
  HALMACISR_TRX24_RX_END_TIME_LIMIT        = 0x21DE,
  HALMACISR_TRX24_TX_END_TIME_LIMIT        = 0x21DF,
  HALMACISR_BAT_LOW_TIME_LIMIT             = 0x21E0,
  HALISR_INT5_VECT_TIME_LIMIT              = 0x21E1,
  HALMACISR_RTIMER_ALREADY_STARTED         = 0x21E2,
  HALISR_ADC_TIME_LIMIT                    = 0x21E3,
  HALISR_TIMER4_COMPA_TIME_LIMIT           = 0x21E4,
  HALATOM_SETLOWFUSES_TIME_LIMIT           = 0x21E5,
  HALATOM_INITFREQ_TIME_LIMIT              = 0x21E6,
  HALISR_EEPROM_READY_TIME_LIMIT           = 0x21E7,
  HALISR_INT6_VECT_TIME_LIMIT              = 0x21E8,
  HALISR_INT7_VECT_TIME_LIMIT              = 0x21E9,
  HALISR_TIMER2_COMPA_TIME_LIMIT           = 0x21EA,
  HALISR_TIMER2_OVF_TIME_LIMIT             = 0x21EB,
  HALISR_USART0_UDR_TIME_LIMIT             = 0x21EC,
  HALISR_USART0_TX_TIME_LIMIT              = 0x21ED,
  HALISR_USART0_RX_TIME_LIMIT              = 0x21EE,
  HALISR_USART1_UDRE_TIME_LIMIT            = 0x21EF,
  HALISR_USART1_TX_TIME_LIMIT              = 0x21F0,
  HALISR_USART1_RX_TIME_LIMIT              = 0x21F1,
  HALISR_INT4_TIME_LIMIT                   = 0x21F2,
  HALISR_TWI_TIME_LIMIT                    = 0x21F3,
  HALATOM_STARTWDT_TIME_LIMIT              = 0x21F4,
  HALISR_WDT_TIME_LIMIT                    = 0x21F5,
  HALATOM_SPI_TIME_LIMIT                   = 0x21F6,
  HALISR_TIMER3_COMPA_TIME_LIMIT           = 0x21F7,
  HALISR_PHYDISPATCH_RFINT_TIME_LIMIT      = 0x21F8,
  HALATOM_GETTIME_OF_APPTIMER_1_TIME_LIMIT = 0x21F9,
  HALATOM_GETTIME_OF_APPTIMER_2_TIME_LIMIT = 0x21FA,
  HALATOM_GETTIME_OF_APPTIMER_3_TIME_LIMIT = 0x21FB,
  HALATOM_WRITE_USART_TIME_LIMIT           = 0x21FC,
  HALATOM_READ_USART_TIME_LIMIT            = 0x21FD,
  HALATOM_USART_RX_COMPLETE_TIME_LIMIT     = 0x21FE,
  HALATOM_CLEAR_TIME_CONTROL_TIME_LIMIT    = 0x21FF
};

/******************************************************************************
                   Prototypes section
******************************************************************************/

#endif /* _HALDBG_H */

// eof halDbg.h
