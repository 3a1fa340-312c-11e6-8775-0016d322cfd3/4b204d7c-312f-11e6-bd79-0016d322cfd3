 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 



 









































































STARTUP(vectors.o)
ENTRY(reset_vector)

INPUT(extras.o)

GROUP(libtarget.a libgcc.a)




 







































 










 

 


MEMORY
{
    ram : ORIGIN = 0, LENGTH = 0x200000
}

SECTIONS
{
    .debug_aranges  0 : { *(.debug_aranges) } .debug_pubnames 0 : { *(.debug_pubnames) } .debug_info     0 : { *(.debug_info) } .debug_abbrev   0 : { *(.debug_abbrev) } .debug_line     0 : { *(.debug_line) } .debug_frame    0 : { *(.debug_frame) } .debug_str      0 : { *(.debug_str) } .debug_loc      0 : { *(.debug_loc) } .debug_macinfo  0 : { *(.debug_macinfo) } 
    __reserved = 0; . = __reserved + 0x8000;
    .rom_vectors   ALIGN (0x4)  :      { . = . ; KEEP (*(.vectors)) } >  ram  
    .text   ALIGN (0x4)  :      { stext = ABSOLUTE(.); PROVIDE (__stext = ABSOLUTE(.)); *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) *(.glue_7) *(.glue_7t) } >  ram  etext = .; PROVIDE (__etext = .); 
    .fini   ALIGN (0x4)  :      { . = . ; *(.fini) } >  ram  
    .rodata   ALIGN (0x4)  :      { . = . ; *(.rodata*) } >  ram  
    .rodata1   ALIGN (0x4)  :      { . = . ; *(.rodata1) } >  ram  
    .fixup   ALIGN (0x4)  :      { . = . ; *(.fixup) } >  ram  
    .gcc_except_table   ALIGN (0x4)  :      { . = . ; *(.gcc_except_table) } >  ram  
    .data   ALIGN (0x4)  :      { __ram_data_start = ABSOLUTE (.); *(.data*) *(.data1)   _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.); _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.); . = ALIGN (4); __DEVTAB__ = ABSOLUTE (.); KEEP (*(SORT (.devtab*))) __DEVTAB_END__ = ABSOLUTE (.); __NETDEVTAB__ = ABSOLUTE (.); KEEP (*(SORT (.netdevtab*))) __NETDEVTAB_END__ = ABSOLUTE (.); __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); *(.dynamic) *(.sdata*) *(.sbss*) } >  ram  __rom_data_start = LOADADDR (.data); __ram_data_end = .; PROVIDE (__ram_data_end = .); _edata = .; PROVIDE (edata = .); PROVIDE (__rom_data_end = LOADADDR (.data) + SIZEOF(.data)); 
    .bss   ALIGN (0x4)  :      { __bss_start = ABSOLUTE (.); *(.scommon) *(.dynbss) *(.bss*) *(COMMON) __bss_end = ABSOLUTE (.); } >  ram  
    . = ALIGN(4); _end = .; PROVIDE (end = .); 
}

