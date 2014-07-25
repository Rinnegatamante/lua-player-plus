/** LPP UMD lib by Nanni */

/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <cusunanni@hotmail.it> --------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@eternalongju2.com> -----------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Official Forum : http://rinnegatamante.eu/luaplayerplus/forum.php ---------------------------------------------------#
#- For help using LuaPlayerPlus, coding help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php #
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Homemister for LPHM sourcecode --------------------------------------------------------------------------------------#
#- Zack & Shine for LP Euphoria sourcecode -----------------------------------------------------------------------------#
#- ab5000 for support on psp-ita.com -----------------------------------------------------------------------------------#
#- valantin for sceIoMvdir and sceIoCpdir improved functions------------------------------------------------------------#
#- Dark_AleX for usbdevice ---------------------------------------------------------------------------------------------#
#- VirtuosFlame & ColdBird for iso drivers and kuBridge ----------------------------------------------------------------#
#- sakya for Media Engine and OslibMod ---------------------------------------------------------------------------------#
#- Booster & silverspring for EEPROM write/read functions --------------------------------------------------------------#
#- Akind for RemoteJoyLite ---------------------------------------------------------------------------------------------#
#- cooleyes for mpeg4 lib ----------------------------------------------------------------------------------------------#
#- Arshia001 for PSPAALib ----------------------------------------------------------------------------------------------#
#- InsertWittyName & MK2k for PGE sourcecode ---------------------------------------------------------------------------#
#- Youresam for LUA BMPLib ---------------------------------------------------------------------------------------------#
#- Raphael for vram manager code ---------------------------------------------------------------------------------------#
#- Dynodzzo for LSD concepts -------------------------------------------------------------------------------------------#
#- ab_portugal for Image.negative function -----------------------------------------------------------------------------#
#- JiCé for drawCircle function ----------------------------------------------------------------------------------------#
#- Rapper_skull & DarkGiovy for testing LuaPlayer Plus and coming up with some neat ideas for it. ----------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <pspkernel.h>
#include "Umd.h"
#include <pspumd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define null (0L)

#ifdef DEBUG
extern "C" 
{
int dwrite_output(const char *format, ...);
int debug_output(const char *format, ...);
}
#endif

unsigned char *UMD::Buffer[LPP_UMD_BUFFER_SIZE] = { };

UMD::UMD() {
}

UMD::~UMD() {
}

int UMD::Init(void) {
    int i = sceUmdCheckMedium();
	if(i == 0) return -1;
	i = sceUmdActivate(1, "disc0:");
	if(i < 0) return -1;
	i = sceUmdWaitDriveStat(UMD_WAITFORINIT);
	return i;
}

int UMD::GetTitle(char *title) {
    int fd = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
	if(fd < 0) return -1;
	sceIoRead(fd, title, 10);
	title[10] = 0;
	sceIoClose(fd);
	return 1;
}

unsigned int UMD::GetSize(void) {
    unsigned int min, max, test;
	min = 0; max = 1024 * 1024;
	while(max > (min + 1)) {
	  test = (max + min) / 2;
	  if(size_is_valid(test))
	   min = test;
	  else
	   max = test;
	}
	return min - 1;
}

bool UMD::size_is_valid(unsigned int blocks) {
    char blockfile[128];
	int fd, r;
	sprintf(blockfile, "disc0:/sce_lbn0x%x_size0x%x", blocks - 1, 1024 * 2);
	fd = sceIoOpen(blockfile, PSP_O_RDONLY, 0777);
	if(fd < 0) return false;
	r = sceIoRead(fd, Buffer, 1024 * 2);
	sceIoClose(fd);
	return (r > 0);
}

int UMD::DumpISO(const char *filename) {
    char blockfile[128];
	int fdin, fdout, rd, wr;
	unsigned long int size, bread, bwritten;
	unsigned int blocks = GetSize();
    sprintf(blockfile, "disc0:/sce_lbn0x%x_size0x%lu", 0, blocks * 2048L);
	fdin = sceIoOpen(blockfile, PSP_O_RDONLY, 0777);
	if(fdin < 0) return -1;
	fdout = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	bread = bwritten = 0L;
	size = blocks * 2048L;
	while(bread < size) {
	  rd = sceIoRead(fdin, Buffer, MIN(size, LPP_UMD_BUFFER_SIZE));
	  bread += rd;
	  wr = sceIoWrite(fdout, Buffer, rd);
	  if(wr < 0) return -1;
	  bwritten += wr;
	}
	sceIoClose(fdin);
	sceIoClose(fdout);
	return 1;
}

static ciso_header ciso;
static int ciso_total_block;
static z_stream z;

static unsigned int *ciso_index_buf = null;
static unsigned int *ciso_crc_buf = null;

static unsigned char *ciso_block_buf1 = null;
static unsigned char *ciso_block_buf2 = null;

unsigned long long check_file_size(int fd)
{
    unsigned long long pos;

    pos = sceIoLseek32(fd, 0, PSP_SEEK_END);
    if(pos == (unsigned long long)-1) return pos;

    memset(&ciso, 0, sizeof(ciso));

    ciso.magic[0] = 'C';
    ciso.magic[1] = 'I';
    ciso.magic[2] = 'S';
    ciso.magic[3] = 'O';

    ciso.ver = 0x01;

    ciso.block_size = 0x800;
    ciso.total_bytes = pos;

#if 0
    for(ciso.align = 0 ; (ciso.total_bytes >> ciso.align) >0x80000000LL ; ciso.align++);
#endif

    ciso_total_block = (pos / ciso.block_size);

    sceIoLseek32(fd, 0, PSP_SEEK_SET);

    return pos;
}

int UMD::DumpCSO(const char *filename, int clevel) {
    #ifdef DEBUG
    debug_output("Initializing cso dumping...");
	#endif
    unsigned long long file_size;
    unsigned long long write_pos;

    int index_size;
    int block;
    unsigned char buf4[64];
    int cmp_size;
    int status;
    int percent_period;
    int percent_cnt;
    int align, align_b, align_m;

    char blockfile[128];
	int ciso_fin, ciso_fout;

	unsigned int blocks = GetSize();
    sprintf(blockfile, "disc0:/sce_lbn0x%x_size0x%lu", 0, blocks * 2048L);
	ciso_fin = sceIoOpen(blockfile, PSP_O_RDONLY, 0777);
	if(ciso_fin < 0) return -1;

	ciso_fout = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	file_size = check_file_size(ciso_fin);

	if(file_size < 0)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Can't get file size.\n", __FUNCTION__, __LINE__);
		#endif
        sceIoClose(ciso_fin);
        sceIoClose(ciso_fout);
        return -1;
    }

    index_size = (ciso_total_block + 1) * sizeof(unsigned long);
    ciso_index_buf = new unsigned int[index_size];
    ciso_crc_buf = new unsigned int[index_size];

    ciso_block_buf1 = new unsigned char[ciso.block_size];
    ciso_block_buf2 = new unsigned char[ciso.block_size * 2];

    if(!ciso_index_buf || !ciso_crc_buf ||!ciso_block_buf1 || !ciso_block_buf2)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Can't allocate memory.\n", __FUNCTION__, __LINE__);
		#endif
        sceIoClose(ciso_fin);
        sceIoClose(ciso_fout);
        return -1;
    }

    memset(ciso_index_buf, 0, index_size);
    memset(ciso_crc_buf, 0, index_size);
    memset(buf4, 0, sizeof(buf4));

    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;

    sceIoWrite(ciso_fout, &ciso, sizeof(ciso));
    sceIoWrite(ciso_fout, ciso_index_buf, index_size);

    write_pos = sizeof(ciso) + index_size;

    percent_period = ciso_total_block / 100;
    percent_cnt = percent_period;

    align_b = 1 << (ciso.align);
    align_m = align_b - 1;

    for(block = 0; block < ciso_total_block; block++)
    {
        if(--percent_cnt <= 0)
        {
            percent_cnt = percent_period;
			#ifdef DEBUG
			debug_output("Progress : %d", (block / percent_period));
			#endif
        }

        if(deflateInit2(&z, clevel, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
		    #ifdef DEBUG
            dwrite_output("Function %s Line %d : DeflateInit : %s-\n", __FUNCTION__, __LINE__, (z.msg) ? z.msg : "???");
			#endif
            sceIoClose(ciso_fin);
            sceIoClose(ciso_fout);
            return -1;
        }

        align = (int)write_pos & align_m;

        if(align)
        {
            align = align_b - align;
            if(sceIoWrite(ciso_fout, buf4, align) != (int)align)
            {
			    #ifdef DEBUG
                dwrite_output("Function %s Line %d : Block %d write error.\n", __FUNCTION__, __LINE__, block);
				#endif
                sceIoClose(ciso_fin);
                sceIoClose(ciso_fout);
                return 1;
            }

            write_pos += align;
        }

        ciso_index_buf[block] = write_pos >> (ciso.align);

        z.next_out = ciso_block_buf2;
        z.avail_out = ciso.block_size * 2;
        z.next_in = ciso_block_buf1;
        z.avail_in = sceIoRead(ciso_fin, ciso_block_buf1, ciso.block_size);

        if(z.avail_in != ciso.block_size)
        {
		    #ifdef DEBUG
            dwrite_output("Function %s Line %d : Block %d read error.\n", __FUNCTION__, __LINE__, block);
			#endif
            sceIoClose(ciso_fin);
            sceIoClose(ciso_fout);
            return -1;
        }

        status = deflate(&z, Z_FINISH);
        if(status != Z_STREAM_END)
        {
		    #ifdef DEBUG
            dwrite_output("Function %s Line %d : deflate : %s[%d]\n", __FUNCTION__, __LINE__, block, z.msg ? z.msg : "???", status);
			#endif
            sceIoClose(ciso_fin);
            sceIoClose(ciso_fout);
            return -1;
        }

        cmp_size = ciso.block_size * 2  - z.avail_out;

        if(cmp_size >= (int)ciso.block_size)
        {
            cmp_size = ciso.block_size;
            memcpy(ciso_block_buf2, ciso_block_buf1, cmp_size);
            ciso_index_buf[block] |= 0x80000000;
        }

        if(sceIoWrite(ciso_fout, ciso_block_buf2, cmp_size) != (int)cmp_size)
        {
		    #ifdef DEBUG
            dwrite_output("Function %s Line %d : block %d write error.\n", __FUNCTION__, __LINE__, block);
			#endif
            sceIoClose(ciso_fin);
            sceIoClose(ciso_fout);
            return -1;
        }

        write_pos += cmp_size;

        if(deflateEnd(&z) != Z_OK)
        {
		    #ifdef DEBUG
            dwrite_output("Function %s Line %d : deflateEnd : %s.", __FUNCTION__, __LINE__, z.msg ? z.msg : "???");
			#endif
            sceIoClose(ciso_fin);
            sceIoClose(ciso_fout);
            return -1;
        }
    }

    ciso_index_buf[block] = write_pos >> (ciso.align);

    sceIoLseek32(ciso_fout, sizeof(ciso), PSP_SEEK_SET);
    sceIoWrite(ciso_fout, ciso_index_buf, index_size);

    if(ciso_index_buf) delete(ciso_index_buf);
    if(ciso_crc_buf) delete(ciso_crc_buf);
    if(ciso_block_buf1) delete(ciso_block_buf1);
    if(ciso_block_buf2) delete(ciso_block_buf2);

    sceIoClose(ciso_fin);
    sceIoClose(ciso_fout);

	return 0;
}
