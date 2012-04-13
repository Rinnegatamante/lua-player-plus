#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "csoread.h"

extern int sceKernelDeflateDecompress(void *dst,int dsize,void *src,int pparam);

#define DEV_NAME "CSO"

// log output switch
#define OUT_LOG_ISO 0

// thread priority in read
// when READ_PRIORITY >= 48 then OutRun2006 are not work.
//#define READ_PRIORITY 47

// short sleep around read/decompress for higher thread switching
//#define SHORT_SLEEP

// index buffer size
#define CISO_INDEX_SIZE (512/4)

// compresed data buffer cache size
#define CISO_BUF_SIZE 0x2000
#define SECTOR_SIZE 0x800

#define SWITCH_THREAD() {}

/****************************************************************************
****************************************************************************/

//#ifdef CISO_BUF_SIZE
//static unsigned char ciso_data_buf[CISO_BUF_SIZE] __attribute__((aligned(64)));
//#else
//static unsigned char ciso_data_buf[0x800] __attribute__((aligned(64)));
//#endif
static unsigned char *ciso_data_buf = NULL; //0x0000A340

static unsigned int ciso_index_buf[CISO_INDEX_SIZE] __attribute__((aligned(64)));

static unsigned int ciso_buf_pos;    // file poisiotn of the top of ciso_data_buf //0x0000A580
static unsigned int ciso_cur_index;  // index(LBA) number of the top of ciso_index_buf //7e60

// header buffer
static CISO_H ciso; //7e48

/****************************************************************************
	Mount UMD event callback
****************************************************************************/
static int max_sectors;
FILE* umdfd = NULL;

//0x000057DC
int CisoOpen(FILE* _umdfd)
{
	int result;
	umdfd = _umdfd;

	// check CISO header
	ciso.magic[0] = 0;
	ciso_buf_pos  = 0x7FFFFFFF;
	//result = dhReadFileRetry(&ciso_fd, 0, &ciso, sizeof(ciso));
	fseek(umdfd, 0, SEEK_SET);
	result = fread(&ciso, 1, sizeof(ciso), umdfd);
	if(result < 0)
	{
		return result;
	}
	if(
		ciso.magic[0]=='C' &&
		ciso.magic[1]=='I' &&
		ciso.magic[2]=='S' &&
		ciso.magic[3]=='O'
	)
	{
		max_sectors = (int)(ciso.total_bytes) / ciso.block_size;
		ciso_cur_index = 0xffffffff;

		if (!ciso_data_buf)
		{
			ciso_data_buf = (unsigned char *)malloc(CISO_BUF_SIZE+64);

			if (!ciso_data_buf)
				return -1;

			if ((((u32)ciso_data_buf) % 64) != 0)
			{
				ciso_data_buf += (64 - (((u32)ciso_data_buf) % 64));
			}
		}

		return 0;
	}

	// header check error
	return 9;
}

/****************************************************************************
	get file pointer in sector
****************************************************************************/
int ReadUmdFileRetry(void *buf, u32 size, int fpointer){
	fseek(umdfd, fpointer, SEEK_SET);
	return fread(buf, 1, size, umdfd);
}

static int inline ciso_get_index(u32 sector,int *pindex)
{
	int result;
	int index_off;

	// search index
	index_off = sector - ciso_cur_index;
	if((ciso_cur_index==0xffffffff) || (index_off<0) || (index_off>=CISO_INDEX_SIZE) )
	{
		// out of area

		SWITCH_THREAD();
		//result = dhReadFileRetry(&ciso_fd,sizeof(ciso)+sector*4,ciso_index_buf,sizeof(ciso_index_buf));
		result = ReadUmdFileRetry(ciso_index_buf, sizeof(ciso_index_buf), sizeof(ciso)+sector*4);
		SWITCH_THREAD();

		if(result < 0) return result;

		// wait for long seek
		// sceKernelDelayThread(20000);

		ciso_cur_index = sector;
		index_off = 0;
	}

	// get file posision and sector size
	*pindex = ciso_index_buf[index_off];
	return 0;
}

/****************************************************************************
	Read one sector
****************************************************************************/
static int ciso_read_one(void *buf, int sector)
{
	int result;
	int index,index2;
	int dpos,dsize;

	// get current index
	result = ciso_get_index(sector, &index);
	if(result < 0) 
	{
		return result;
	}

	// get file posision and sector size
	dpos  = (index & 0x7fffffff) << ciso.align;//0x0000A59D

	if(index & 0x80000000)
	{
		// plain sector
		//result = dhReadFileRetry(&ciso_fd,dpos,buf,0x800);
		result = ReadUmdFileRetry(buf, 0x800, dpos);
		return result;
	}

	// compressed sector

	// get sectoer size from next index
	result = ciso_get_index(sector+1,&index2);
	if(result < 0){
		return result;
	}
	
	dsize = ((index2 & 0x7fffffff) << ciso.align) - dpos;
	
	// adjust to maximum size for scramble(shared) sector index
	if((dsize <= 0) || (dsize > 0x800)) dsize = 0x800;

	// read sector buffer
	if( (dpos < ciso_buf_pos) || ( (dpos+dsize) > (ciso_buf_pos+CISO_BUF_SIZE))  )
	{
		// seek & read
		//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,CISO_BUF_SIZE);
		result = ReadUmdFileRetry(ciso_data_buf, CISO_BUF_SIZE, dpos);
		if(result<0)
		{
			ciso_buf_pos = 0xfff00000; // set invalid position
			return result;
		}
		ciso_buf_pos = dpos;
	}
	result = sceKernelDeflateDecompress(buf, 0x800, ciso_data_buf + dpos - ciso_buf_pos, CISO_BUF_SIZE);

	if(result < 0){
		return result;
	}

	return 0x800;
}

/****************************************************************************
	Read Request
****************************************************************************/
//0x000055AC
int CisofileReadSectors(int lba, int nsectors, void *buf, int *eod)
{
	int result;
	u32 i;

	u32 num_bytes = nsectors * 0x800;

	for(i = 0; i < num_bytes; i+=0x800)
	{
		result = ciso_read_one(buf, lba);
		if(result < 0)
		{
			nsectors = result;
			break;
		}
		buf += 0x800;
		lba++;
	}

	return nsectors;
}

/****************************************************************************
****************************************************************************/
//0x000058D8
int CisofileGetDiscSize(FILE* umdfd)
{
	return (int)(ciso.total_bytes) / ciso.block_size;
}

