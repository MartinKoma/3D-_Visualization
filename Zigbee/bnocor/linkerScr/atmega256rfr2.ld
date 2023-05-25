OUTPUT_FORMAT("elf32-avr")
OUTPUT_ARCH(avr:6)

MEMORY
{
  text   (rx)   : ORIGIN = 0x00000000, LENGTH = 256K
  boot   (rx)   : ORIGIN = 0x0003F000, LENGTH = 4K
  access (rx)   : ORIGIN = 0x0003FFF0, LENGTH = 16
  data   (rw!x) : ORIGIN = 0x00800200, LENGTH = 32K - 500  /* leave 500 bytes for stack */
  eeprom (rw!x) : ORIGIN = 0x00810000, LENGTH = 8K
}

SECTIONS
{
  .text :
  {
    PROVIDE(__text_start = .);

    *(.vectors)
    KEEP(*(.vectors))
    . = ALIGN(0x400);

    /* PDS NV memory section */
    PROVIDE(__d_nv_mem_start = .);
    . = ALIGN(0x4400);
    PROVIDE(__d_nv_mem_end = .);
    
    /* Non-volatile file system PDS_FF section */
    PROVIDE(__pds_ff_start = .);
    KEEP(*(.pds_ff))
    PROVIDE(__pds_ff_end = .);

    /* Non-volatile file system PDS_FD section */
    PROVIDE(__pds_fd_start = .);
    KEEP(*(.pds_fd))
    PROVIDE(__pds_fd_end = .);

    *(.progmem.gcc*)
    *(.progmem*)
    . = ALIGN(2);

    *(.trampolines*)
    *(.jumptables*)
    *(.lowtext*)

    *(.init0)
    KEEP (*(.init0))
    *(.init1)
    KEEP (*(.init1))
    *(.init2)
    KEEP (*(.init2))
    *(.init3)
    KEEP (*(.init3))
    *(.init4)
    KEEP (*(.init4))
    *(.init5)
    KEEP (*(.init5))
    *(.init6)
    KEEP (*(.init6))
    *(.init7)
    KEEP (*(.init7))
    *(.init8)
    KEEP (*(.init8))

    *(.text.main)
    KEEP (*(.text*main))
    *(.text)
    *(.text.*)

    PROVIDE(__text_end = .);
  } > text

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    PROVIDE(__data_start = .);
    *(.data*)
    *(.rodata*)
    *(.gnu.linkonce.d*)
    . = ALIGN(2);
    PROVIDE(__data_end = .);
  } > data

  .bss __data_end :
  {
    PROVIDE(__bss_start = .);
    *(.bss*)
    *(COMMON)
    PROVIDE(__bss_end = .);
  } > data

  .noinit __bss_end :
  {
    *(.noinit*)
    PROVIDE(__heap_start = .);
  } > data

  __stack_start = .;

  __data_load_start = LOADADDR(.data);
  __data_load_end = __data_load_start + SIZEOF(.data);

  .access_section :
  {
    KEEP(*(.access_section*))
    *(.access_section*)
  } > access
  
  .boot_section :
  {
    *(.boot_section*)
  } > boot
  
  .eeprom :
  {
    FILL(0xff)
    BYTE(0xff)
    . = . + LENGTH(eeprom)-1;
  } > eeprom
  
  /DISCARD/ :
  {
    *(.init9)
    *(.fini9)
  }

}
