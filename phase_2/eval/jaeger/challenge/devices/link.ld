/* ---- Original Script: /opt/riscv32i/riscv32-unknown-elf/lib/ldscripts/elf32lriscv.x ---- */
/* Default linker script, for normal executables */
/* Copyright (C) 2014-2017 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */


OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY {
  rom (rwx): ORIGIN = 0x00000000, LENGTH = 0x100000,
  ram (rwx): ORIGIN = 0x00100000, LENGTH = 0x400000,
  int (rwx): ORIGIN = 0x01000000, LENGTH = 4
}

SECTIONS
{
  .text           :
  {
    *(.text._start)
    *(.text)
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } > rom
  
  .data           :
  {
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(.dynsbss)
    *(.data .data.* .gnu.linkonce.d.*)
    *(.got.plt) *(.igot.plt) *(.got) *(.igot)
    *(.data1)
  } > ram
}