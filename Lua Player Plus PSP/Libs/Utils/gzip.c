#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char

//#define CHUNK 16384

int sceKernelDeflateDecompress(u8* dest, u32 destSize, u8* src, u32 inSize){
	int ret;
	unsigned have;
	z_stream strm;
	u8 out[0x800];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, -15);
	if(ret != Z_OK){
		return ret;
	}

	strm.avail_in = inSize;
	strm.next_in = src;
	strm.avail_out = destSize;
	strm.next_out = out;

	ret = inflate(&strm, Z_NO_FLUSH);
	assert(ret != Z_STREAM_ERROR);
	switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			return ret;
	}
	have = destSize - strm.avail_out;
	memcpy(dest, out, have);

	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
