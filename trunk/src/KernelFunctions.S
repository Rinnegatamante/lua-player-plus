	.set noreorder

#include "pspstub.s"

	STUB_START "ISOLoader",0x40090000,0x00020005
	STUB_FUNC  0x3D1D44CD,startISO
	STUB_FUNC  0xC134E355,launch_pops
	STUB_END
