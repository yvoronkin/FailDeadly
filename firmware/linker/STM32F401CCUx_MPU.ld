/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* end of RAM memory */

_Min_Heap_Size  = 0x200;
_Min_Stack_Size = 0x400;

/* Memory definitions */
MEMORY
{
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
}

__privileged_functions_region_size__ = 32K;
__privileged_data_region_size__      = 8K;

__FLASH_segment_start__ = ORIGIN(FLASH);
__FLASH_segment_end__   = ORIGIN(FLASH) + LENGTH(FLASH);

__SRAM_segment_start__  = ORIGIN(RAM);
__SRAM_segment_end__    = ORIGIN(RAM) + LENGTH(RAM);



SECTIONS
{
    .privileged_functions ORIGIN(FLASH):
    {
        . = ALIGN(4);
        __privileged_functions_start__ = .;
        /* The startup code */
        KEEP(*(.isr_vector))
        . = ALIGN(4);

        /* priv sections */
        KEEP(*(privileged_functions))
        KEEP(*(privileged_functions.*))

        . = ALIGN(4);
        __privileged_functions_actual_end__ = .;

        /* fill rest of privileged region*/
        FILL(0xDEAD);

        . = ORIGIN(FLASH) + __privileged_functions_region_size__;
        __privileged_functions_end__ = .;
    } >FLASH

    .text :
    {
        . = ALIGN(32);

        __syscalls_flash_start__ = .;

        KEEP(*(freertos_system_calls))
        KEEP(*(freertos_system_calls.*))

        . = ALIGN(32);
        __syscalls_flash_end__ = .;

        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* read-only data */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >FLASH

    .ARM.extab   : {
        . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } >FLASH

    .ARM : {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } >FLASH

    .preinit_array     :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
    } >FLASH

    .init_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
    } >FLASH

    .fini_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
    } >FLASH


   .privileged_data ORIGIN(RAM) :
    {
        . = ALIGN(8);

        __privileged_data_start__ = .;
        __privileged_data_actual_start__ = .;

        KEEP(*(privileged_data))
        KEEP(*(privileged_data.*))

        . = ALIGN(8);
        __privileged_data_actual_end__ = .;

    } >RAM AT> FLASH

    __privileged_data_load_start__ = LOADADDR(.privileged_data);
    __privileged_data_end__ = __privileged_data_actual_end__;

    .privileged_data_reserve
    __privileged_data_actual_end__ (NOLOAD) :
    {
        . = ALIGN(8);

        /* reserve all mpu region */
        . = ORIGIN(RAM) + __privileged_data_region_size__;

        __privileged_data_end__ = .;
    } > RAM

    /* Used by the startup to initialize data */
    _sidata = LOADADDR(.data);

    /* Initialized data sections into RAM memory */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        *(.RamFunc)
        *(.RamFunc*)

        . = ALIGN(4);
        _edata = .;
    } >RAM AT> FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;

        __bss_end__ = _ebss;
    } >RAM

    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    /* Remove information from the compiler libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}

ASSERT(
    __privileged_data_actual_end__ <= __privileged_data_end__,
    "Privileged data exceeds MPU region"
)

ASSERT(
    __privileged_functions_actual_end__ <=
        __privileged_functions_end__,
    "Privileged functions exceed MPU region"
)

ASSERT(
    (__privileged_functions_start__ %
        __privileged_functions_region_size__) == 0,
    "Privileged flash base is not MPU aligned"
)

ASSERT(
    (__privileged_data_start__ %
        __privileged_data_region_size__) == 0,
    "Privileged RAM base is not MPU aligned"
)
