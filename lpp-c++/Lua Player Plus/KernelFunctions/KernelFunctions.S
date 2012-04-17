	.set noreorder

#include "pspstub.s"

	STUB_START "KernelFunctions",0x40090000,0x00070005
	STUB_FUNC  0x7106B2F3,StartISO
	STUB_FUNC  0xEA1A3788,LauncPops
	STUB_FUNC  0x487871A0,Read_eeprom
	STUB_FUNC  0xBD56102F,Write_eeprom
	STUB_FUNC  0x97B05960,getBaryon
	STUB_FUNC  0xE21C181C,getPommel
	STUB_FUNC  0xE2A5D1EE,sceSysregGetTachyonVersion
	STUB_END
