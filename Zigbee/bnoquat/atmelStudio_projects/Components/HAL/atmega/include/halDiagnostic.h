/**************************************************************************//**
  \file  halDiagnostic.h

  \brief Implementation of diagnostics defines.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      20/05/09 D. Kasyanov - Created
 ******************************************************************************/

#ifndef _HALDIAGNOSTIC_H
#define _HALDIAGNOSTIC_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halFCPU.h>
#include <halDbg.h>
#include <halClkCtrl.h>
#include <sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined (MEASURE)

        //#define SEPARATE_MEASUREMENTS

        #define TCNT5_ACCESS_TIME 8
        #define DEFALUT_TIME_LIMIT 100
        #define TIMER3_COMPA_TIME_LIMIT 0
        #define PHYDISPATCH_RFINT_TIME_LIMIT 0

        #define MAX_COUNTER_VALUE (0xFFFF)

#if defined (SEPARATE_MEASUREMENTS)
        #define STORE_MEASUREMENT(code, time)     if (time > maxTimesArray[code]) {maxTimesArray[code] = time;}
#else
        #define STORE_MEASUREMENT(code, time)
#endif

        #define BEGIN_MEASURE  { \
                                  uint16_t timeLimit = DEFALUT_TIME_LIMIT; \
                                  uint16_t start; uint16_t offset; \
                                  start = TCNT5; \
                                  if (TIFR5 & (1 << TOV5)) TIFR5 |= (1 << TOV5); \
                                  start = TCNT5; \
                                  if (TIFR5 & (1 << TOV5)) {TIFR5 |= (1 << TOV5); start = TCNT5;}

        #define END_MEASURE(code) offset = TCNT5; \
                                  offset = (TIFR5 & (1 << TOV5)) ? \
                                           (MAX_COUNTER_VALUE - start + offset) : \
                                           (offset - start); \
                                  offset = (offset - TCNT5_ACCESS_TIME) / (HAL_ReadFreq()/1000000ul); \
                                  STORE_MEASUREMENT(code, offset);  \
                                  if (timeLimit != 0) { \
                                    if (offset > timeLimit) { \
                                      TCCR5B = 0; SYS_E_ASSERT_FATAL(0, (0xE000 | code)); \
                                    } \
                                  } \
                               }
#else
    #define BEGIN_MEASURE
    #define END_MEASURE(code)
#endif


#if defined (MEASURE)
/******************************************************************************
                   Types section
******************************************************************************/
enum _CriticalSectionMeasureCode_t {
  HALISR_ADC_MEASURE_CODE                    = 0x00,
  HALISR_TWI_MEASURE_CODE                    = 0x01,
  HALATOM_APP_TIMER_SYNCHRONIZE_MEASURE_CODE = 0x02,
  HALISR_TIMER4_COMPA_MEASURE_CODE           = 0x03,
  HALATOM_SETLOWFUSES_MEASURE_CODE           = 0x04,
  HALATOM_INITFREQ_MEASURE_CODE = 0x05,
  HALISR_EEPROM_WRITE_MEASURE_CODE = 0x06,
  HALISR_EEPROM_READY_MEASURE_CODE = 0x07,
  HALISR_INT0_VECT_MEASURE_CODE = 0x08,
  HALISR_INT1_VECT_MEASURE_CODE = 0x09,
  HALISR_INT2_VECT_MEASURE_CODE = 0x0A,
  HALISR_INT3_VECT_MEASURE_CODE = 0x0B,
  HALISR_INT4_VECT_MEASURE_CODE = 0x0C,
  HALISR_INT5_VECT_MEASURE_CODE = 0x0D,
  HALISR_INT6_VECT_MEASURE_CODE = 0x0E,
  HALISR_INT7_VECT_MEASURE_CODE = 0x0F,
  HALATOM_CLEAR_TIME_CONTROL_MEASURE_CODE = 0x10,
  HALATOM_GET_SLEEP_MEASURE_CODE = 0x11,
  HALATOM_SLEEP_TIMER_SYNCHRONIZE_MEASURE_CODE = 0x12,
  HALISR_TIMER2_COMPA_MEASURE_CODE = 0x13,
  HALISR_TIMER2_OVF_MEASURE_CODE = 0x14,
  HALATOM_EEPROM_WRITE_MEASURE_CODE = 0x15,
  HALISR_USART0_UDR_MEASURE_CODE = 0x16,
  HALISR_USART0_TX_MEASURE_CODE = 0x17,
  HALISR_USART0_RX_MEASURE_CODE = 0x18,
  HALISR_USART1_UDRE_MEASURE_CODE = 0x19,
  HALISR_USART1_TX_MEASURE_CODE = 0x1A,
  HALISR_USART1_RX_MEASURE_CODE = 0x1B,
  HALISR_INT4_MEASURE_CODE = 0x1C,
  HALATOM_STARTWDT_MEASURE_CODE = 0x1D,
  HALATOM_WRITEBYTE_RFSPI_MEASURE_CODE = 0x1E,
  HALISR_TIMER3_COMPA_MEASURE_CODE = 0x1F,
  HALISR_PHYDISPATCH_RFINT_MEASURE_CODE = 0x20,
  HALATOM_APP_TIMER_SYNC_MEASURE_CODE = 0x21,
  HAL_PWM1_MEASURE_CODE = 0x22,
  HAL_PWM2_MEASURE_CODE = 0x23,
  HALATOM_WDT_START_MEASURE_CODE = 0x24,
  HALMACISR_TRX24_RX_END_MEASURE_CODE = 0x25,
  HALMACISR_TRX24_TX_END_MEASURE_CODE = 0x26,
  HALMACISR_BAT_LOW_MEASURE_CODE = 0x27,
  HALISR_SPI_MEASURE_CODE = 0x28,
  HAL_GET_SLEEP_MEASURE_CODE = 0x29,
  HALISR_TIMER4_COMPB_MEASURE_CODE = 0x2A,
  // = 0x2B,
  HAL_SPI_RX1_MEASURE_CODE = 0x2C,
  HAL_SPI_RX2_MEASURE_CODE = 0x2D,
  HAL_SPI_RX3_MEASURE_CODE = 0x2E,
  HALATOM_USART_CHECK_TASK_MEASURE_CODE = 0x2F,
  HALATOM_USART_CLEAR_TASK_MEASURE_CODE = 0x30,
  HALATOM_WRITE_USART_0_MEASURE_CODE = 0x31,
  HALATOM_WRITE_USART_1_MEASURE_CODE = 0x32,
  HALATOM_WRITE_USART_2_MEASURE_CODE = 0x33,
  HALATOM_READ_USART_0_MEASURE_CODE = 0x34,
  HALATOM_READ_USART_1_MEASURE_CODE = 0x35,
  HALATOM_USART_RX_COMPLETE_MEASURE_CODE = 0x36,
  MACHWD_POST_TASK_MEASURE_CODE = 0x37,
  MACHWD_RESET_MEASURE_CODE = 0x38,
  MACHWD_SETHANDLER_ID_1_MEASURE_CODE = 0x39,
  MACHWD_SETHANDLER_ID_2_MEASURE_CODE = 0x3A,
  MACHWD_SETHANDLER_ID_3_MEASURE_CODE = 0x3B,
  PHY_ED_REQ_MEASURE_CODE = 0x3C,
  MACHWD_RX_RELEASE_BUFFER_MEASURE_CODE = 0x3D,
  MEASURE_SECTIONS_NUMBER
};

#if defined (SEPARATE_MEASUREMENTS)
/******************************************************************************
                   External variables section
******************************************************************************/
extern uint16_t maxTimesArray[MEASURE_SECTIONS_NUMBER];
#endif // SEPARATE_MEASUREMENTS

#endif // MEASURE

#endif /* _HALDIAGNOSTIC_H */

