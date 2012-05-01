	.set noreorder

#include "pspstub.s"

	STUB_START "cooleyesBridge",0x40090000,0x00020005
	STUB_FUNC  0x53E6D624,cooleyesAudioSetFrequency
	STUB_FUNC  0x90CC7DD7,cooleyesMeBootStart
	STUB_END
