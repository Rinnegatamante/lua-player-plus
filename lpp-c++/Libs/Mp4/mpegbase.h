/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspvideocodec.h - Prototypes for the sceVideocodec library.
 *
 * Copyright (c) 2007 cooleyes
 *
 */

#ifndef mpegbase_h
#define mpegbase_h

#define DMABLOCK 4095
#define MEAVCBUF 0x4a000
#define MEMP4VBUF 0xD0000


struct SceMpegLLI
	{
	ScePVoid pSrc;
	ScePVoid pDst;
	ScePVoid Next;
	SceInt32 iSize;
	};

SceInt32 sceMpegbase_BEA18F91(struct SceMpegLLI *p);

SceInt32 sceMpegBaseYCrCbCopyVme(unsigned long* destBuffer, unsigned long *srcBuffer, int unused);

SceInt32 sceMpegBaseCscInit(SceInt32 width);

SceInt32 sceMpegBaseCscVme(ScePVoid p1, ScePVoid p2, SceInt32 width, ScePVoid p3);

#endif
