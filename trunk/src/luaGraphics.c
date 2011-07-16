#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <png.h>
#include <pspgu.h>

#include <jpeglib.h>
#include <jerror.h>

#include "graphics.h"
#include "framebuffer.h"

#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define false 0
#define true 1
#define int_p_NULL NULL
#define png_voidp_NULL NULL
#define png_infopp_NULL NULL
#define png_bytep_NULL NULL
#define png_error_ptr_NULL NULL
#define png_set_gray_1_2_4_to_8 png_set_expand_gray_1_2_4_to_8
#define png_set_expand_gray_1_2_4_to_8(p) png_set_expand(p)

typedef struct
{
	unsigned short u, v;
	short x, y, z;
} Vertex;

extern u8 msx[];

unsigned int __attribute__((aligned(16))) list[262144];
static int dispBufferNumber;
static int initialized = 0;

static int getNextPower2(int width){
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

Color* getVramDrawBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 0) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

Color* getVramDisplayBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 1) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}

static int isJpegFile(const char* filename)
{
	char* suffix = strrchr(filename, '.');
	if (suffix) {
		if (stricmp(suffix, ".jpg") == 0 || stricmp(suffix, ".jpeg") == 0) return true;
	}
	return false;
}

Image* loadImage(const char* filename)
{
	if (isJpegFile(filename)) {
		return loadJpegImage(filename);
	} else {
		return loadPngImage(filename);
	}
}

Image* loadPngImage(const char* filename)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height, x, y;
	int bit_depth, color_type, interlace_type;
	u32* line;
	FILE *fp;
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;

	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		free(image);
		fclose(fp);
		return NULL;;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	if (width > 512 || height > 512) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	line = (u32*) malloc(width * 4);
	if (!line) {
		free(image->data);
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color = line[x];
			image->data[x + y * image->textureWidth] =  color;
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	return image;
}

Image* loadJpegImage(const char* filename)
{
	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;
	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&dinfo);
	FILE* inFile = fopen(filename, "rb");
	if (!inFile) {
		jpeg_destroy_decompress(&dinfo);
		return NULL;
	}
	jpeg_stdio_src(&dinfo, inFile);
	jpeg_read_header(&dinfo, TRUE);
	int width = dinfo.image_width;
	int height = dinfo.image_height;
	jpeg_start_decompress(&dinfo);
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) {
		jpeg_destroy_decompress(&dinfo);
		fclose(inFile);
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	u8* line = (u8*) malloc(width * 3);
	if (!line) {
		jpeg_destroy_decompress(&dinfo);
		fclose(inFile);
		return NULL;
	}
	if (dinfo.jpeg_color_space == JCS_GRAYSCALE) {
		while (dinfo.output_scanline < dinfo.output_height) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines(&dinfo, &line, 1);
			int x;
			for (x = 0; x < width; x++) {
				Color c = line[x];
				image->data[x + image->textureWidth * y] = c | (c << 8) | (c << 16) | 0xff000000;;
			}
		}
	} else {
		while (dinfo.output_scanline < dinfo.output_height) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines(&dinfo, &line, 1);
			u8* linePointer = line;
			int x;
			for (x = 0; x < width; x++) {
				Color c = *(linePointer++);
				c |= (*(linePointer++)) << 8;
				c |= (*(linePointer++)) << 16;
				image->data[x + image->textureWidth * y] = c | 0xff000000;
			}
		}
	}
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	free(line);
	fclose(inFile);
	return image;
}

void blitImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{
	Color* destinationData = &destination->data[destination->textureWidth * dy + dx];
	int destinationSkipX = destination->textureWidth - width;
	Color* sourceData = &source->data[source->textureWidth * sy + sx];
	int sourceSkipX = source->textureWidth - width;
	int x, y;
	for (y = 0; y < height; y++, destinationData += destinationSkipX, sourceData += sourceSkipX) {
		for (x = 0; x < width; x++, destinationData++, sourceData++) {
			*destinationData = *sourceData;
		}
	}
}

void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;
	Color* vram = getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
	sceGuFinish();
	sceGuSync(0,0);
}

void blitAlphaImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{
	Color* destinationData = &destination->data[destination->textureWidth * dy + dx];
	int destinationSkipX = destination->textureWidth - width;
	Color* sourceData = &source->data[source->textureWidth * sy + sx];
	int sourceSkipX = source->textureWidth - width;
	int x, y;
	s32 rcolorc, gcolorc, bcolorc, acolorc,rcolord, gcolord, bcolord, acolord;
	for (y = 0; y < height; y++, destinationData += destinationSkipX, sourceData += sourceSkipX) {
		for (x = 0; x < width; x++, destinationData++, sourceData++) {
			Color color = *sourceData;
			if (!IS_ALPHA(color)) {
				*destinationData = color;
			} else {
				rcolorc = color & 0xff;
				gcolorc = (color >> 8) & 0xff;
				bcolorc = (color >> 16) & 0xff;
				acolorc = (color >> 24) & 0xff;
				rcolord = *destinationData & 0xff;
				gcolord = (*destinationData >> 8) & 0xff;
				bcolord = (*destinationData >> 16) & 0xff;
				acolord = (*destinationData >> 24) & 0xff;
				
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
				*destinationData = rcolorc | (gcolorc << 8) | (bcolorc << 16) | (acolorc << 24);
			}
		}
	}
}

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

Image* createImage(int width, int height)
{
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) return NULL;
	memset(image->data, 0, image->textureWidth * image->textureHeight * sizeof(Color));
	return image;
}

void freeImage(Image* image)
{
	free(image->data);
	free(image);
}

void clearImage(Color color, Image* image)
{
	int i;
	int size = image->textureWidth * image->textureHeight;
	Color* data = image->data;
	for (i = 0; i < size; i++, data++) *data = color;
}

void clearScreen(Color color)
{
	if (!initialized) return;
	guStart();
	sceGuClearColor(color);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}

void fillImageRect(Color color, int x0, int y0, int width, int height, Image* image)
{
	int skipX = image->textureWidth - width;
	int x, y;
	Color* data = image->data + x0 + y0 * image->textureWidth;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void fillScreenRect(Color color, int x0, int y0, int width, int height)
{
	if (!initialized) return;
	int skipX = PSP_LINE_SIZE - width;
	int x, y;
	Color* data = getVramDrawBuffer() + x0 + y0 * PSP_LINE_SIZE;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void putPixelScreen(Color color, int x, int y)
{
	Color* vram = getVramDrawBuffer();
	vram[PSP_LINE_SIZE * y + x] = color;
}

void putPixelImage(Color color, int x, int y, Image* image)
{
	image->data[x + y * image->textureWidth] = color;
}

Color getPixelScreen(int x, int y)
{
	Color* vram = getVramDrawBuffer();
	return vram[PSP_LINE_SIZE * y + x];
}

Color getPixelImage(int x, int y, Image* image)
{
	return image->data[x + y * image->textureWidth];
}

void printTextScreen(int x, int y, const char* text, u32 color)
{
	int i, j, l;
	u8 *font;
	Color *vram_ptr;
	Color *vram;
	
	if (!initialized) return;
	size_t c;
	for (c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 > SCREEN_WIDTH || y < 0 || y + 8 > SCREEN_HEIGHT) break;
		char ch = text[c];
		vram = getVramDrawBuffer() + x + y * PSP_LINE_SIZE;
		
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			vram_ptr  = vram;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *vram_ptr = color;
				vram_ptr++;
			}
			vram += PSP_LINE_SIZE;
		}
		x += 8;
	}
}

void printTextImage(int x, int y, const char* text, u32 color, Image* image)
{
	int i, j, l;
	u8 *font;
	Color *data_ptr;
	Color *data;
	
	if (!initialized) return;
	size_t c;
	for (c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 > image->imageWidth || y < 0 || y + 8 > image->imageHeight) break;
		char ch = text[c];
		data = image->data + x + y * image->textureWidth;
		
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			data_ptr  = data;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *data_ptr = color;
				data_ptr++;
			}
			data += image->textureWidth;
		}
		x += 8;
	}
}

static void fontPrintTextImpl(FT_Bitmap* bitmap, int xofs, int yofs, Color color, Color* framebuffer, int width, int height, int lineSize)
{
	u8 rf = color & 0xff; 
	u8 gf = (color >> 8) & 0xff;
	u8 bf = (color >> 16) & 0xff;
	
	u8* line = bitmap->buffer;
	Color* fbLine = framebuffer + xofs + yofs * lineSize;
	int y;
	int x;
	for (y = 0; y < bitmap->rows; y++) {
		u8* column = line;
		Color* fbColumn = fbLine;
		for (x = 0; x < bitmap->width; x++) {
			if (x + xofs < width && x + xofs >= 0 && y + yofs < height && y + yofs >= 0) {
				u8 val = *column;
				color = *fbColumn;
				u8 r = color & 0xff; 
				u8 g = (color >> 8) & 0xff;
				u8 b = (color >> 16) & 0xff;
				u8 a = (color >> 24) & 0xff;
				r = rf * val / 255 + (255 - val) * r / 255;
				g = gf * val / 255 + (255 - val) * g / 255;
				b = bf * val / 255 + (255 - val) * b / 255;
				*fbColumn = r | (g << 8) | (b << 16) | (a << 24);
			}
			column++;
			fbColumn++;
		}
		line += bitmap->pitch;
		fbLine += lineSize;
	}
}

void fontPrintTextImage(FT_Bitmap* bitmap, int x, int y, Color color, Image* image)
{
	fontPrintTextImpl(bitmap, x, y, color, image->data, image->imageWidth, image->imageHeight, image->textureWidth);
}

void fontPrintTextScreen(FT_Bitmap* bitmap, int x, int y, Color color)
{
	fontPrintTextImpl(bitmap, x, y, color, getVramDrawBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE);
}

void saveImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha)
{
	if (isJpegFile(filename)) {
		saveJpegImage(filename, data, width, height, lineSize);
	} else {
		savePngImage(filename, data, width, height, lineSize, saveAlpha);
	}
}

void savePngImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha)
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;
	int i, x, y;
	u8* line;
	
	if ((fp = fopen(filename, "wb")) == NULL) return;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) return;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8,
		saveAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	line = (u8*) malloc(width * (saveAlpha ? 4 : 3));
	for (y = 0; y < height; y++) {
		for (i = 0, x = 0; x < width; x++) {
			Color color = data[x + y * lineSize];
			u8 r = color & 0xff; 
			u8 g = (color >> 8) & 0xff;
			u8 b = (color >> 16) & 0xff;
			u8 a = saveAlpha ? (color >> 24) & 0xff : 0xff;
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
			if (saveAlpha) line[i++] = a;
		}
		png_write_row(png_ptr, line);
	}
	free(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
}

void saveJpegImage(const char* filename, Color* data, int width, int height, int lineSize)
{
	FILE* outFile = fopen(filename, "wb");
	if (!outFile) return;
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct cinfo;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outFile);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	u8* row = (u8*) malloc(width * 3);
	if (!row) return;
	int y;
	int x;
	for (y = 0; y < height; y++) {
		u8* rowPointer = row;		
		for (x = 0; x < width; x++) {
			Color c = data[x + cinfo.next_scanline * lineSize];
			*(rowPointer++) = c & 0xff;
			*(rowPointer++) = (c >> 8) & 0xff;
			*(rowPointer++) = (c >> 16) & 0xff;
		}
		jpeg_write_scanlines(&cinfo, &row, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outFile);
	free(row);
}

void flipScreen()
{
	if (!initialized) return;
	sceGuSwapBuffers();
	dispBufferNumber ^= 1;
}

static void drawLine(int x0, int y0, int x1, int y1, int color, Color* destination, int width)
{
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			destination[x0+y0] = color;
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			destination[x0+y0] = color;
		}
	}
}

void drawLineScreen(int x0, int y0, int x1, int y1, Color color)
{
	drawLine(x0, y0, x1, y1, color, getVramDrawBuffer(), PSP_LINE_SIZE);
}

void drawLineImage(int x0, int y0, int x1, int y1, Color color, Image* image)
{
	drawLine(x0, y0, x1, y1, color, image->data, image->textureWidth);
}

//Fast Blit Functions!
static int fastInit = 0;
void FastStartBlit()
{
	guStart();
	fastInit = 1;
}

void FastEndBlit()
{
	if (!fastInit) return;
	sceGuFinish();
	sceGuSync(0, 0);
	fastInit = 0;
}

void fastClearScreen(Color color)
{
	if (!initialized || !fastInit) return;
	sceGuClearColor(color);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
}

void fastBlitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized || !fastInit) return;
	Color* vram = getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
}

void fastBlitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized || !fastInit) return;

	sceKernelDcacheWritebackInvalidateAll();
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

void initGraphics()
{
	dispBufferNumber = 0;

	sceGuInit();

	guStart();
	sceGuDrawBuffer(GU_PSM_8888, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) (FRAMEBUFFER_SIZE*2), PSP_LINE_SIZE);
	sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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
	sceGuFinish();
	sceGuSync(0, 0);
	
	sceGuCallMode(GU_FALSE);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	initialized = 1;
}

void disableGraphics()
{
	initialized = 0;
}

void guStart()
{
	sceGuStart(GU_DIRECT, list);
}
