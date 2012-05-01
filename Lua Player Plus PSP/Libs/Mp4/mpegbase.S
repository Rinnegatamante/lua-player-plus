	.set noreorder

#include "pspimport.s"

	IMPORT_START	"sceMpegbase",0x00090000
	IMPORT_FUNC	"sceMpegbase",0xBEA18F91,sceMpegbase_BEA18F91
	IMPORT_FUNC	"sceMpegbase",0xBE45C284,sceMpegBaseYCrCbCopyVme
	IMPORT_FUNC	"sceMpegbase",0x7AC0321A,sceMpegBaseYCrCbCopy
	IMPORT_FUNC	"sceMpegbase",0x492B5E4B,sceMpegBaseCscInit
	IMPORT_FUNC	"sceMpegbase",0xCE8EB837,sceMpegBaseCscVme
	IMPORT_FUNC	"sceMpegbase",0x91929A21,sceMpegBaseCscAvc