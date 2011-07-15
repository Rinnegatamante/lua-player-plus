	.set noreorder

#include "pspstub.s"

	STUB_START "KernelFunctions",0x40090000,0x00070005
	STUB_FUNC  0x3D1D44CD,startISO
	STUB_FUNC  0xC134E355,launch_pops
	STUB_FUNC  0x7BB9197B,read_eeprom
	STUB_FUNC  0xC323E11F,write_eeprom
	STUB_FUNC  0x97B05960,getBaryon
	STUB_FUNC  0xE21C181C,getPommel
	STUB_FUNC  0xE2A5D1EE,sceSysregGetTachyonVersion
	STUB_END

