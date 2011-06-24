	.set noreorder

#include "pspstub.s"

	STUB_START "loadlib",0x40090000,0x00020005
	STUB_FUNC  0x23C4746A,findFunction
	STUB_FUNC  0xBFA30E77,psploadlib
	STUB_END
