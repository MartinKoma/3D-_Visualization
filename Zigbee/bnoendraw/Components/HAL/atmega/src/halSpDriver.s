/**************************************************************************//**
  \file  halSpDriver.s

  \brief Implementation of self programming functions.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      13/05/09 A. Khromykh - Created
      29/06/12 D. Kolmakov - Copied and modified
    Last change:
      $Id: halSpDriver.s 27584 2015-01-09 14:45:42Z unithra.c $
*******************************************************************************/
/*******************************************************************************
This routine enables RWW section.
Parameters:
  none
Returns:
  none.
*******************************************************************************/
.section .boot_section
.global halSpRWWSectionEnable
halSpRWWSectionEnable:
  in     r19, 0x3B                            ; Save RAMPZ, which is restored in SP_CommonSPM.
  ldi    r20, 0x10                            ; Prepare NVM command in R20.  (1 << RWWSRE)
  jmp    SP_CommonSPM                         ; Jump to common SPM code.

/*******************************************************************************
This routine writes the word from R23:R22 into the Flash page buffer at
address R25:R24.
Parameters:
  R25:R24 - Byte address into Flash page.
  R23:R22 - Word to write.
Returns:
  none.
*******************************************************************************/
.section .boot_section
.global halSpLoadFlashWord
halSpLoadFlashWord:
  in     r19, 0x3B                            ; Save RAMPZ, which is restored in SP_CommonSPM.
  movw   r0,  r22                             ; Prepare flash word in R1:R0.
  clr    r20                                  ; prepare command for SPM
  jmp    SP_CommonSPM                         ; Jump to common SPM code.

/*******************************************************************************
This routine erases the page buffer to the
Flash page at address R25:R24:R23:R22 in the application section. The address
can point anywhere inside the page.
Parameters:
  R25:R24:R23:R22 - Byte address into Flash page.
Returns:
  none.
*******************************************************************************/
.section .boot_section
.global halSpEraseApplicationPage
halSpEraseApplicationPage:
  in     r19, 0x3B                             ; Save RAMPZ, which is restored in SP_CommonSPM.
  out    0x3B, r24                             ; Load RAMPZ with the MSB of the address.
  movw   r24, r22                              ; Move low bytes of address to ZH:ZL from R23:R22
  ldi    r20, 0x02                             ; Prepare SPM command in R20. (1 << PGERS)
  jmp    SP_CommonSPM                          ; Jump to common SPM code.

/*******************************************************************************
This routine writes the page buffer to the
Flash page at address R25:R24:R23:R22 in the application section. The address
can point anywhere inside the page.
Parameters:
  R25:R24:R23:R22 - Byte address into Flash page.
Returns:
  none.
*******************************************************************************/
.section .boot_section
.global halSpWriteApplicationPage
halSpWriteApplicationPage:
  in     r19, 0x3B                             ; Save RAMPZ, which is restored in SP_CommonSPM.
  out    0x3B, r24                             ; Load RAMPZ with the MSB of the address.
  movw   r24, r22                              ; Move low bytes of address to ZH:ZL from R23:R22
  ldi    r20, 0x04                             ; Prepare NVM command in R20.  (1 << PGWRT)
  jmp    SP_CommonSPM                          ; Jump to common SPM code.

/*******************************************************************************
This routine is called by several other routines, and contains common code
for executing an SPM command, including the return statement itself.

If the operation (SPM command) requires the R1:R0 registers to be
prepared, this must be done before jumping to this routine.
Parameters:
  R1:R0    - Optional input to SPM command.
  R25:R24  - Low bytes of Z pointer.
  R20      - SPM Command code.
Returns:
  none.
*******************************************************************************/
.section .boot_section
SP_CommonSPM:
  movw   r30, r24                              ; Load R25:R24 into Z.
  ori    r20, 0x01                             ; prepare command for SPM
  sts    0x57, r20                             ; Load prepared command into SPMCSR register.
  spm                                         ; Self-program.
  clr    r1                                    ; Clear R1 for GCC _zero_reg_ to function properly.
  out    0x3B, r19                             ; Restore RAMPZ register.
  ret

/*******************************************************************************
Reads byte from internal flash
Parameters:
  R25:R24:R23:R22 - Byte address into flash.
Returns:
  R24 - read byte from flash.
*******************************************************************************/
.section .boot_section
.global halSpReadInternalFlash
halSpReadInternalFlash:
    in     r25,  0x3B                             ; Save RAMPZ.
    out    0x3B, r24                              ; Load RAMPZ with the MSB of the address.
    movw   r30,  r22                              ; Move low bytes of address to ZH:ZL from R23:R22
    elpm   r24,  Z                                ; Extended load program memory from Z address
    out    0x3B, r25                              ; Restore RAMPZ register.
    ret                                          ; return from function

; eof halSpDriver.s

