	.set noreorder

#include "pspstub.s"

	STUB_START "ISOLoader",0x40090000,0x00010005
	STUB_FUNC  0x3D1D44CD,startISO
	STUB_END
