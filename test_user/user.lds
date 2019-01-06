OUTPUT_ARCH(arm)
ENTRY(main)

SECTIONS
{
	. = 0x00000000;
	.text : 
	{
		*(.text)
	}
	. = ALIGN(32);
										
	.data : 
	{
		*(.data)
	}

	. = ALIGN(32);
	__bss_start__	= .;
	.bss : 
	{
		*(.bss)
	}
	__bss_end__ = .;
}
