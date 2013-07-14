ENTRY (storm)
OUTPUT_FORMAT("binary")
SECTIONS{
    . = 0x00100000;

    .text ALIGN (0x1000) : {
	*(.text)
    }

    . = ALIGN (0x1000);

    .rodata ALIGN (0x1000): {
	*(.rodata)
    }

    .data ALIGN (0x1000): {
	*(.data)
    }

    .bss ALIGN (0x1000) : {
	_bss = .;
	*(.bss)
	. = ALIGN (0x1000);
        _bss_ = .;

    }
    _kernel_ = .;
}

