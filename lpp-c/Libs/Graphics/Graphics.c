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
#-----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <malloc.h>

#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>

#include <png.h>
#include <jpeglib.h>
#include <jerror.h>

#include <assert.h>
#include <psprtc.h>

#include "../Vram/vram.h"

#include "Graphics.h"
#include "../Math/Math.h"
#include "../LibnsGif/libnsgif.h"

#include "vera.c"
#include "veraMono.c"

static u32 __attribute__((aligned(64))) dList[2][(262144) >> 2];

static u32 *lppg_vram_base = (u32 *) (0x40000000 | 0x04000000);

static int dBuffer_n = 0;
static u8 Initialized = 0;

static void *BackBuffer  = null;
static void *FrontBuffer = null;
static void *DepthBuffer = null;
static void *FrameBuffer = null;

extern u8 msx[];

static u64 timeNow;
static u64 timeLastAsk;
static u32 tickResolution;

static int fps = 0;
static u64 timeNowFPS;
static u64 timeLastFPS;

#ifdef DEBUG
extern int dwrite_output(L_CONST char *format, ...);
#endif

u32 *LPPG_GetVDrawBuffer(void) {
    u32 *vram = (u32 *)lppg_vram_base;
    if(dBuffer_n == 0) vram += PSP_FRAME_BUFFER_SIZE / sizeof(u32);
    return (vram);
}

u32 *LPPG_GetVDisplayBuffer(void) {
    u32 *vram = (u32 *)lppg_vram_base;
    if(dBuffer_n == 1) vram += PSP_FRAME_BUFFER_SIZE / sizeof(u32);
    return (vram);
}

u32 *LPPG_GetFrameBuffer(void) {
    return (u32*)FrameBuffer;
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
    (void)png_ptr;
    (void)warning_msg;
}

int LPPG_IsPNG(L_CONST char *filename) {
    png_byte fsig[8] = { 0x0 };

    FILE *source = fopen(filename, "rb");

    if(!source)
    {
        return 0;
    }

    fread(fsig, 8, 1, source);
    fclose(source);

    return png_check_sig(fsig, 8);
}

int LPPG_IsJPG(L_CONST char *filename) {
    char fsig[10] = "";

    FILE *source = fopen(filename, "rb");

    if(!source)
    {
        return 0;
    }

    fread(fsig, 10, 1, source);
    fclose(source);

    if((fsig[6] == 'J' && fsig[7] == 'F' && fsig[8] == 'I' && fsig[9] == 'F'))
        return 1;

    return 0;
}

int LPPG_IsBMP(L_CONST char *filename) {
    char fsig[2] = "";

	FILE *source = fopen(filename, "rb");

	if(!source)
	{
	    return 0;
	}

	fread(fsig, 2,1,source);
	fclose(source);

	if(fsig[0] == 'B' && fsig[1] == 'M')
	   return 1;

	 return 0;
}

int LPPG_IsTGA(L_CONST char *filename)
{
    char fsig[32] = "";

    FILE *source = fopen(filename, "rb");

    if(!source)
    {
        return 0;
    }

    fseek(source, -32, SEEK_END);
    fread(fsig, 32, 1, source);
    fclose(source);

    if(strstr(fsig + 14, "TRUEVISION-XFILE"))
        return 1;

    return 0;
}

int LPPG_IsGIF(L_CONST char *filename)
{
    char fsig[3] = "";

    FILE *source = fopen(filename, "rb");

    if(!source)
    {
        return 0;
    }

    fread(fsig, 3, 1, source);
    fclose(source);

    if(fsig[0] == 'G' && fsig[1] == 'I' && fsig[2] == 'F')
        return 1;

    return 0;
}

u8 LPPG_IsInPsar(L_CONST char *f)
{
    if(strlen(f) < 10) return GU_FALSE;

    if(f[0] == 'E' && f[1] == 'B' && f[2] == 'O' && f[3] == 'O' && f[4] == 'T' &&
       f[5] == '.' && f[6] == 'P' && f[7] == 'B' && f[8] == 'P' && (f[9] == '/' || f[9] == '\\'))
       return GU_TRUE;

    return GU_FALSE;
}

void LPPG_InitTimer(void)
{
    sceRtcGetCurrentTick(&timeLastAsk);
    sceRtcGetCurrentTick(&timeLastFPS);
    tickResolution = sceRtcGetTickResolution();
}

float LPPG_GetDeltaTime(void)
{
    sceRtcGetCurrentTick(&timeNow);
    float dt = (timeNow - timeLastAsk) / ((float)tickResolution);
    timeLastAsk = timeNow;

    return(dt);
}

int LPPG_GetFPS(void)
{
    fps += 1;
    sceRtcGetCurrentTick(&timeNowFPS);
    if(((timeNowFPS - timeLastFPS) / ((float)tickResolution)) >= 1.0f)
    {
        timeLastFPS = timeNowFPS;
    }
    int ret = fps;
    fps = 0;
    return ret;
}

LPP_Surface *LPPG_LoadImagePNGImpl(png_structp png_ptr) {
    png_infop info_ptr = null;
    u32 sig_read = 0;
    png_uint_32 width, height, x, y;
    int bpp, color_type, interlace_type;
    u32 *line = null;

    LPP_Surface *tmp = (LPP_Surface *)malloc(LPP_Surface_size);
    if(!tmp) return null;

    png_set_error_fn(png_ptr, null, null, user_warning_fn);

    info_ptr = png_create_info_struct(png_ptr);

    if(!info_ptr) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'info_ptr'.\n", __FUNCTION__, __LINE__);
		#endif
	    free(tmp);
		png_destroy_read_struct(&png_ptr, null, null);
		return null;
	}

    png_set_sig_bytes(png_ptr, sig_read);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpp, &color_type, &interlace_type, null, null);

    if(width > 512 || height > 512) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Image dimensions are highter than 512.\n", __FUNCTION__, __LINE__);
		#endif
	    free(tmp);
		png_destroy_read_struct(&png_ptr, null, null);
		return null;
	}

    tmp->width = width;
    tmp->height = height;

    tmp->realW = NextPow2(width);
    tmp->realH = NextPow2(height);

    tmp->swizzled = 0;

    png_set_strip_16(png_ptr);
    png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if(color_type == PNG_COLOR_TYPE_GRAY && bpp < 8) png_set_gray_1_2_4_to_8(png_ptr);
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    tmp->pixels = memalign(16, tmp->realW * tmp->realH * sizeof(u32));
    tmp->location = LPP_RAM;

    if(!tmp->pixels) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate image pixels to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    free(tmp);
		png_destroy_read_struct(&png_ptr, null, null);
		return null;
	}

    line = (u32 *)malloc(width * 4);

    if(!line) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'line' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    free(tmp->pixels);
		free(tmp);
		png_destroy_read_struct(&png_ptr, null, null);
		return null;
	}

    for(y = 0; y < height; y++) {
        png_read_row(png_ptr, (u8 *)line, null);
        for(x = 0; x < width; x++) tmp->pixels[x + y * tmp->realW] = line[x];
    }

    tmp->bpp = 4;

    free(line);
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, null);

    return (tmp);
}

LPP_Surface* LPPG_LoadImagePNG(L_CONST char *filename) {
    png_structp png_ptr = null;
    FILE *fp = null;

    if((fp = fopen(filename, "rb")) == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open '%s' for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
	    return null;
	}

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, null, null, null);
    if(png_ptr == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'png_ptr'.\n", __FUNCTION__, __LINE__);
		#endif
        fclose(fp);
        return null;
    }

    png_init_io(png_ptr, fp);
    LPP_Surface *tmp = LPPG_LoadImagePNGImpl(png_ptr);
    fclose(fp);
    return (tmp);
}

LPP_Surface* LPPG_LoadImageBMPImpl(u8 *data, size_t in_size, u8 *inv)
{
    LPP_Surface *tmp = null;
    size_t sg = 0;

    u8 fheader[14] = { 0x0 };

    memcpy(fheader, data, 14);
    sg += 14;

    u8 inversed = GU_TRUE;

    if(Bm32(fheader, 2) != in_size)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : bitmap header doesn't contains the correct size.\n", __FUNCTION__, __LINE__);
		#endif
        return null;
    }

    u32 data_ofs = Bm32(fheader, 10);

    u8 header[40] = { 0x0 };
    memcpy(header, data + sg, 40);
    sg += 40;

    int compression = Bm16(header, 18);
    int planes = 0;

    u32 height, width;

    u32 bpp = 0;

    if(compression > 3)
    {
        width = Bm16(header, 4);
        height = Bm16(header, 6);
        planes = Bm16(header, 8);
        bpp = Bm16(header, 10);
        compression = 0;
        header[18] = 3;
    }
    else
    {
        width = Bm32(header, 4);
        height = Bm32(header, 8);
        planes = Bm16(header, 12);
        bpp = Bm16(header, 14);
        header[18] = 4;
    }

    if((planes != 1) || (bpp < 8))
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Incorrect number of planes or bit depth is too low.\n", __FUNCTION__, __LINE__);
		#endif
        LPPG_FreeSurface(tmp);
        return null;
    }

    if(height < 0) {
        height = -height;
        inversed = GU_FALSE;
    }

    tmp = LPPG_CreateSurface(width, height);
    tmp->bpp = bpp;

    sg = 0;
    header[19] = (u8)data_ofs;

    if(compression == 0)
    {
        if(tmp->bpp <= 8)
        {
            int palette_bpp = header[18];
            int palette_sz = (1 << tmp->bpp) * palette_bpp;

            u8 *palette = (u8*)malloc(palette_sz);
            memcpy(palette, data + sg, palette_sz);
            sg += palette_sz;
            int dst = 0;

            u8 *buffer = (u8*)malloc(width);
            int j, i;
            for(j = 0; j < height; j++, dst += (tmp->realW - width)) {
                memcpy(buffer, data + sg, width);
                sg += width;
                for(i = 0; i < width; i++, dst++) {
                    u8 c = buffer[50 + palette_sz + i];
                    if(tmp->bpp == 8) tmp->pixels[dst] = RGB(palette[c * palette_bpp + 2], palette[c * palette_bpp + 1], palette[c * palette_bpp]);
                }
            }

            free(palette);
            free(buffer);
        }

    if(tmp->bpp >= 16)
    {
        sg = data_ofs;
        int src = 0, dst = 0;
        int jump = (tmp->bpp / 8);

        u8 *buffer = (u8*)malloc(width * jump);
        int j, i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width)) {
            memcpy(buffer, data + sg, (width * jump));
            sg += (width * jump);
            src = 0;
            for(i = 0; i < width; i++, src += jump, dst++) {
                if(tmp->bpp == 16) {
                    u16 c = Bm16(buffer, src);
                    u8 r = (u8)(((c & 0x7C00) >> 10) << 3);
                    u8 g = (u8)(((c & 0x03E0) >>  5) << 3);
                    u8 b = (u8)(((c & 0x001F) >>  3));
                    tmp->pixels[dst] = RGB(r, g, b);
                }
                if(tmp->bpp == 24)
                {
                    tmp->pixels[dst] = RGB(buffer[src + 2], buffer[src + 1], buffer[src]);
                }
                if(tmp->bpp == 32) {
                    tmp->pixels[dst] = RGBA(buffer[src + 2], buffer[src + 1], buffer[src], buffer[src + 3]);
                }
            }
        }
        free(buffer);
    }
    }

    *inv = inversed;

    return (tmp);
}

LPP_Surface *LPPG_LoadImageJPGImpl(struct jpeg_decompress_struct dinfo) {
    jpeg_read_header(&dinfo, TRUE);
    u32 width = dinfo.image_width;
    u32 height = dinfo.image_height;
    jpeg_start_decompress(&dinfo);

    if(width > 512 || height > 512) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Image dimensions are highter than 512.\n", __FUNCTION__, __LINE__);
		#endif
        jpeg_destroy_decompress(&dinfo);
        return null;
    }

    LPP_Surface *tmp = (LPP_Surface*)malloc(sizeof(LPP_Surface));
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
        jpeg_destroy_decompress(&dinfo);
        return null;
    }

    tmp->width = width;
    tmp->height = height;
    tmp->realW = NextPow2(width);
    tmp->realH = NextPow2(height);
    tmp->pixels = (u32*)memalign(16, tmp->realH * tmp->realW * sizeof(u32));
    tmp->location = LPP_RAM;

    u8 *line = (u8*)malloc(width * 3);
    if(!line) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'line' to memory.\n", __FUNCTION__, __LINE__);
		#endif
        LPPG_FreeSurface(tmp);
        jpeg_destroy_decompress(&dinfo);
        return null;
    }

    if(dinfo.jpeg_color_space == JCS_GRAYSCALE) {
        while(dinfo.output_scanline < dinfo.output_height) {
            int y = dinfo.output_scanline, x;
            jpeg_read_scanlines(&dinfo, &line, 1);
            for(x = 0; x < width; x++) {
                u32 c = line[x];
                tmp->pixels[x + tmp->realW * y] = c | (c << 8) | (c << 16) | 0xff000000;
            }
        }
    } else {
        while(dinfo.output_scanline < dinfo.output_height) {
            int y = dinfo.output_scanline, x;
            jpeg_read_scanlines(&dinfo, &line, 1);
            u8* line_ptr = line;
            for(x = 0; x < width; x++) {
                u32 c  = *(line_ptr++);
                c |= (*(line_ptr++)) << 8;
                c |= (*(line_ptr++)) << 16;
                tmp->pixels[x + tmp->realW * y] = c | 0xff000000;
            }
        }
    }

    tmp->bpp = 4;

    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    free(line);

    return tmp;
}

LPP_Surface *LPPG_LoadImageJPG(L_CONST char *filename)
{
    struct jpeg_decompress_struct dinfo;
    struct jpeg_error_mgr jerr;
    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);

    FILE *fp = fopen(filename, "rb");
    if(!fp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
        jpeg_destroy_decompress(&dinfo);
        return null;
    }

    jpeg_stdio_src(&dinfo, fp);
    LPP_Surface *tmp = LPPG_LoadImageJPGImpl(dinfo);
    fclose(fp);

    return (tmp);
}

LPP_Surface *LPPG_LoadImageBMP(L_CONST char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open ghe file '%s' for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
	    return null;
	}

    fseek(fp, 0, SEEK_END);
    size_t in_size = ftell(fp);
    rewind(fp);

    u8 *data = (u8*)malloc(in_size);
	if(!data) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'data' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fp);
		return null;
	}

    fread(data, in_size,1,fp);
    u8 inversed = GU_FALSE;
    LPP_Surface *ld = LPPG_LoadImageBMPImpl(data, in_size, &inversed);
    fclose(fp);

    if(ld == null) return null;
    if(inversed == GU_TRUE)
    {
        LPP_Surface *tmp = LPPG_FlipSurfaceVertical(ld);
        LPPG_FreeSurface(ld);
        return tmp;
    } else return(ld);
}

LPP_Surface *LPPG_LoadImageTGAImpl(u8 *data, size_t len)
{
    u8 header[20] = { 0x0 };

    size_t sg = 0;

    memcpy(header, data, 18);
    sg += 18;

    u32 width = (header[13] * 256 + header[12]);
    u32 height = (header[15] * 256 + header[14]);

    if((width <= 0) || (height <= 0)) {
        return null;
    }

    LPP_Surface *tmp = LPPG_CreateSurface(width, height);
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
		return null;
	}

    tmp->bpp = header[16];

    if(header[2] == 1)
    {
        int palette_bpp = (header[7] / 8);
        int palette_count = (header[5] + (header[6] << 8));

        u8 *palette = (u8*)malloc(palette_bpp * palette_count);
		if(!palette) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'palette' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        memcpy(palette, data + sg, (palette_count * palette_bpp));
        sg += (palette_count * palette_bpp);

        u8 *buffer = (u8*)malloc(width * height);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			free(palette);
			LPPG_FreeSurface(tmp);
			return null;
		}

        memcpy(palette, data + sg, (width * height));
        sg += (width * height);

        int src = 0, dst = 0;

        int j, i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width))
        {
            for(i = 0; i < width; i++, dst++, src++) {
                if(palette_bpp == 4) tmp->pixels[dst] = RGBA(palette[buffer[src] * 4 + 2], palette[buffer[src] * 4 + 1], palette[buffer[src] * 4], palette[buffer[src] * 4 + 3]);
                if(palette_bpp == 3) tmp->pixels[dst] = RGB(palette[buffer[src] * 3 + 2], palette[buffer[src] * 3 + 1], palette[buffer[src] * 3]);
                if(palette_bpp == 2) {
                    u8 r = (u8)(((palette[buffer[src] * 2 + 2] & 0x7C00) << 10) << 3);
                    u8 g = (u8)(((palette[buffer[src] * 2 + 1] & 0x03E0) >>  5) << 3);
                    u8 b = (u8)((palette[buffer[src] * 2] & 0x001F) << 3);
                    tmp->pixels[dst] = RGB(r, g, b);
                }
            }
        }

        free(palette);
        free(buffer);
    } else if(header[2] == 2)
    {
        u32 size = (tmp->bpp / 8) * width * height;
        u8 *buffer = (u8*)malloc((tmp->bpp > 16 ? tmp->bpp / 8 : 3) * width * height);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        memcpy(buffer, data + sg, size);
        sg += size;

        if(tmp->bpp >= 24)
        {
            tmp->bpp /= 8;
            int c;
            for(c = 0; c < size; c += tmp->bpp)
                buffer[c] ^= buffer[c + 2] ^= buffer[c] ^= buffer[c + 2];
            tmp->bpp *= 8;
        }

        if(tmp->bpp == 8)
        {
            memcpy(tmp->pixels, buffer, size);
            u8 *buf2 = (u8*)tmp->pixels;
            int s, d;
            for(s = 0, d = 0; s < size; s++, d += 3)
            {
                buffer[d] = buf2[s];
                buffer[d + 1] = buf2[s];
                buffer[d + 2] = buf2[s];
            }
            tmp->bpp = 24;

            size = (tmp->bpp / 8) * width * height;
            memset(tmp->pixels, 0xFF, tmp->realH * tmp->realW);
        }
        else if(tmp->bpp == 16)
        {
            memcpy(tmp->pixels, buffer, size);
            u8 *buf2 = (u8*)tmp->pixels;
            int s, d;
            for(s = 0, d = 0; s < size; s += 2, d += 3) {
                u16 c = (buf2[s] + (buf2[s + 1] << 8));
                buffer[d] = (u8)(((c & 0x7C00) >> 10) << 3);
                buffer[d + 1] = (u8)(((c & 0x03E0) >> 5) << 3);
                buffer[d + 2] = (u8)((c & 0x001F) << 3);
            }
            tmp->bpp = 24;
            size = (tmp->bpp / 8) * width * height;
            memset(tmp->pixels, 0xFF, tmp->realW * tmp->realH);
        }

        tmp->bpp /= 8;
        int src = 0, dst = 0, j , i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width))
        {
            for(i = 0; i < width && src < size; i++, dst++, src += tmp->bpp)
            {
                if(tmp->bpp == 4) tmp->pixels[dst] = RGBA(buffer[src], buffer[src + 1], buffer[src + 2], buffer[src + 3]);
                if(tmp->bpp == 3) tmp->pixels[dst] = RGBA(buffer[src], buffer[src + 1], buffer[src + 2], 255);
            }
        }

        free(buffer);
    } else if(header[2] == 3)
    {
        u8 *buffer = malloc(width * height);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        memcpy(buffer, data + sg, (width * height));
        sg += (width * height);

        int src = 0, dst = 0, jmp = (tmp->bpp / 8), j, i;

        for(j = 0; j < height; j++, dst += (tmp->realW - width))
        {
            for(i = 0; i < width; i++, dst++, src += jmp) {
                if(tmp->bpp == 8) tmp->pixels[dst] = RGB(buffer[src], buffer[src], buffer[src]);
                if(tmp->bpp == 16) tmp->pixels[dst] = RGBA(buffer[src], buffer[src], buffer[src], buffer[src + 1]);
            }
        }

        free(buffer);
    } else if(header[2] == 9)
    {
        u32 size;
        u8 color, alpha = 0;
        u8 packet_header;

        tmp->bpp /= 8;

        u8 *buffer = (u8*)malloc(width * height);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        u8 *ptr = buffer;

        while(ptr < buffer + (width * height) * tmp->bpp)
        {
            memcpy(&packet_header, data + sg, 1);
            sg += 1;
            size = 1 + (packet_header & 0x7f);

            if(packet_header & 0x80) {
                memcpy(&color, data + sg, 1);
                sg += 1;
                if(tmp->bpp == 2)
                {
                    memcpy(&alpha, data + sg, 1);
                    sg += 1;
                }

                u32 i;
                for(i = 0; i < size; i++, ptr += tmp->bpp)
                {
                    ptr[0] = color;
                    if(tmp->bpp == 2) ptr[1] = alpha;
                }
            } else {
                u32 i;
                for(i = 0; i < size; i++, ptr += tmp->bpp)
                {
                    memcpy(&color, data + sg, 1);
                    sg += 1;
                    if(tmp->bpp == 2)
                    {
                        memcpy(&alpha, data + sg, 1);
                        sg += 1;
                    }
                    ptr[0] = color;
                    if(tmp->bpp == 2) ptr[1] = alpha;
                }
            }
        }

        int src = 0, dst = 0;
        u32 j, i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width))
            for(i = 0; i < width; i++, dst++, src += tmp->bpp)
                tmp->pixels[dst] = RGB(buffer[src], buffer[src], buffer[src]);

        free(buffer);
    } else if(header[2] == 10)
    {
        u32 size = 0;
        u8 rgb[4];
        u8 packet_header;
        tmp->bpp /= 8;

        int jmp = (tmp->bpp < 3 ? 3 : tmp->bpp);
        u16 color = 0x0;

        u8 *buffer = (u8*)malloc(width * height * jmp);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        u8 *ptr = buffer;

        while(ptr < buffer + (width * height) * jmp) {
            memcpy(&packet_header, data + sg, 1);
            sg += 1;
            size = 1 + (packet_header & 0x7f);

            if(packet_header & 0x80) {
                if(tmp->bpp >= 3) {
                    memcpy(rgb, data + sg, 1);
                    sg += 1;
                } else if(tmp->bpp == 2) {
                    char c, cc;
                    memcpy(&c, data + sg, 1);
                    sg += 1;
                    memcpy(&cc, data + sg, 1);
                    sg += 1;

                    color = (u16)((u8)c + (u16)(cc << 8));
                }

                u32 i;
                for(i = 0; i < size; ++i, ptr += jmp)
                {
                    if(tmp->bpp == 2) {
                        ptr[0] = (u8)(((color & 0x7c00) >> 10) << 3);
                        ptr[1] = (u8)(((color & 0x03E0) >>  5) << 3);
                        ptr[2] = (u8)(((color & 0x001F) >>  0) << 3);
                    }
                    if(tmp->bpp >= 3) {
                        ptr[0] = rgb[2];
                        ptr[1] = rgb[1];
                        ptr[2] = rgb[0];
                        if(tmp->bpp == 4) ptr[3] = rgb[3];
                    }
                }
            } else {
                u32 i;
                for(i = 0; i < size; ++i, ptr += jmp)
                {
                    if(tmp->bpp == 2) {
                        char c, cc;
                        memcpy(&c, data + sg, 1);
                        sg += 1;
                        memcpy(&cc, data + sg, 1);
                        sg += 1;
                        color = (u16)((u8)c + (u16)(cc << 8));
                        ptr[0] = (u8)(((color & 0x7C00) >> 10) << 3);
                        ptr[1] = (u8)(((color & 0x03E0) >>  5) << 3);
                        ptr[2] = (u8)(((color & 0x001F) >>  0) << 3);
                    }
                    if(tmp->bpp >= 3) {
                        memcpy(&ptr[2], data + sg, 1);
                        sg += 1;
                        memcpy(&ptr[1], data + sg, 1);
                        sg += 1;
                        memcpy(&ptr[0], data + sg, 1);
                        sg += 1;
                        if(tmp->bpp == 4)
                        {
                            memcpy(&ptr[3], data + sg, 1);
                            sg += 1;
                        }
                    }
                }
            }
        }

        int src = 0, dst = 0;
        u32 j, i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width)) {
            for(i = 0; i < width; i++, dst++, src += jmp) {
                if(tmp->bpp == 4) tmp->pixels[dst] = RGBA(buffer[src], buffer[src + 1], buffer[src + 2], buffer[src + 3]);
                if(tmp->bpp == 2 || tmp->bpp == 3) tmp->pixels[dst] = RGB(buffer[src], buffer[src + 1], buffer[src + 2]);
            }
        }

        free(buffer);
    } else if(header[2] == 1)
    {
        u32 size = 0;
        u8 color, alpha = 0;
        u8 packet_header;
        tmp->bpp /= 8;

        u8 *buffer = (u8*)malloc(width * height);
		if(!buffer) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			LPPG_FreeSurface(tmp);
			return null;
		}

        u8 *ptr = buffer;

        while(ptr < buffer + (width * height) * tmp->bpp)
        {
            memcpy(&packet_header, data + sg, 1);
            sg += 1;
            size = 1 + (packet_header & 0x7f);

            if(packet_header & 0x80) {
                memcpy(&color, data + sg, 1);
                sg += 1;
                if(tmp->bpp == 2)
                {
                    memcpy(&alpha, data + sg, 1);
                    sg += 1;
                }
                u32 i;
                for(i = 0; i< size; ++i, ptr += tmp->bpp)
                {
                    ptr[0] = color;
                    if(tmp->bpp == 2) ptr[1] = alpha;
                }
            } else {
                u32 i;
                for(i = 0; i < size; ++i, ptr += tmp->bpp)
                {
                    memcpy(&color, data + sg, 1);
                    sg += 1;
                    if(tmp->bpp == 2)
                    {
                        memcpy(&alpha, data + sg, 1);
                        sg += 1;
                    }
                    ptr[0] = color;
                    if(tmp->bpp == 2) ptr[1] = alpha;
                }
            }
        }

        int src = 0, dst = 0;
        u32 j, i;
        for(j = 0; j < height; j++, dst += (tmp->realW - width)) {
            for(i = 0; i < width; i++, dst++, src += tmp->bpp) {
                tmp->pixels[dst] = RGB(buffer[src], buffer[src], buffer[src]);
            }
        }

        free(buffer);
    }

    return (tmp);
}

LPP_Surface *LPPG_LoadImageTGA(L_CONST char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
	    return null;
	}

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    rewind(fp);
    u8 *data = (u8*)malloc(len);
	if(!data) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot alloc 'data' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fp);
		return null;
	}

    fread(data, len, 1, fp);
    LPP_Surface *ld = LPPG_LoadImageTGAImpl(data, len);
    free(data);
    fclose(fp);

    if(ld == null) return null;
    LPP_Surface *tmp = LPPG_FlipSurfaceVertical(ld);
    LPPG_FreeSurface(ld);
    return (tmp);
}

static void *bitmap_create(int width, int height)
{
	return calloc(width * height, 4);
}

static void bitmap_set_opaque(void *bitmap, bool opaque)
{
	(void) opaque;  /* unused */
	assert(bitmap);
}

static bool bitmap_test_opaque(void *bitmap)
{
	assert(bitmap);
	return false;
}

static u8 *bitmap_get_buffer(void *bitmap)
{
	assert(bitmap);
	return bitmap;
}

static void bitmap_destroy(void *bitmap)
{
	assert(bitmap);
	free(bitmap);
}

static void bitmap_modified(void *bitmap)
{
	assert(bitmap);
	return;
}

LPP_Surface *LPPG_LoadImageGIFImpl(u8 *data, u32 size, int fn)
{
    gif_bitmap_callback_vt bitmap_callbacks = {
        bitmap_create,
        bitmap_destroy,
        bitmap_get_buffer,
        bitmap_set_opaque,
        bitmap_test_opaque,
        bitmap_modified
    };

    gif_animation gif;
    gif_result code;

    gif_create(&gif, &bitmap_callbacks);

    code = gif_initialise(&gif, size, data);
    if(code != GIF_OK && code != GIF_WORKING) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot initialise gif.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

    u32 width = gif.width;
    u32 height = gif.height;

    LPP_Surface *tmp = LPPG_CreateSurface(width, height);
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
		return null;
	}

    int frame = (fn < 0) ? abs((gif.frame_count) >> 1) : fn;
    if(frame > gif.frame_count) frame = gif.frame_count;

    code = gif_decode_frame(&gif, frame);
    if(code != GIF_OK) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot decode the frame %d.\n", __FUNCTION__, __LINE__, frame);
		#endif
	    return null;
	}

    u8 *buffer = (u8*)gif.frame_image;

    int src = 0, dst = 0;
    u32 j, i;

    for(j = 0; j < height; j++, dst += (tmp->realW - width)) {
        for(i = 0; i < width; i++, dst++, src += tmp->bpp) {
            tmp->pixels[dst] = RGB(buffer[src], buffer[src + 1], buffer[src + 2]);
        }
    }

    gif_finalise(&gif);

    return (tmp);
}

LPP_Surface *LPPG_LoadImageGIF(L_CONST char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file %s for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
	    return null;
	}

    fseek(fp, 0, SEEK_END);
    u32 size = ftell(fp);
    rewind(fp);

    u8 *data = (u8*)malloc(size);
	if(!data) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'data' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		return null;
	}

    fread(data,size,1,fp);
    fclose(fp);

    LPP_Surface *tmp = LPPG_LoadImageGIFImpl(data, size, -1);

    free(data);

    return (tmp);
}

LPP_Surface *LPPG_LoadImageGIFframe(L_CONST char *filename, int frame)
{
    PSAR_ENTRY *entry = null;
    u8 *data = null;
    size_t size = 0;

    u8 isInPsar = LPPG_IsInPsar(filename);

    if(isInPsar)
    {
        entry = LPP_PsarDecoder_getEntry(filename);
		if(!entry) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot get '%s' from DATA.PSAR.\n", __FUNCTION__, __LINE__);
			#endif
			return null;
		}

        data = entry->data;
        size = entry->len;
    } else {
        FILE *fp = fopen(filename, "rb");
        if(fp == null) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot open '%s' for read.\n", __FUNCTION__, __LINE__, filename);
			#endif
		    return null;
		}

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        data = (u8*)malloc(size);
		if(!data) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocata 'data' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			return null;
		}
        fread(data,size,1,fp);
        fclose(fp);
    }

    LPP_Surface *tmp = LPPG_LoadImageGIFImpl(data, size, frame);

    if(isInPsar) {
        LPP_PsarDecoder_freeEntry(entry);
    } else free(data);

    return (tmp);
}

u32 LPPG_GifFramesCount(L_CONST char *filename)
{
    PSAR_ENTRY *entry = null;
    u8 *data = null;
    size_t size = 0;

    u8 isInPsar = LPPG_IsInPsar(filename);

    if(isInPsar)
    {
        entry = LPP_PsarDecoder_getEntry(filename);
		if(!entry) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot get '%s' from DATA.PSAR.\n", __FUNCTION__, __LINE__);
			#endif
			return null;
		}

        data = entry->data;
        size = entry->len;
    } else {
        FILE *fp = fopen(filename, "rb");
        if(fp == null) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot open '%s' for read.\n", __FUNCTION__, __LINE__, filename);
			#endif
		    return null;
		}

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        data = (u8*)malloc(size);
		if(!data) {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocata 'data' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			return null;
		}

        fread(data, size, 1, fp);
        fclose(fp);
    }

    gif_bitmap_callback_vt bitmap_callbacks = {
        bitmap_create,
        bitmap_destroy,
        bitmap_get_buffer,
        bitmap_set_opaque,
        bitmap_test_opaque,
        bitmap_modified
    };

    gif_animation gif;
    gif_result code;

    gif_create(&gif, &bitmap_callbacks);

    code = gif_initialise(&gif, size, data);
    if(code != GIF_OK && code != GIF_WORKING) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot initialise Gif.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

    u32 count = gif.frame_count;

    gif_finalise(&gif);

    if(isInPsar) {
        LPP_PsarDecoder_freeEntry(entry);
    }else {
        free(data);
    }

    return (count);
}

typedef struct {
    struct jpeg_source_mgr pub;
    L_CONST u8 *membuff;
    int location;
    int membufflenght;
    JOCTET *buffer;
    boolean start_of_buff;
} mem_source_mgr;

typedef mem_source_mgr* mem_src_ptr;

#define INPUT_BUF_SIZE (4096)

METHODDEF(void) mem_init_source(j_decompress_ptr cinfo)
{
    mem_src_ptr src = (mem_src_ptr)cinfo->src;
    src->location = 0;
    src->start_of_buff = TRUE;
}

METHODDEF(boolean) mem_fill_input_buffer(j_decompress_ptr cinfo)
{
    mem_src_ptr src;
    size_t bytes_to_read;
    size_t nbytes;

    src = (mem_src_ptr)cinfo->src;

    if((src->location) + INPUT_BUF_SIZE >= src->membufflenght)
        bytes_to_read = src->membufflenght - src->location;
    else
        bytes_to_read = INPUT_BUF_SIZE;

    memcpy(src->buffer, (src->membuff) + (src->location), bytes_to_read);
    nbytes = bytes_to_read;
	src->location += (int)bytes_to_read;

    if(nbytes <= 0) {
        if(src->start_of_buff)
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        WARNMS(cinfo, JWRN_JPEG_EOF);
        src->buffer[0] = (JOCTET)0xFF;
        src->buffer[1] = (JOCTET)JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_buff = FALSE;

    return TRUE;
}

METHODDEF(void) mem_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    mem_src_ptr src = (mem_src_ptr)cinfo->src;

    if(num_bytes > 0) {
        while(num_bytes > (long)src->pub.bytes_in_buffer) {
            num_bytes -= (long)src->pub.bytes_in_buffer;
            mem_fill_input_buffer(cinfo);
        }
        src->pub.next_input_byte += (size_t)num_bytes;
        src->pub.bytes_in_buffer -= (size_t)num_bytes;
    }
}

METHODDEF(void) mem_term_source(j_decompress_ptr cinfo)
{
    (void)cinfo;
}

GLOBAL(void)jpeg_mem_src(j_decompress_ptr cinfo, L_CONST u8 *mbuff, int mbufflen)
{
    mem_src_ptr src;

    if(cinfo->src == null) {
        cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(mem_source_mgr));
        src = (mem_src_ptr)cinfo->src;
        src->buffer = (JOCTET*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
    }

    src = (mem_src_ptr)cinfo->src;
    src->pub.init_source = mem_init_source;
    src->pub.fill_input_buffer = mem_fill_input_buffer;
    src->pub.skip_input_data = mem_skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = mem_term_source;
    src->membuff = mbuff;
    src->membufflenght = mbufflen;
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = null;
}

typedef struct {
    L_CONST u8 *data;
    png_size_t size;
    png_size_t seek;
} PngData;

static void ReadPngData(png_structp png_ptr, png_bytep data, png_size_t lenght)
{
    PngData *pngData = (PngData *)png_get_io_ptr(png_ptr);
    if(pngData) {
        png_size_t i;
        for(i = 0; i < lenght; i++) {
            if(pngData->seek >= pngData->size) break;
            data[i] = pngData->data[pngData->seek++];
        }
    }
}

LPP_Surface *LPPG_LoadImage(L_CONST char *filename) {
    LPP_Surface *tmp = null;

    if(LPPG_IsPNG(filename))
    {
        tmp = LPPG_LoadImagePNG(filename);
    } else
    if(LPPG_IsJPG(filename))
    {
        tmp = LPPG_LoadImageJPG(filename);
    } else
    if(LPPG_IsBMP(filename))
    {
        tmp = LPPG_LoadImageBMP(filename);
    } else
    if(LPPG_IsTGA(filename))
    {
        tmp = LPPG_LoadImageTGA(filename);
    } else
    if(LPPG_IsGIF(filename))
    {
        tmp = LPPG_LoadImageGIF(filename);
    }

    return (tmp);
}

LPP_Surface *LPPG_LoadImageFMem(u8* data, size_t len, L_CONST char *filename)
{
    if(len < 8) return null;
    LPP_Surface *tmp = null;

    if(data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
	    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	    PngData pngData;
		pngData.data = data;
		pngData.size = len;
		pngData.seek = 0x0;

		png_set_read_fn(png_ptr, (void*)&pngData, ReadPngData);
		tmp = LPPG_LoadImagePNGImpl(png_ptr);

       } else if(data[6] == 'J' && data[7] == 'F' && data[8] == 'I' && data[9] == 'F') {
           struct jpeg_decompress_struct dinfo;
		   struct jpeg_error_mgr jerr;
		   dinfo.err = jpeg_std_error(&jerr);
		   jpeg_create_decompress(&dinfo);
		   jpeg_mem_src(&dinfo, data, len);
		   tmp = LPPG_LoadImageJPGImpl(dinfo);
       } else if(data[0] == 'B' && data[1] == 'M') {
           u8 inv = GU_TRUE;
           tmp = LPPG_LoadImageBMPImpl(data, len, &inv);
           if(inv == GU_TRUE) {
                LPP_Surface *l = LPPG_FlipSurfaceVertical(tmp);
                LPPG_FreeSurface(tmp);
                return l;
           }
       } else if(data[0] == 'G' && data[1] == 'I' && data[2] == 'F')
       {
           tmp = LPPG_LoadImageGIFImpl(data, len, -1);
       } else
       {
           char fsig[32] = "";
           memcpy(fsig, (char*)data + len - 32, 32);
           if(strstr(fsig + 14, "TRUEVISION-XFILE"))
           {
               tmp = LPPG_LoadImageTGAImpl(data, len);
               LPP_Surface *l = LPPG_FlipSurfaceVertical(tmp);
               LPPG_FreeSurface(tmp);
               return l;
           }
       }

       return (tmp);
}

LPP_Surface *LPPG_LoadImageFPsar(L_CONST char *filename)
{
    PSAR_ENTRY *f = LPP_PsarDecoder_getEntry(filename);
    if(f == null)
    {
        #ifdef DEBUG
		dwrite_output("Function %s Line %d : cannot get the file '%s' from DATA.PSAR.\n", __FUNCTION__, __LINE__, filename);
		#endif
        return null;
    }

    LPP_Surface *t = LPPG_LoadImageFMem(f->data, f->len, filename);
    LPP_PsarDecoder_freeEntry(f);
    return t;
}

LPP_Surface *LPPG_CreateSurface(u32 Width, u32 Height)
{
    LPP_Surface *tmp = (LPP_Surface *)malloc(LPP_Surface_size);

    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

    tmp->width = Width;
    tmp->height = Height;

    tmp->realW = NextPow2(Width);
    tmp->realH = NextPow2(Height);

    tmp->pixels = (u32 *)memalign(16, tmp->realH * tmp->realW * sizeof(u32));
	if(!tmp->pixels) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'pixels' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp);
		return null;
	}

    tmp->swizzled = 0;
    tmp->bpp = 4;

    memset(tmp->pixels, 0, tmp->realW * tmp->realH * sizeof(u32));

    return (tmp);
}

void LPPG_FreeSurface(LPP_Surface *s) {
    if(s->pixels)
    {
        if(s->location == LPP_RAM)
            free(s->pixels);
        else
            vfree(s->pixels);
    }
    if(s) free(s);
}

u8 gu_start = 0;

void LPPG_StartDrawing(void) {
    if(!gu_start) {
        sceGuStart(GU_DIRECT, dList[dBuffer_n]);
        dBuffer_n ^= 1;
        gu_start = 1;
    }
}

void LPPG_EndDrawing(void)
{
    if(gu_start) {
        sceGuFinish();
        sceGuSync(0, 0);
        gu_start = 0;
    }
}

void LPPG_FlipScreen(void) {
    if(!Initialized) return;
    FrameBuffer = vabsptr(sceGuSwapBuffers());
}

void LPPG_ClearSurface(LPP_Surface *s, Color color) {
    int i, size = s->realH * s->realW;
    u32 *data = s->pixels;
    for(i = 0; i < size; i++, data++) *data = color;
}

void LPPG_ClearScreen(Color color) {
    if(!Initialized) return;
    LPPG_StartDrawing();
    sceGuClearColor(color);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT|GU_FAST_CLEAR_BIT);
    LPPG_EndDrawing();
}

LPP_Surface* LPPG_FlipSurfaceVertical(LPP_Surface *s)
{
    u32 width = s->width;
    u32 height = s->height;
    LPP_Surface *tmp = LPPG_CreateSurface(width, height);

    u32 x, y;
    for(x = 0; x < width; x++)
    {
        for(y = 0; y < height; y++)
        {
            Color clr = LPPG_GetSurfacePixel(x, y, s);
            LPPG_PutSurfacePixel(x, height - y - 1, clr, tmp);
        }
    }

    return (tmp);
}

LPP_Surface *LPPG_CopySurface(LPP_Surface *s)
{
    LPP_Surface *tmp = LPPG_CreateSurface(s->width, s->height);
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
		return null;
	}

    u32 *sv = tmp->pixels;
    memcpy(tmp, s, sizeof(LPP_Surface));

    tmp->pixels = sv;
    memcpy(tmp->pixels, s->pixels, sizeof(u32) * s->realW * s->realH);

    return (tmp);
}

u8 LPPG_SurfaceEquals(LPP_Surface *s1, LPP_Surface *s2)
{
    if(s1->width != s2->width) return 0;
    if(s1->height != s2->height) return 0;

    u32 i, j;
    for(j = 0; j < s1->height; j++) {
        for(i = 0; i < s1->width; i++) {
            if(LPPG_GetSurfacePixel(i, j, s1) != LPPG_GetSurfacePixel(i, j, s2)) return 0;
        }
    }

    return 1;
}

void LPPG_SwizzleSurface(LPP_Surface *s)
{
    if(s->swizzled) return;

    size_t size = (s->realH * s->realW * s->bpp);

    u8 *tmp = (s->location == LPP_RAM) ? (u8 *)malloc(size) : (u8*)valloc(size);
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    return;
	}

    u32 width = s->realW * s->bpp;
    u32 height = s->realH;

    u32 blockx, blocky, j;

    u32 width_blocks = width / 16;
    u32 height_blocks = height / 8;

    u32 src_pitch = (width - 16) / 4;
    u32 src_row = width * 8;

    L_CONST u8 *ysrc = (u8*)s->pixels;
    u32 *dst = (u32*)tmp;

    for(blocky = 0; blocky < height_blocks; ++blocky)
    {
        L_CONST u8 *xsrc = ysrc;
        for(blockx = 0; blockx < width_blocks; ++blockx)
        {
            L_CONST u32 *src = (u32*)xsrc;
            for(j = 0; j < 8; ++j) {
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                src += src_pitch;
            }
            xsrc += 16;
        }
        ysrc += src_row;
    }

    if(s->location == LPP_RAM) free(s->pixels); else vfree(s->pixels);

    s->pixels = (u32*)tmp;
    s->swizzled = 1;
}

void LPPG_UnSwizzleSurface(LPP_Surface *s) {
    if(!s->swizzled) return;

    size_t size = s->realW * s->realH * s->bpp;

    u8* tmp = (s->location == LPP_RAM) ? (u8 *)malloc(size) : (u8*)valloc(size);
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    return;
	}

    u32 width = s->realW * s->bpp;
    u32 height = s->realH;

    u32 blockx, blocky, j;
    u32 width_blocks = width / 16;
    u32 height_blocks = height / 8;

    u32 dst_pitch = (width - 16) / 4;
    u32 dst_row = width * 8;

    u32 *src = (u32*)s->pixels;
    u8 *ydst = tmp;

    for(blocky = 0; blocky < height_blocks; ++blocky) {
        L_CONST u8 *xdst = ydst;
        for(blockx = 0; blockx < width_blocks; ++blockx) {
            u32 *dst = (u32*)xdst;
            for(j = 0; j < 8; ++j) {
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                dst += dst_pitch;
            }
            xdst += 16;
        }
        ydst += dst_row;
    }

    if(s->location == LPP_RAM) free(s->pixels); else vfree(s->pixels);
    s->pixels = (u32*)tmp;
    s->swizzled = 0;
}

void LPPG_SurfaceToVram(LPP_Surface *s)
{
    if(s->location == LPP_VRAM) return;
    size_t size = s->realH * s->realW * 4;
    u32 *tmp = (u32*)valloc(size);
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    return;
	}

    memcpy(tmp, s->pixels, size);
    free(s->pixels);
    s->location = LPP_VRAM;
    s->pixels = tmp;
    sceKernelDcacheWritebackAll();
}

void LPPG_SurfaceToRam(LPP_Surface *s)
{
    if(s->location == LPP_RAM) return;
    size_t size = s->realH * s->realW * 4;
    u32 *tmp = (u32*)malloc(size);
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
	    return;
	}

    memcpy(tmp, s->pixels, size);
    vfree(s->pixels);
    s->pixels = tmp;
    s->location = LPP_RAM;
    sceKernelDcacheWritebackAll();
}

LPP_Surface *LPPG_SurfaceResized(LPP_Surface *s, u32 new_width, u32 new_height)
{
    LPP_Surface *tmp = LPPG_CreateSurface(new_width, new_height);
    if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

	/* To Implement */

    return (tmp);
}

LPP_Surface *LPPG_SurfaceNegative(LPP_Surface *s)
{
    LPP_Surface *tmp = LPPG_CreateSurface(s->width, s->height);
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

    u32 i, k;
    for(i = 0; i < s->width; i++){
        for(k = 0; k < s->height; k++){
            Color c = LPPG_GetSurfacePixel(i, k, s);
            u8 r = 255 - R(c);
            u8 g = 255 - G(c);
            u8 b = 255 - B(c);
            u8 a = 255 - A(c);
            LPPG_PutSurfacePixel(i, k, RGBA(r, g, b, a), tmp);
        }
    }

    return (tmp);
}

LPP_Surface *LPPG_ScreenNegative(void)
{
    LPP_Surface *tmp = LPPG_CreateSurface(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'tmp'.\n", __FUNCTION__, __LINE__);
		#endif
	    return null;
	}

    u32 i, k;
    for(i = 0; i < PSP_SCREEN_WIDTH; i++) {
        for(k = 0; k < PSP_SCREEN_HEIGHT; k++) {
            Color c = LPPG_GetScreenPixel(i, k);
            u8 r = 255 - R(c);
            u8 g = 255 - G(c);
            u8 b = 255 - B(c);
            u8 a = 255 - A(c);
            LPPG_PutSurfacePixel(i, k, RGBA(r, g, b, a), tmp);
        }
    }

    return (tmp);
}

static int fixed_write(SceUID fd, void *data, int len)
{
	int writelen = 0;

	while(writelen < len)
	{
		int ret;

		ret = sceIoWrite(fd, data + writelen, len - writelen);
		if(ret <= 0)
		{
			writelen = -1;
			break;
		}
		writelen += ret;
	}

	return writelen;
}

int write_8888_data(void *frame, void *pixel_data, u32 width, u32 height, u32 lineSize)
{
	uint8_t *line;
	uint8_t *p;
	int i;
	int h;

	line = pixel_data;
	for(h = (height - 1); h >= 0; h--)
	{
		p = frame + (h*lineSize*4);
		for(i = 0; i < width; i++)
		{
			line[(i*3) + 2] = p[i*4];
			line[(i*3) + 1] = p[(i*4) + 1];
			line[(i*3) + 0] = p[(i*4) + 2];
		}
		line += width * 3;
	}

	return 0;
}

int write_5551_data(void *frame, void *pixel_data, u32 height, u32 width, u32 lineSize)
{
	uint8_t *line;
	uint16_t *p;
	int i;
	int h;

	line = pixel_data;
	for(h = (height - 1); h >= 0; h--)
	{
		p = frame;
		p += (h * lineSize);
		for(i = 0; i < width; i++)
		{
			line[(i*3) + 2] = (p[i] & 0x1F) << 3;
			line[(i*3) + 1] = ((p[i] >> 5) & 0x1F) << 3;
			line[(i*3) + 0] = ((p[i] >> 10) & 0x1F) << 3;
		}
		line += width*3;
	}

	return 0;
}

int write_565_data(void *frame, void *pixel_data, u32 width, u32 height, u32 lineSize)
{
	uint8_t *line;
	uint16_t *p;
	int i;
	int h;

	line = pixel_data;
	for(h = (height - 1); h >= 0; h--)
	{
		p = frame;
		p += (h * lineSize);
		for(i = 0; i < width; i++)
		{
			line[(i*3) + 2] = (p[i] & 0x1F) << 3;
			line[(i*3) + 1] = ((p[i] >> 5) & 0x3F) << 2;
			line[(i*3) + 0] = ((p[i] >> 11) & 0x1F) << 3;
		}
		line += width*3;
	}

	return 0;
}

int write_4444_data(void *frame, void *pixel_data, u32 width, u32 height, u32 lineSize)
{
	uint8_t *line;
	uint16_t *p;
	int i;
	int h;

	line = pixel_data;
	for(h = (height - 1); h >= 0; h--)
	{
		p = frame;
		p += (h * lineSize);
		for(i = 0; i < width; i++)
		{
			line[(i*3) + 2] = (p[i] & 0xF) << 4;
			line[(i*3) + 1] = ((p[i] >> 4) & 0xF) << 4;
			line[(i*3) + 0] = ((p[i] >> 8) & 0xF) << 4;
		}
		line += width*3;
	}

	return 0;
}

int bitmapWrite(void *frame_addr, void *tmp_buf, int format, const char *file, u32 width, u32 height, u32 lineSize)
{
	struct BitmapHeader *bmp;
	void *pixel_data = tmp_buf + sizeof(struct BitmapHeader);
	int fd;

	bmp = (struct BitmapHeader *) tmp_buf;
	memset(bmp, 0, sizeof(struct BitmapHeader));
	memcpy(bmp->id, BMP_ID, sizeof(bmp->id));
	bmp->filesize = width * height * 4 + sizeof(struct BitmapHeader);
	bmp->offset = sizeof(struct BitmapHeader);
	bmp->headsize = 0x28;
	bmp->width = width;
	bmp->height = height;
	bmp->planes = 1;
	bmp->bpp = 24;
	bmp->bitmapsize = width * height * 3;
	bmp->hres = 2834;
	bmp->vres = 2834;

	switch(format)
	{
		case PSP_DISPLAY_PIXEL_FORMAT_565: write_565_data(frame_addr, pixel_data, width, height, lineSize);
										   break;
		case PSP_DISPLAY_PIXEL_FORMAT_5551: write_5551_data(frame_addr, pixel_data, width, height, lineSize);
										   break;
		case PSP_DISPLAY_PIXEL_FORMAT_4444: write_4444_data(frame_addr, pixel_data, width, height, lineSize);
											break;
		case PSP_DISPLAY_PIXEL_FORMAT_8888: write_8888_data(frame_addr, pixel_data, width, height, lineSize);
											break;
	};

	if((fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777)) >= 0) {
		fixed_write(fd, tmp_buf, bmp->filesize);
		sceIoClose(fd);
		return 0;
	}

	return -1;
}

void savePng(L_CONST char *filename, u32 *pixels, u32 width, u32 height, u32 linesize, u8 saveAlpha)
{
    png_structp png_ptr = null;
    png_infop info_ptr = null;
    FILE *fp = null;
    u8 *line = null;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, null, null, null);
    if(!png_ptr) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'png_ptr'.\n", __FUNCTION__, __LINE__);
		#endif
	    return;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == null)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create 'info_ptr'.\n", __FUNCTION__, __LINE__);
		#endif
        png_destroy_write_struct(&png_ptr, null);
        return;
    }

    u32 col_type = saveAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;

    fp = fopen(filename, "wb");
    if(!fp)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' for write.\n", __FUNCTION__, __LINE__, filename);
		#endif
        png_destroy_write_struct(&png_ptr, null);
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, col_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    line = (u8*)malloc(width * 4);
    if(line == null)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'line' to memory.\n", __FUNCTION__, __LINE__);
		#endif
        png_destroy_write_struct(&png_ptr, null);
        fclose(fp);
        return;
    }

    int x, y, i;

    for(y = 0; y < height; y++)
    {
        for(i = 0, x = 0; x < width; x++)
        {
            u32 col32 = pixels[x + y * linesize];

            line[i++] = R(col32);
            line[i++] = G(col32);
            line[i++] = B(col32);
            if(saveAlpha) line[i++] = A(col32);
        }

        png_write_row(png_ptr, line);
    }

    free(line);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, null);
    fclose(fp);
}

void saveJpg(L_CONST char *filename, u32 *pixels, u32 width, u32 height, u32 lineSize)
{
    FILE *fp = null;
    if((fp = fopen(filename, "wb")) == null) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' for write.\n", __FUNCTION__, __LINE__, filename);
		#endif
	    return;
	}

    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    u8 *row = (u8*)malloc(width * 3);
    if(!row)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'row' to memory.\n", __FUNCTION__, __LINE__);
		#endif

        fclose(fp);
        jpeg_destroy_compress(&cinfo);
        return;
    }

    u32 y;
    for(y = 0; y < height; y++)
    {
        u8 *row_ptr = row;
        u32 x;
        for(x = 0; x < width; x++) {
            u32 c = pixels[x + cinfo.next_scanline * lineSize];
            *(row_ptr++) = c & 0xff;
            *(row_ptr++) = (c >> 8) & 0xff;
            *(row_ptr++) = (c >> 16) & 0xff;
        }
        jpeg_write_scanlines(&cinfo, &row, 1);
    }

    free(row);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
}

void saveBmp(L_CONST char *filename, u32 *pixels, u32 width, u32 height, u32 lineSize)
{
    u8 *tmp = (u8*)malloc(width * height * 3 + sizeof(struct BitmapHeader));
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		return;
	}

    bitmapWrite(pixels, tmp, PSP_DISPLAY_PIXEL_FORMAT_8888, filename, width, height, lineSize);
    free(tmp);
}

void LPPG_SaveSurface(L_CONST char *filename, u32 format, LPP_Surface *s)
{
    u8 isSwizzled = s->swizzled;
    if(isSwizzled) LPPG_UnSwizzleSurface(s);
    if(format == LPP_Image_PNG)
        savePng(filename, s->pixels, s->width, s->height, s->realW, GU_TRUE);
    else if(format == LPP_Image_JPG)
        saveJpg(filename, s->pixels, s->width, s->height, s->realW);
    else if(format == LPP_Image_BMP)
        saveBmp(filename, s->pixels, s->width, s->height, s->realW);
    if(isSwizzled) LPPG_SwizzleSurface(s);
}

void LPPG_SaveScreen(L_CONST char *filename, u32 format)
{
    sceDisplayWaitVblankStart();
    if(format == LPP_Image_PNG)
        savePng(filename, LPPG_GetVDisplayBuffer(), PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, PSP_LINE_SIZE, GU_FALSE);
    else if(format == LPP_Image_JPG)
        saveJpg(filename, LPPG_GetVDisplayBuffer(), PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, PSP_LINE_SIZE);
    else if(format == LPP_Image_BMP)
        saveBmp(filename, LPPG_GetVDisplayBuffer(), PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, PSP_LINE_SIZE);
}

void LPPG_FillSurfaceRect(int x0, int y0, int width, int height, Color color, LPP_Surface *dest) {
    int skipX = dest->realW - width, x, y;
    u32 *data = dest->pixels + x0 + y0 * dest->realW;
    for(y = 0; y < height; y++, data += skipX)
        for(x = 0; x < width; x++, data++) *data = color;
}

void LPPG_ScreenRect(int x, int y, int width, int height, Color color)
{
    if(!Initialized) return;

    LPPG_StartDrawing();

    LPP_VertCV *vertices = (LPP_VertCV*)sceGuGetMemory((sizeof(LPP_VertCV)) << 4);

    int i;

    if(width < 0) {
        i = x;
        x = width;
        width = i;
    }

    if(height < 0) {
        i = y;
        y = height;
        height = i;
    }

    for(i = 0; i < 8; i++) {
        vertices[i].color = color;
        vertices[i].z = 0;
    }

    vertices[0].x = x;
    vertices[0].y = y + 1;

    vertices[1].x = x;
    vertices[1].y = y + height;

    vertices[2].x = x + 1;
    vertices[2].y = y + height - 1;

    vertices[3].x = x + width;
    vertices[3].y = y + height - 1;

    vertices[4].x = x + width - 1;
    vertices[4].y = y;

    vertices[5].x = x + width -1;
    vertices[5].y = y + height - 1;

    vertices[6].x = x;
    vertices[6].y = y;

    vertices[7].x = x + width - 1;
    vertices[7].y = y = y;

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_LINES, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 8, 0, vertices);
    sceKernelDcacheInvalidateRange(vertices, sizeof(LPP_VertCV) << 4);
    sceGuEnable(GU_TEXTURE_2D);

    LPPG_EndDrawing();
}

void LPPG_SurfaceRect(int x, int y, int width, int height, Color color, LPP_Surface *s)
{
    LPPG_SurfaceLine(x, y, x + width, y, color, s);
    LPPG_SurfaceLine(x + width, y, x + width, y + height, color, s);
    LPPG_SurfaceLine(x + width, y + height, x, y + height, color, s);
    LPPG_SurfaceLine(x, y + height, x, y, color, s);
}

void LPPG_FillScreenRect(int x, int y, int width, int height, Color color)
{
    if(!Initialized) return;

    LPPG_StartDrawing();

    LPP_VertCV *vertices = (LPP_VertCV*)sceGuGetMemory((sizeof(LPP_VertCV)) << 1);

    vertices[0].color = color;
    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = 0;

    vertices[1].color = color;
    vertices[1].x = x + width;
    vertices[1].y = y + height;
    vertices[1].z = 0;

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
    sceKernelDcacheInvalidateRange(vertices, sizeof(LPP_VertV) * 2);
    sceGuEnable(GU_TEXTURE_2D);

    LPPG_EndDrawing();
}

void LPPG_ScreenLine(int x1, int y1, int x2, int y2, Color color)
{
    LPPG_StartDrawing();

    LPP_VertCV *vertices = (LPP_VertCV*)sceGuGetMemory((sizeof(LPP_VertCV)) << 1);

    vertices[0].color = vertices[1].color = color;
    vertices[0].z = vertices[1].z = 0;

    vertices[0].x = x1;
    vertices[0].y = y1;
    vertices[1].x = x2;
    vertices[1].y = y2;

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_LINES, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
    sceKernelDcacheInvalidateRange(vertices, sizeof(LPP_VertCV) << 1);
    sceGuEnable(GU_TEXTURE_2D);

    LPPG_EndDrawing();
}

void LPPG_ScreenCircle(int centerX, int centerY, int radius, Color color)
{
    int x, y;
    radius++;
    for(x = -1 * radius; x <= radius; x++)
    {
        y = sqrt2((radius * radius) - (x * x));
        LPPG_ScreenLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color);
        y = sqrt2((radius * radius) - (x * x)) * -1;
        LPPG_ScreenLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color);
    }
    for(y = - 1 * radius; x <= radius; y++)
    {
        x = sqrt2((radius * radius) - (y * y));
        LPPG_ScreenLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color);
        x = sqrt2((radius * radius) - (y * y)) * -1;
        LPPG_ScreenLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color);
    }
}

void LPPG_SurfaceCircle(int centerX, int centerY, int radius, Color color, LPP_Surface *s)
{
    int x, y;
    radius++;
    for(x = -1 * radius; x <= radius; x++)
    {
        y = sqrt2((radius * radius) - (x * x));
        LPPG_SurfaceLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color, s);
        y = sqrt2((radius * radius) - (x * x)) * -1;
        LPPG_SurfaceLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color, s);
    }
    for(y = - 1 * radius; x <= radius; y++)
    {
        x = sqrt2((radius * radius) - (y * y));
        LPPG_SurfaceLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color, s);
        x = sqrt2((radius * radius) - (y * y)) * -1;
        LPPG_SurfaceLine(x + centerX, y + centerY, x + centerX + 1, y + centerY + 1, color, s);
    }
}

void LPPG_FillScreenCircle(int centerX, int centerY, int radius, Color color)
{
    int x, y;
    for(x = -1 * radius; x <= radius; x++)
    {
        y = sqrt2((radius * radius) - (x * x));
        LPPG_ScreenLine(x + centerX, y + centerY, centerX, y + centerY, color);
        y = sqrt2((radius * radius) - (x * x)) * -1;
        LPPG_ScreenLine(x + centerX, y + centerY, centerX, y + centerY, color);
    }
    for(y = -1 * radius; y <= radius; y++)
    {
        x = sqrt2((radius * radius) - (y * y));
        LPPG_ScreenLine(x + centerX, y + centerY, centerX, y + centerY, color);
        x = sqrt2((radius * radius) - (y * y)) * -1;
        LPPG_ScreenLine(x + centerX, y + centerY, centerX, y + centerY, color);
    }
}

void LPPG_FillSurfaceCircle(int centerX, int centerY, int radius, Color color, LPP_Surface *s)
{
    int x, y;
    for(x = -1 * radius; x <= radius; x++)
    {
        y = sqrt2((radius * radius) - (x * x));
        LPPG_SurfaceLine(x + centerX, y + centerY, centerX, y + centerY, color, s);
        y = sqrt2((radius * radius) - (x * x)) * -1;
        LPPG_SurfaceLine(x + centerX, y + centerY, centerX, y + centerY, color, s);
    }
    for(y = -1 * radius; y <= radius; y++)
    {
        x = sqrt2((radius * radius) - (y * y));
        LPPG_SurfaceLine(x + centerX, y + centerY, centerX, y + centerY, color, s);
        x = sqrt2((radius * radius) - (y * y)) * -1;
        LPPG_SurfaceLine(x + centerX, y + centerY, centerX, y + centerY, color, s);
    }
}

void LPPG_SurfaceLine(int x1, int y1, int x2, int y2, Color color, LPP_Surface *s)
{
    /* Bresenham's Algorithm */
    int delta_x = x2 - x1;
    signed char ix = (delta_x > 0) - (delta_x < 0);
    delta_x = abs(delta_x) << 1;

    int delta_y = y2 - y1;
    signed char iy = (delta_y > 0) - (delta_y < 0);
    delta_y = abs(delta_y) << 1;

    LPPG_PutSurfacePixel(x1, y1, color, s);

    if(delta_x >= delta_y)
    {
        int error = delta_y - (delta_x >> 1);

        while(x1 != x2) {
            if(error >= 0) {
                if(error || (ix > 0))
                {
                    y1 += iy;
                    error -= delta_x;
                }
            }

            x1 += ix;
            error += delta_y;

            LPPG_PutSurfacePixel(x1, y1, color, s);
        }
    }
    else
    {
        int error = delta_x - (delta_y >> 1);

        while(y1 != y2)
        {
            if(error >= 0)
            {
                if(error || (iy > 0))
                {
                    x1 += ix;
                    error -= delta_y;
                }
            }

            y1 += iy;
            error += delta_x;

            LPPG_PutSurfacePixel(x1, y1, color, s);
        }
    }
}

void LPPG_PutSurfacePixel(int x, int y, Color color, LPP_Surface *dest) {
    dest->pixels[x + y * dest->realW] = color;
}

void LPPG_PutScreenPixel(int x, int y, Color color) {
    u32 *vram = LPPG_GetVDrawBuffer();
    vram[PSP_LINE_SIZE * y + x] = color;
}

Color LPPG_GetSurfacePixel(int x, int y, LPP_Surface *dest) {
    return dest->pixels[x + y * dest->realW];
}

Color LPPG_GetScreenPixel(int x, int y) {
    u32 *vram = LPPG_GetVDrawBuffer();
    return vram[PSP_LINE_SIZE * y + x];
}

void LPPG_BlitSurfaceScreen(float x, float y, LPP_Surface *source, u8 alpha, float angle, float sx, float sy, float w, float h) {
    if(!Initialized) return;

    LPPG_StartDrawing();

	if(angle == 0.0f) {
	    sceGuColor((alpha << 24) | 0xFFFFFF);
        sceGuTexMode(GU_PSM_8888, 0, 0, source->swizzled);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuTexImage(0, source->realW, source->realH, source->realW, (void*)source->pixels);
        float u = 1.0f / ((float)source->realW);
        float v = 1.0f / ((float)source->realH);
        sceGuTexScale(u, v);

        int j = 0;

        while(j < w)
        {
            LPP_VertTV *vertices = (LPP_VertTV*)sceGuGetMemory((sizeof(LPP_VertTV)) << 1);

            int sliceW = 64;
            if(j + sliceW > w) sliceW = w - j;

            vertices[0].u = sx + j;
            vertices[0].v = sy;
            vertices[0].x = x + j;
			vertices[0].y = y;
			vertices[0].z = 0;

            vertices[1].u = sx + j + sliceW;
            vertices[1].v = sy + h;
            vertices[1].x = x + j + sliceW;
            vertices[1].y = y + h;
            vertices[1].z = 0;

            sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
            j += sliceW;
            sceKernelDcacheInvalidateRange(vertices, (sizeof(LPP_VertTV)) << 1);
        }
        sceGuTexMode(GU_PSM_8888, 0, 0, GU_TRUE);
	}
	else
	{
	    sceGuColor((alpha << 24) | 0xFFFFFF);
        sceGuTexMode(GU_PSM_8888, 0, 0, source->swizzled);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuTexImage(0, source->realW, source->realH, source->realW, (void*)source->pixels);
        float u = 1.0f / ((float)source->realW);
        float v = 1.0f / ((float)source->realH);
        sceGuTexScale(u, v);

		LPP_VertTV *vertices = null;
		float cX, cY, tmpY;
		float uVal, xVal;
		float uCoeff, xCoeff;
		float angleRadians;

		xVal = 0.0f;
		uVal = sx;
		angleRadians = (angle * GU_PI) / 180.0f;

		uCoeff = (w >= sx) ? 64 : -64;

		xCoeff = uCoeff / ((float)abs(w - sx) / (float)source->width);
		cX = -(source->width >> 1) / (int)(w - sx);
		cY = -(source->height >> 1) / (int)(h - sy);

		tmpY = cY + source->height;
		xVal = cX;

		while(uVal != w) {
		    vertices = (LPP_VertTV*)sceGuGetMemory(4 * sizeof(LPP_VertTV));

			vertices[0].u = uVal;
			vertices[0].v = sy;
			vertices[0].x = LPP_cosf(angleRadians, xVal) - LPP_sinf(angleRadians, cY) + x;
			vertices[0].y = LPP_sinf(angleRadians, xVal) + LPP_cosf(angleRadians, cY) + y;
			vertices[0].z = 0;

			vertices[2].u = uVal;
			vertices[2].v = h;
			vertices[2].x = LPP_cosf(angleRadians, xVal) - LPP_sinf(angleRadians, tmpY) + x;
			vertices[2].y = LPP_sinf(angleRadians, xVal) + LPP_cosf(angleRadians, tmpY) + y;
			vertices[2].z = 0;

			uVal += uCoeff;
			xVal = xCoeff;

			if(uVal > w) {
			    xVal = cX + source->width;
				uVal = w;
			}

			vertices[1].u = uVal;
			vertices[1].v = sy;
			vertices[1].x = LPP_cosf(angleRadians, xVal) - LPP_sinf(angleRadians, cY) + x;
			vertices[1].y = LPP_sinf(angleRadians, xVal) - LPP_cosf(angleRadians, cY) + y;
			vertices[1].z = 0;

			vertices[3].u = uVal;
			vertices[3].v = h;
			vertices[3].x = LPP_cosf(angleRadians, xVal) - LPP_sinf(angleRadians, tmpY) + x;
			vertices[3].y = LPP_sinf(angleRadians, xVal) + LPP_cosf(angleRadians, tmpY) + y;

			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 4, 0, vertices);
			sceKernelDcacheInvalidateRange(vertices, (sizeof(LPP_VertTV)) * 4);
		}
		sceGuTexMode(GU_PSM_8888, 0, 0, GU_TRUE);
	}

    LPPG_EndDrawing();
}

void LPPG_BlitSurfaceSurface(int x, int y, LPP_Surface *source, int sx,int sy, int w,int h, LPP_Surface *dest) {
    u32 *dstData = &dest->pixels[dest->realW * y + x];
    int dstSkipX = dest->realW - w;
    u32 *srcData = &source->pixels[source->realW * y + x];
    int srcSkipX = source->realW - w;
    int X, Y;
    s32 rcolorc, gcolorc, bcolorc, acolorc,rcolord, gcolord, bcolord, acolord;

    for(Y = 0; Y < h; Y++, dstData += dstSkipX, srcData += srcSkipX) {
        for(X = 0; X < w; X++, dstData++, srcData++) {
            u32 color = *srcData;
            if(!IS_ALPHA(color)) *dstData = color;
            else {
                rcolorc = color & 0xff;
                gcolorc = (color >> 8) & 0xff;
                bcolorc = (color >> 16) & 0xff;
                acolorc = (color >> 24) & 0xff;
                rcolord = *dstData & 0xff;
                gcolord = (*dstData >> 8) & 0xff;
                bcolord = (*dstData >> 16) & 0xff;
                acolord = (*dstData >> 24) & 0xff;

                rcolorc = ((acolorc*rcolorc)>>8) + (((255-acolorc) * rcolord)>>8);
                if (rcolorc > 255) rcolorc = 255;
                gcolorc = ((acolorc*gcolorc)>>8) + (((255-acolorc) * gcolord)>>8);
                if (gcolorc > 255) gcolorc = 255;
                bcolorc = ((acolorc*bcolorc)>>8) + (((255-acolorc) * bcolord)>>8);
                if (bcolorc > 255) bcolorc = 255;
                if (acolord + acolorc < 255) {
                    acolorc = acolord+acolorc;
                } else {
                    acolorc = 255;
                }
                *dstData = rcolorc | (gcolorc << 8) | (gcolorc << 16) | (acolorc << 24);
            }
        }
    }
}

void LPPG_DrawGradientRect(int x, int y, int width, int height, Color c1, Color c2, Color c3, Color c4)
{
    if(!Initialized) return;

    LPPG_StartDrawing();

    LPP_VertCV *vertices = (LPP_VertCV*)sceGuGetMemory(4 * sizeof(LPP_VertCV));

    vertices[0].color = c1;
    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = 0;

    vertices[1].color = c2;
    vertices[1].x = x + width;
    vertices[1].y = y;
    vertices[1].z = 0;

    vertices[2].color = c3;
    vertices[2].x = x;
    vertices[2].y = y + height;
    vertices[2].z = 0;

    vertices[3].color = c4;
    vertices[3].x = x + width;
    vertices[3].y = y + height;
    vertices[3].z = 0;

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_TRIANGLE_STRIP, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 4, 0, vertices);
    sceKernelDcacheInvalidateRange(vertices, 4 * sizeof(LPP_VertCV));
    sceGuEnable(GU_TEXTURE_2D);

    LPPG_EndDrawing();
}

void LPPG_PrintTextScreen(int x, int y, L_CONST char *text, u32 color) {
    if(!Initialized) return;
    int i, j, l;
    u8 *font = null;
    u32 *vram_ptr, *vram;

    size_t c, len = strlen(text);
    for(c = 0; c < len; c++) {
        if(x < 0 || x + 8 > PSP_SCREEN_WIDTH || y < 0 || y + 8 > PSP_SCREEN_HEIGHT ) break;
        char ch = text[c];
        vram = LPPG_GetVDrawBuffer() + x + y * PSP_LINE_SIZE;

        font = &msx[(int)ch * 8];
        for(i = l = 0; i < 8; i++, l += 8, font++) {
            vram_ptr = vram;
	    for(j = 0; j < 8; j++) {
	      if((*font & (128 >> j))) *vram_ptr = color;
	      vram_ptr++;
	    }
	    vram += PSP_LINE_SIZE;
        }
        x += 8;
    }
}

void LPPG_PrintTextSurface(int x, int y, L_CONST char *text, u32 color, LPP_Surface *dest) {
  if(!Initialized) return;
  int i, j, l;
  u8 *font = null;
  u32 *data_ptr, *data;
  size_t c, len = strlen(text);
  for(c = 0; c < len; c++) {
    if(x < 0 || x + 8 > dest->realW || y < 0 || y + 8 > dest->realH) {
	    break;
	}

    char ch = text[c];
    data = dest->pixels + x + y * dest->realW;

    font = &msx[(int)ch * 8];
    for(i = l = 0; i < 8; i++, l += 8, font++) {
      data_ptr = data;
      for(j = 0; j < 8; j++) {
	    if((*font & (128 >> j))) *data_ptr = color;
	    data_ptr++;
      }
      data += dest->realW;
    }
    x += 8;
  }
}

int LPPG_IsPGF(L_CONST char *filename)
{
    char fsig[7] = "";

    FILE *source = fopen(filename, "rb");

    if(!source)
    {
        return 0;
    }

    fread(fsig, 7, 1, source);
    fclose(source);

    if(fsig[4] == 'P' && fsig[5] == 'G' && fsig[6] == 'F')
        return 1;

    return 0;
}

LPP_intraFont *LPPG_LoadIntraFont(L_CONST char *filename)
{
    LPP_intraFont *fnt = intraFontLoad(filename, INTRAFONT_ALIGN_LEFT | INTRAFONT_CACHE_MED);
    return (fnt);
}

void LPPG_SetIntraFontAngle(LPP_intraFont *f, float angle) {
    if(angle < 0.0f) angle = 360.0f;
    if(angle > 360.0f) angle -= (360.0f * (int)(angle / 360.0f));

    if(f->angle != angle)
    {
        f->angle = angle;
        if(angle == 0.0f) {
            f->Rsin = 0.0f;
            f->Rcos = 1.0f;
            f->isRotated = 0;
        } else {
            f->Rsin = sinf(angle * GU_PI / 180.0f);
            f->Rcos = cosf(angle * GU_PI / 180.0f);
            f->isRotated = 1;
        }
    }
}

void LPPG_SetIntraFontSize(LPP_intraFont *f, float size) {
    f->size = size;
}

void LPPG_SetIntraFontStyle(LPP_intraFont *f, float size, Color textColor, Color shadowColor, float angle, u32 options)
{
    if(angle < 0.0f) angle = 360.0f;
    if(angle > 360.0f) angle -= (360.0f * (int)(angle / 360.0f));
    intraFontSetStyle(f, size, textColor, shadowColor, angle, options);
}

void LPPG_FreeIntraFont(LPP_intraFont *f)
{
    intraFontUnload(f);
}

void LPPG_IntraFontPrint(short x, short y, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrint(f, x, y, text);
    LPPG_EndDrawing();
}

void LPPG_IntraFontSetEncoding(LPP_intraFont *f, u32 encoding)
{
    intraFontSetEncoding(f, encoding);
}

float LPPG_IntraFontMeasureText(LPP_intraFont *f, L_CONST char *text)
{
    return intraFontMeasureText(f, text);
}

float LPPG_IntraFontMeasureTextEx(LPP_intraFont *f, L_CONST char *text, int len)
{
    return intraFontMeasureTextEx(f, text, len);
}

float LPPG_IntraFontMeasureTextUCS2(LPP_intraFont *f, L_CONST u16 *text)
{
    return intraFontMeasureTextUCS2(f, text);
}

float LPPG_IntraFontMeasureTextUCS2Ex(LPP_intraFont *f, L_CONST u16 *text, int len)
{
    return intraFontMeasureTextUCS2Ex(f, text, len);
}

void LPPG_IntraFontPrintEx(short x, short y, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor, int len)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrintEx(f, x, y, text, len);
    LPPG_EndDrawing();
}

void LPPG_IntraFontPrintUCS2(short x, short y, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrintUCS2(f, x, y, text);
    LPPG_EndDrawing();
}

void LPPG_IntraFontPrintUCS2Ex(short x, short y, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor, int len)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrintUCS2Ex(f, x, y, text, len);
    LPPG_EndDrawing();
}

void LPPG_IntraFontSetAltFont(LPP_intraFont *f, LPP_intraFont *a)
{
    intraFontSetAltFont(f, a);
}

void LPPG_IntraFontPrintColumn(short x, short y, short width, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor)
{
    LPPG_StartDrawing();
    f->color = textColor;
	f->shadowColor = shadowColor;
	intraFontPrintColumn(f, x, y, width, text);
	LPPG_EndDrawing();
}

void LPPG_IntraFontPrintColumnEx(short x, short y, short width, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor, int len)
{
    LPPG_StartDrawing();
    f->color = textColor;
	f->shadowColor = shadowColor;
	intraFontPrintColumnEx(f, x, y, width, text, len);
	LPPG_EndDrawing();
}

void LPPG_IntraFontPrintColumnUCS2(short x, short y, short width, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrintColumnUCS2(f, x, y, width, text);
    LPPG_EndDrawing();
}

void LPPG_IntraFontPrintColumnUCS2Ex(short x, short y, short width, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor, int len)
{
    LPPG_StartDrawing();
    f->color = textColor;
    f->shadowColor = shadowColor;
    intraFontPrintColumnUCS2Ex(f, x, y, width, text, len);
    LPPG_EndDrawing();
}

FT_Library library = null;

int LPPG_TTFInit(void)
{
    if(FT_Init_FreeType(&library))
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot initialize FreeType library.\n", __FUNCTION__, __LINE__);
		#endif
        return 0;
    }
    return 1;
}

void LPPG_TTFShutdown(void)
{
    FT_Done_FreeType(library);
}

void LPPG_FreeTrueType(LPP_TrueTypeFont *f)
{
    FT_Done_Face(f->face);
    if(f->name) free(f->name);
    if(f) free(f);
}

LPP_TrueTypeFont *LPPG_LoadTrueTypeImpl(void *data, size_t size, u32 fontSize, u32 fontsizeType)
{
    LPP_TrueTypeFont *f = (LPP_TrueTypeFont*)malloc(sizeof(LPP_TrueTypeFont));
	if(!f) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'f' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		return null;
	}

    memset(f, 0, sizeof(LPP_TrueTypeFont));

    f->name = null;

    FT_Error error = FT_New_Memory_Face(library, data, size, 0, &f->face);
    if(error)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot create new memory face (FreeType), error : %d", __FUNCTION__, __LINE__, error);
		#endif
        free(f);
        return null;
    }

    if(fontsizeType == LPP_FONT_SIZE_PIXELS)
    {
        if(FT_Set_Pixel_Sizes(f->face, fontSize, 0))
        {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot set font pixel sizes.\n", __FUNCTION__, __LINE__);
			#endif
            LPPG_FreeTrueType(f);
            return null;
        }
    }
    else
    {
        if(FT_Set_Char_Size(f->face, fontSize * 64, 0, 100, 0))
        {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot set font char size.\n", __FUNCTION__, __LINE__);
			#endif
            LPPG_FreeTrueType(f);
            return null;
        }
    }

    f->size = fontSize;

    return f;
}

LPP_TrueTypeFont *LPPG_LoadTrueType(L_CONST char *filename, u32 fontSize, u32 fontsizeType)
{
    FILE *Fp = fopen(filename, "rb");
    if(Fp == null)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filename);
		#endif
        return null;
    }

    fseek(Fp, 0, SEEK_END);
    size_t file_size = ftell(Fp);

    u8 *file_data = (u8*)malloc(file_size);
    if(file_data == null)
    {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'file_data' to memory.\n", __FUNCTION__, __LINE__);
		#endif
        fclose(Fp);
        return null;
    }

    rewind(Fp);
    fread(file_data, file_size, 1, Fp);
    fclose(Fp);

    LPP_TrueTypeFont *f = LPPG_LoadTrueTypeImpl(file_data, file_size, fontSize, fontsizeType);
    if(f == null) return null;

    f->name = strdup(filename);

    return (f);
}

LPP_TrueTypeFont *LPPG_TTFCreateProportional(u32 fontSize, u32 fontSizeType)
{
    LPP_TrueTypeFont *f = LPPG_LoadTrueTypeImpl(ttfVera, size_ttfVera, fontSize, fontSizeType);
    if(f == null) return null;

    L_CONST char *fname = "Vera proportional";
    f->name = strdup(fname);

    return f;
}

LPP_TrueTypeFont *LPPG_TTFCreateMonoSpaced(u32 fontSize, u32 fontSizeType)
{
    LPP_TrueTypeFont *f = LPPG_LoadTrueTypeImpl(ttfVeraMono, size_ttfVeraMono, fontSize, fontSizeType);
    if(f == null) return null;

    L_CONST char *fname = "Vera mono spaced";
    f->name = strdup(fname);

    return f;
}

void LPPG_TTFSetSize(LPP_TrueTypeFont *f, u32 fontSize, u32 fontSizeType)
{
    if(fontSizeType == LPP_FONT_SIZE_PIXELS)
    {
        if(FT_Set_Pixel_Sizes(f->face, fontSize, 0))
        {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot set font pixel sizes.\n", __FUNCTION__, __LINE__);
			#endif
            return;
        }
    }
    else
    {
        if(FT_Set_Char_Size(f->face, fontSize * 64, 0, 100, 0))
        {
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot set font char size.\n", __FUNCTION__, __LINE__);
			#endif
            return;
        }
    }

    f->size = fontSize;
}

u32 LPPG_TTFGetSize(LPP_TrueTypeFont *f)
{
    return f->size;
}

u32 LPPG_TTFSetCharSize(u32 width, u32 height, u32 dpiX, u32 dpiY, LPP_TrueTypeFont *f)
{
    return FT_Set_Char_Size(f->face, width, height, dpiX, dpiY);
}

void LPPG_TTFSetAngle(LPP_TrueTypeFont *f, float angle)
{
    if(angle < 0.0f) angle = 360.0f;
    if(angle > 360.0f) angle -= (360.0f * (int)(angle / 360.0f));

    f->angle = angle;
}

void LPPG_PrintTTFScreen(short x, short y, LPP_TrueTypeFont *f, L_CONST char *text, Color color)
{
    LPPG_StartDrawing();
    FT_GlyphSlot slot;
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error error;
    double angle;

    u32 num_chars = 0;

    if(!text || !*text) return;

    slot = f->face->glyph;
    f->color = color;

    num_chars = strlen(text);

    angle = ((f->angle / 360.0f) * GU_PI) * 2.0f;

    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = -matrix.xy;
	matrix.yy = matrix.xx;

    pen.x = x * 64;
    pen.y = (PSP_SCREEN_HEIGHT - y) * 64;

    int n;
    for(n = 0; n < num_chars; n++)
    {
        FT_Set_Transform(f->face, &matrix, &pen);

        error = FT_Load_Char(f->face, text[n], FT_LOAD_RENDER);
        if(error) continue;

        short xx = slot->bitmap_left;
        short yy = (PSP_SCREEN_HEIGHT - slot->bitmap_top);

        FT_Int i, j, p, q;
        FT_Int x_max = xx + (&slot->bitmap)->width;
        FT_Int y_max = yy + (&slot->bitmap)->rows;
        Color pixel, grey;
        int r, g, b;

        for(i = xx, p = 0; i < x_max; i++, p++)
        {
            for(j = yy, q = 0; j < y_max; j++, q++)
            {
                if(i >= PSP_SCREEN_WIDTH || i < 0 || j >= PSP_SCREEN_HEIGHT || j < 0) continue;

                grey = (&slot->bitmap)->buffer[q * (&slot->bitmap)->width + p];

                if(grey > 0)
                {
                    r = (grey * R(f->color)) / 255;
                    g = (grey * G(f->color)) / 255;
                    b = (grey * B(f->color)) / 255;
                    pixel = 0xff000000 | (b << 16) | (g << 8) | r;
                } else pixel = 0;

                if(pixel)
                {
                    LPPG_PutScreenPixel(i, j, pixel);
                }
            }
        }

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
    LPPG_EndDrawing();
}

void LPPG_PrintTTFSurface(short x, short y, LPP_TrueTypeFont *f, L_CONST char *text, Color color, LPP_Surface *dest)
{
    LPPG_StartDrawing();
    FT_GlyphSlot slot;
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error error;
    double angle;

    u32 num_chars = 0;

    if(!text || !*text) return;

    slot = f->face->glyph;
    f->color = color;

    num_chars = strlen(text);

    angle = ((f->angle / 360.0f) * GU_PI) * 2.0f;

    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = -matrix.xy;
	matrix.yy = matrix.xx;

    pen.x = x * 64;
    pen.y = (dest->realH - y) * 64;

    int n;
    for(n = 0; n < num_chars; n++)
    {
        FT_Set_Transform(f->face, &matrix, &pen);

        error = FT_Load_Char(f->face, text[n], FT_LOAD_RENDER);
        if(error) continue;

        short xx = slot->bitmap_left;
        short yy = (dest->realH - slot->bitmap_top);

        FT_Int i, j, p, q;
        FT_Int x_max = xx + (&slot->bitmap)->width;
        FT_Int y_max = yy + (&slot->bitmap)->rows;
        Color pixel, grey;
        int r, g, b;

        for(i = xx, p = 0; i < x_max; i++, p++)
        {
            for(j = yy, q = 0; j < y_max; j++, q++)
            {
                if(i >= dest->width || i < 0 || j >= dest->height || j < 0) continue;

                grey = (&slot->bitmap)->buffer[q * (&slot->bitmap)->width + p];

                if(grey > 0)
                {
                    r = (grey * R(f->color)) / 255;
                    g = (grey * G(f->color)) / 255;
                    b = (grey * B(f->color)) / 255;
                    pixel = 0xff000000 | (b << 16) | (g << 8) | r;
                } else pixel = 0;

                if(pixel)
                {
                    LPPG_PutSurfacePixel(i, j, pixel, dest);
                }
            }
        }

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
    LPPG_EndDrawing();
}

void LPPG_PrintTTFScreenFixed(short x, short y, LPP_TrueTypeFont *font, L_CONST char *text, Color color)
{
    size_t num_chars = strlen(text);
	FT_GlyphSlot slot = font->face->glyph;
	size_t n;
	for(n = 0; n < num_chars; n++) {
	    FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
		int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
		if(error) continue;
		error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
		if(error) continue;
		Color ccolor = color;
		u8 rf = ccolor & 0xff;
		u8 gf = (ccolor >> 8) & 0xff;
		u8 bf = (ccolor >> 16) & 0xff;
			
		u8 *line = (&slot->bitmap)->buffer;
		Color *fbLine = LPPG_GetVDrawBuffer() + (x + slot->bitmap_left) + (y - slot->bitmap_top) * PSP_LINE_SIZE;
		short i, k;
		for(i = 0; i < (&slot->bitmap)->rows; i++)
		{
			u8 *column = line;
			u32 *fbColumn = fbLine;
			for(k = 0; k < (&slot->bitmap)->width; k++) {
			    if(k + (x + slot->bitmap_left) < PSP_SCREEN_WIDTH && k + (x + slot->bitmap_left) >= 0 && i + (y - slot->bitmap_top) < PSP_SCREEN_HEIGHT && i + (y - slot->bitmap_top) >= 0)
				{
					u8 val = *column;
					ccolor = *fbColumn;
					u8 r = ccolor & 0xff;
					u8 g = (ccolor >> 8) & 0xff;
					u8 b = (ccolor >> 16) & 0xff;
					u8 a = (ccolor >> 24) & 0xff;
					r = rf * val / 255 + (255 - val) * r / 255;
					g = gf * val / 255 + (255 - val) * g / 255;
					b = bf * val / 255 + (255 - val) * b / 255;
					*fbColumn = r | (g << 8) | (b << 16) | (a << 24);
				}
				column++;
				fbColumn++;
			}
			line += (&slot->bitmap)->pitch;
			fbLine += PSP_LINE_SIZE;
		}
		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}
}

void LPPG_PrintTTFSurfaceFixed(short x, short y, LPP_TrueTypeFont *font, L_CONST char *text, Color color, LPP_Surface *dest)
{
    size_t num_chars = strlen(text);
	FT_GlyphSlot slot = font->face->glyph;
	size_t n;
	for(n = 0; n < num_chars; n++) {
	    FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
		int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
		if(error) continue;
		error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
		if(error) continue;
		Color ccolor = color;
		u8 rf = ccolor & 0xff;
		u8 gf = (ccolor >> 8) & 0xff;
		u8 bf = (ccolor >> 16) & 0xff;
			
		u8 *line = (&slot->bitmap)->buffer;
		Color *fbLine = dest->pixels + (x + slot->bitmap_left) + (y - slot->bitmap_top) * dest->realW;
		short i, k;
		for(i = 0; i < (&slot->bitmap)->rows; i++)
		{
			u8 *column = line;
			u32 *fbColumn = fbLine;
			for(k = 0; k < (&slot->bitmap)->width; k++) {
			    if(k + (x + slot->bitmap_left) < dest->width && k + (x + slot->bitmap_left) >= 0 && i + (y - slot->bitmap_top) < dest->height && i + (y - slot->bitmap_top) >= 0)
				{
					u8 val = *column;
					ccolor = *fbColumn;
					u8 r = ccolor & 0xff;
					u8 g = (ccolor >> 8) & 0xff;
					u8 b = (ccolor >> 16) & 0xff;
					u8 a = (ccolor >> 24) & 0xff;
					r = rf * val / 255 + (255 - val) * r / 255;
					g = gf * val / 255 + (255 - val) * g / 255;
					b = bf * val / 255 + (255 - val) * b / 255;
					*fbColumn = r | (g << 8) | (b << 16) | (a << 24);
				}
				column++;
				fbColumn++;
			}
			line += (&slot->bitmap)->pitch;
			fbLine += dest->realW;
		}
		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}
}

void LPPG_Init(void) {
    if(Initialized) return;

    LPPG_TTFInit();
    intraFontInit();

    dBuffer_n = 0;

    BackBuffer  = vrelptr(valloc(PSP_FRAME_BUFFER_SIZE));
    FrontBuffer = vrelptr(valloc(PSP_FRAME_BUFFER_SIZE));
    DepthBuffer = vrelptr(valloc(PSP_FRAME_BUFFER_SIZE >> 1));

    FrameBuffer = vabsptr(FrontBuffer);

    sceGuInit();
    sceGuStart(GU_DIRECT, dList[dBuffer_n]);

    sceGuDrawBuffer(GU_PSM_8888, FrontBuffer, PSP_LINE_SIZE);
    sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, BackBuffer, PSP_LINE_SIZE);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
    sceGuDepthBuffer(DepthBuffer, PSP_LINE_SIZE);

    sceGuOffset(2048 - (PSP_SCREEN_WIDTH >> 1), 2048 - (PSP_SCREEN_HEIGHT >> 1));
    sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
    sceGuDepthRange(0xc350, 0x2710);

    sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuAlphaFunc(GU_GREATER, 0, 0xff);

    sceGuEnable(GU_ALPHA_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuEnable(GU_CLIP_PLANES);
    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuAmbientColor(0xffffffff);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    sceGuClearColor(0x0);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    sceGuFinish();
    sceGuSync(0,0);

    sceDisplayWaitVblankStart();

    FrameBuffer = vabsptr(sceGuSwapBuffers());

    sceGuDisplay(GU_TRUE);

    Initialized = 1;
}

void LPPG_Shutdown(void) {
    if(!Initialized) return;

    LPPG_TTFShutdown();
    intraFontShutdown();

    LPPG_StartDrawing();
    vfree(vrelptr(BackBuffer));
    vfree(vrelptr(FrontBuffer));
    Initialized = 0;
    dBuffer_n = 0;
    sceGuTerm();
}
