/**************************************************************************//**
  \file  halW1.s

  \brief Implementation of 1-wire hardware-dependent module.
  
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

.include "halGccD.h"
.set __w1_port, 0x14			; PORTG
.set __w1_bit, 5				; PORTG5
.set __w1_ddr, 0x13				; DDRG
.set __w1_pin, 0x12				; PING

/*=============================================================
 Resets all devices connected to the bus. Function asserts on
 the bus reset pulse and detects presence pulse. The result is
 contained in r24.
 Parameters:
 Returns:
   W1_SUCCESS_STATUS   - If device(s) was(were) detected.
   W1_NO_DEVICE_STATUS - If device(s) was(were) not detected.
===============================================================*/
.global halResetW1
.type halResetW1,@function
halResetW1:
  ; Store SREG
  push r23
  in r23, 0x3F
  ; Disable interrupts
  cli
  ; Pull down
  sbi __w1_ddr, __w1_bit
  cbi __w1_port, __w1_bit
  ; Reset Low Time (500 us)
  ldi r24, 250
  call __delay_us
  ldi r24, 250
  call __delay_us
  ; Tri-state (external pullup)
  cbi __w1_ddr, __w1_bit
  ; Presence-Detect Sample Time (70 us)
  ldi r24, 70
  call __delay_us
  ; Precense-Detect
  ldi r24, 0x01
  sbic __w1_pin, __w1_bit
  ldi r24, 0x00
  push r24
  ; Tail of Reset High Time
  ldi r24, 240
  call __delay_us
  pop r24
  ; Restore SREG
  out 0x3F, r23
  pop r23
  ret

/*=============================================================
 Reads bit from the bus
 Returns:
   bit read from the bus in r24
===============================================================*/
.global halReadW1Bit
.type halReadW1Bit,@function
halReadW1Bit:
  ; Store SREG
  push r23
  in r23, 0x3F
  ; Disable interrupts
  cli
  ; Pull down
  sbi __w1_ddr, __w1_bit
  cbi __w1_port, __w1_bit
  ; Read Low Time (6 us)
  ldi r24, 6
  call __delay_us
  ; Tri-state (external pullup)
  cbi __w1_ddr, __w1_bit
  ; Tail of Read Sample Time (10 us)
  ldi r24, 10
  call __delay_us
  ; Read Sample
  clc
  sbic __w1_pin, __w1_bit
  sec
  rol r24
  ; Tail of Timeslot Duration
  push r24
  ldi r24, 100
  call __delay_us
  pop r24
  ; Restore SREG
  out 0x3F, r23
  pop r23
  ret

/*=============================================================
 Reads byte from the bus
 Returns:
   byte read from the bus in r24
===============================================================*/
.global halReadW1
.type halReadW1,@function
halReadW1:
  push r25
  push r23
  ldi r25, 8
__read_bit_again:
  call halReadW1Bit
  ror r24
  ror r23
  dec r25
  tst r25
  brne __read_bit_again
  mov r24, r23
  pop r23
  pop r25
  ret

/*=============================================================
 Writes bit to the bus
 Parameters:
   value - bit that should be written to the bus.
===============================================================*/
.global halWriteW1bit
.type halWriteW1bit,@function
halWriteW1bit:
  ; Store SREG
  push r23
  in r23, 0x3F
  ; Disable interrupts
  cli
  ; Pull down
  cbi __w1_port, __w1_bit
  sbi __w1_ddr, __w1_bit
  ; Write-1 Low Time
  push r24
  ldi r24, 6
  call __delay_us
  pop r24
  ; Write bit
  ror r24
  brcc __w1_write_zero
  ; Write-One -> tri-state (external pullup)
  cbi __w1_ddr, __w1_bit
__w1_write_zero:
  ; Tail of Timeslot Duration
  push r24
  ldi r24, 100
  call __delay_us
  pop r24
  ; Tri-state (external pullup)
  cbi __w1_ddr, __w1_bit
  ; Restore SREG
  out 0x3F, r23
  pop r23
  ret

/*=============================================================
 Writes byte to the bus
 Parameters:
   value - byte that should be written to the bus.
===============================================================*/
.global halWriteW1
.type halWriteW1,@function
halWriteW1:
  push r25
  ldi r25, 8
__write_bit_again:
  call halWriteW1bit
  dec r25
  tst r25
  brne __write_bit_again
  pop r25
  ret

/*=============================================================
 Delay in microseconds.
 Parameters:
   us - delay time in microseconds
===============================================================*/
.global __delay_us
.type __delay_us,@function
__delay_us:
__w0:
.if FCPU==8000000
    nop
    nop
    nop
    nop
.endif
.if FCPU==16000000
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
.endif
  dec r24
  tst r24
  brne __w0
  ret
; eof halW1.s
