/** LPP Graphics lib by Nanni */

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

#ifndef __GRAPHICSL_H_
#define __GRAPHICSL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Some Macros */
#define PSP_SCREEN_WIDTH  (480)
#define PSP_SCREEN_HEIGHT (272)
#define PSP_BUFFER_WIDTH  (512)
#define PSP_LINE_SIZE     (512)
#define PSP_FRAME_BUFFER_SIZE (PSP_LINE_SIZE*PSP_SCREEN_HEIGHT*4)

#define IS_ALPHA(c) (((c)&0xff000000) == 0xff000000 ? 0 : 1)
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))
#define RGBA(r,g, b, a) (a << 24 | b << 16 | g << 8 | r)
#define RGB(r, g, b) RGBA(r, g, b, 255)

#include "../Types.h"
#include "../intraFont/intraFont.h"

#include "../Psar/Psar.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define Bm32(b,a) ((int)((u8)b[a] | (u16)(b[a+1]<<8) | (u32)(b[a+2]<<16) | (u32)(b[a+3]<<24)))
#define Bm16(b,a) ((short)((u8)b[a] | (u16)(b[a+1]<<8)))

typedef struct {
    u16 u, v;
    short x, y, z;
} LPP_VertTV;

typedef struct {
    u32 color;
    short x, y, z;
} LPP_VertCV;

typedef struct {
    short x, y, z;
} LPP_VertV;

typedef struct {
    float u, v;
    short x, y, z;
} LPP_VertUVf;

typedef struct {
    float u, v;
	float x, y, z;
} LPP_VertTVf;

typedef struct {
    short u, v;
	float x, y, z;
} LPP_VertT;

#define BMP_ID "BM"

struct BitmapHeader
{
	char id[2];
	uint32_t filesize;
	uint32_t reserved;
	uint32_t offset;
	uint32_t headsize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t comp;
	uint32_t bitmapsize;
	uint32_t hres;
	uint32_t vres;
	uint32_t colors;
	uint32_t impcolors;
} __attribute__((packed));

/**
 * Enum for Image location.
 */
enum LPP_Location {
    LPP_RAM = 0x2,
    LPP_VRAM = 0x4
};

/**
 * Enum for TrueType font size.
 */
enum LPP_FontSizeType {
    LPP_FONT_SIZE_PIXELS = 0,
    LPP_FONT_SIZE_POINTS = 1
};

/**
 * Enum for Image extension identifier.
 */
enum LPP_ImageFormat {
    LPP_Image_PNG = 1, /* Portable Network Graphics */
    LPP_Image_JPG = 2, /* JPEG */
    LPP_Image_BMP = 3, /* BitMap */
    LPP_Image_TGA = 4, /* Targa */
    LPP_Image_GIF = 5  /* Graphics Interchange Format */
};

/**
 * An Image struct.
 */
typedef struct LPP_Surface {
    u32 *pixels; /**< Image data. */
    u32 width, height; /**< Image dimensions. */
    u32 realW, realH; /**< Image texture sizes. */
    u32 bpp; /**< Bit per pixel. */
    u8 swizzled; /**< Is Image swizzled. */
    enum LPP_Location location; /**< Image memory location. (RAM or VRAM) */
} LPP_Surface;

/**
 * An Intrafont struct.
 */
typedef intraFont LPP_intraFont;

/**
 * An TrueType font struct.
 */
typedef struct {
    char *name; /**< Font name (or path). */
    FT_Face face;
    Color color; /**< Font text color. */
    float angle; /**< Font angle. */
    u32 size; /**< Font size (Pixels or Points). */
} LPP_TrueTypeFont;

#define LPP_Surface_size (sizeof(LPP_Surface))

/**
 * Get the current DrawBuffer.
 *
 * @return A pointer to the current drawbuffer.
 */
u32 *LPPG_GetVDrawBuffer(void);

/**
 * Get the current DisplayBuffer.
 *
 * @return A pointer to the current displaybuffer.
 */
u32 *LPPG_GetVDisplayBuffer(void);

/**
 * Get the current framebuffer.
 *
 * @raturn a pointer to the current framebuffer.
 *
 */
u32 *LPPG_GetFrameBuffer(void);

/**
 * Check if an image is PNG.
 *
 * @param filename - The image path.
 *
 * @return 1 if the image is PNG.
 */
int LPPG_IsPNG(L_CONST char *filename);

/**
 * Check if an image is JPG.
 *
 * @param filename - The image path.
 *
 * @return 1 if the image is JPG.
 */
int LPPG_IsJPG(L_CONST char *filename);

/**
 * Check if an image is BMP.
 *
 * @param filename - The image path.
 *
 * @return 1 if the image is BMP.
 */
int LPPG_IsBMP(L_CONST char *filename);

/**
 * Check if an image is TGA.
 *
 * @param filename - The image path.
 *
 * @return 1 if the image is TGA.
 */
int LPPG_IsTGA(L_CONST char *filename);

/**
 * Check if an image is GIF.
 *
 * @param filename - The image path.
 *
 * @return 1 if the image is GIF.
 */
int LPPG_IsGIF(L_CONST char *filename);

/**
 * Check if a file is contained in DATA.PSAR
 *
 * @param f - The file path.
 *
 * @return 1 if the file is in DATA.PSAR.
 */
u8 LPPG_IsInPsar(L_CONST char *f);

/**
 * Load an image from DATA.PSAR.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageFPsar(L_CONST char *filename);

/**
  *
  * Load an image from a buffer.
  *
  * @param data - The image buffer.
  *
  * @param len - The image size in bytes.
  *
  * @param filename - no needed it can be an empty string.
  *
  * @return a pointer to the loaded image.
  *
  */
LPP_Surface *LPPG_LoadImageFMem(u8* data, size_t len, L_CONST char *filename);

void LPPG_InitTimer(void);

float LPPG_GetDeltaTime(void);

int LPPG_GetFPS(void);

/**
 * Load a PNG image.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImagePNG(L_CONST char *filename);

/**
 * Load a JPG image.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageJPG(L_CONST char *filename);

/**
 * Load a BMP image.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageBMP(L_CONST char *filename);

/**
 * Load a TGA image.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageTGA(L_CONST char *filename);

/**
 * Load a GIF image.
 *
 * @param filename - The image path.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageGIF(L_CONST char *filename);

/**
 * Load a GIF image frame.
 *
 * @param filename - The image path.
 *
 * @param frame - The frame number to load.
 *
 * @return A pointer to loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImageGIFframe(L_CONST char *filename, int frame);

/**
 * Return the number of frames contained in a GIF image.
 *
 * @param filename - The image path.
 *
 * @return The number of frames.
 */
u32 LPPG_GifFramesCount(L_CONST char *filename);

/**
 * Load an Image.
 *
 * @param filename - The path of the image.
 *
 * @return A pointer to the loaded LPP_Surface or null on error.
 */
LPP_Surface *LPPG_LoadImage(L_CONST char *filename);

/**
 * Create an empty LPP_Surface.
 *
 * @param Width - The surface width.
 *
 * @param Height - The surface height;
 *
 * @return a pointer to the created LPP_Surface or null on error.
 */
LPP_Surface *LPPG_CreateSurface(u32 Width, u32 Height);

/**
 * Create a LPP_Surface clone.
 *
 * @param s - The surface to copy.
 *
 * @return a pointer to the cloned LPP_Surface or null on error.
 */
LPP_Surface *LPPG_CopySurface(LPP_Surface *s);

/**
 * Check if s1 is equals to s2.
 *
 * @param s1 - the first surface.
 *
 * @param s1 - the second surface.
 *
 * @return 1 if s1 are equals to s2 else 0
 */
u8 LPPG_SurfaceEquals(LPP_Surface *s1, LPP_Surface *s2);

/**
 * Delete a surface from memory.
 *
 * @param s - The surface to delete.
 *
 */
void LPPG_FreeSurface(LPP_Surface *s);

/**
 * Swap the framebuffers.
 */
void LPPG_FlipScreen(void);

/**
 * Start drawing.
 */
void LPPG_StartDrawing(void);

/**
 * End drawing.
 */
void LPPG_EndDrawing(void);

/**
 * Clear a surface.
 *
 * @param s - the surface to clear.
 *
 * @param color - the color to clear surface to.
 *
 */
void LPPG_ClearSurface(LPP_Surface *s, Color color);

/**
 * Clear the screen.
 *
 * @param color - the color to clear screen to.
 *
 */
void LPPG_ClearScreen(Color color);

/**
 * Flip a surface vertically.
 *
 * @param s - the LPP_Surface to flip.
 *
 * @return a pointer to the flipper surface.
 *
 */
LPP_Surface *LPPG_FlipSurfaceVertical(LPP_Surface *s);

/**
 * Swizzle a LPP_Surface.
 *
 * @param s - the surface to swizzle.
 *
 */
void LPPG_SwizzleSurface(LPP_Surface *s);

/**
 * UnSwizzle a LPP_Surface.
 *
 * @param s - the surface to unswizzle.
 *
 */
void LPPG_UnSwizzleSurface(LPP_Surface *s);

/**
 * Move a LPP_Surface to Vram.
 *
 * @param s - the surface to move.
 *
 */
void LPPG_SurfaceToVram(LPP_Surface *s);

/**
 * Move a LPP_Surface to Ram.
 *
 * @param s - the surface to move.
 *
 */
void LPPG_SurfaceToRam(LPP_Surface *s);

/**
 * Resize a LPP_Surface.
 *
 * @param s - the surface to resize.
 *
 * @param new_width - the new surface width.
 *
 * @param new_height - the new surface height.
 *
 * @return a pointer to the resized surface.
 *
 */
LPP_Surface *LPPG_SurfaceResized(LPP_Surface *s, u32 new_width, u32 new_height);

/**
 * Return the negative of a LPP_Surface.
 *
 * @param s - the surface to get negative.
 *
 * @return a pointer to the negative.
 *
 */
LPP_Surface *LPPG_SurfaceNegative(LPP_Surface *s);

/**
 * Return the negative of the screen.
 *
 * @return a pointer to the negative.
 *
 */
LPP_Surface *LPPG_ScreenNegative(void);

/**
 * Draw a fill rect on a LPP_Surface.
 *
 * @param x0 - the start x position of the rect.
 *
 * @param y0 - the start y position of the rect.
 *
 * @param width - the rect width.
 *
 * @param height - the rect height.
 *
 * @param color - the rect color.
 *
 * @param s - the surface to draw rect to.
 *
 */
void LPPG_FillSurfaceRect(int x0, int y0, int width, int height, Color color, LPP_Surface *dest);

/**
 * Draw a fill rect on the screen.
 *
 * @param x0 - the start x position of the rect.
 *
 * @param y0 - the start y position of the rect.
 *
 * @param width - the rect width.
 *
 * @param height - the rect height.
 *
 * @param color - the rect color.
 *
 */
void LPPG_FillScreenRect(int x, int y, int width, int height, Color color);

/**
 * Draw an outline rect on a LPP_Surface.
 *
 * @param x0 - the start x position of the rect.
 *
 * @param y0 - the start y position of the rect.
 *
 * @param width - the rect width.
 *
 * @param height - the rect height.
 *
 * @param color - the rect color.
 *
 * @param s - the surface to draw rect to.
 *
 */
void LPPG_SurfaceRect(int x, int y, int width, int height, Color color, LPP_Surface *s);

/**
 * Draw an outline rect on the screen.
 *
 * @param x0 - the start x position of the rect.
 *
 * @param y0 - the start y position of the rect.
 *
 * @param width - the rect width.
 *
 * @param height - the rect height.
 *
 * @param color - the rect color.
 *
 */
void LPPG_ScreenRect(int x, int y, int width, int height, Color color);

/**
 * Draw a line on a LPP_Surface.
 *
 * @param x1 - the start x position of the line.
 *
 * @param y1 - the start y position of the line.
 *
 * @param x2 - the end x position of the line.
 *
 * @param y2 - the end y position of the line.
 *
 * @param s - the surface to draw line to.
 *
 */
void LPPG_SurfaceLine(int x1, int y1, int x2, int y2, Color color, LPP_Surface *s);

/**
 * Draw a line on the screen.
 *
 * @param x1 - the start x position of the line.
 *
 * @param y1 - the start y position of the line.
 *
 * @param x2 - the end x position of the line.
 *
 * @param y2 - the end y position of the line.
 *
 */
void LPPG_ScreenLine(int x1, int y1, int x2, int y2, Color color);

/**
 * Draw a gradiend rect on the screen.
 *
 * @param x - the start x cordinate of the rect.
 *
 * @param y - the start y cordinate of the rect.
 *
 * @param width - the rect width.
 *
 * @param height - the rect height.
 *
 * @param c1 - top left corner color.
 *
 * @param c2 - top right corner color.
 *
 * @param c3 - bottom left corner color.
 *
 * @param c4 - bottom right corner color.
 *
 */
void LPPG_DrawGradientRect(int x, int y, int width, int height, Color c1, Color c2, Color c3, Color c4);

/**
 * Draw a fill circle to a LPP_Surface.
 *
 * @param centerX - the x cordinate of the circle.
 *
 * @param centerY - the y cordinate of the circle.
 *
 * @param radius - the radius of the circle.
 *
 * @param color - the color of the circle.
 *
 * @param s - the surface to draw circle to.
 *
 */
void LPPG_FillSurfaceCircle(int centerX, int centerY, int radius, Color color, LPP_Surface *s);

/**
 * Draw a fill circle to the screen.
 *
 * @param centerX - the x cordinate of the circle.
 *
 * @param centerY - the y cordinate of the circle.
 *
 * @param radius - the radius of the circle.
 *
 * @param color - the color of the circle.
 *
 */
void LPPG_FillScreenCircle(int centerX, int centerY, int radius, Color color);

/**
 * Draw an outline circle to a LPP_Surface.
 *
 * @param centerX - the x cordinate of the circle.
 *
 * @param centerY - the y cordinate of the circle.
 *
 * @param radius - the radius of the circle.
 *
 * @param color - the color of the circle.
 *
 * @param s - the surface to draw circle to.
 *
 */
void LPPG_SurfaceCircle(int centerX, int centerY, int radius, Color color, LPP_Surface *s);

/**
 * Draw an outline circle to the screen.
 *
 * @param centerX - the x cordinate of the circle.
 *
 * @param centerY - the y cordinate of the circle.
 *
 * @param radius - the radius of the circle.
 *
 * @param color - the color of the circle.
 *
 */
void LPPG_ScreenCircle(int centerX, int centerY, int radius, Color color);

/**
 * Set a color value in a LPP_Surface.
 *
 * @param x - x position.
 *
 * @param y - y position.
 *
 * @param color - the color value to set.
 *
 * @param s - the surface to set pixel.
 *
 */
void LPPG_PutSurfacePixel(int x, int y, Color color, LPP_Surface *dest);

/**
 * Set a color value in the screen.
 *
 * @param x - x position.
 *
 * @param y - y position.
 *
 * @param color - the color value to set.
 *
 */
void LPPG_PutScreenPixel(int x, int y, Color color);

/**
 * Get a color value from a LPP_Surface.
 *
 * @param x - x position.
 *
 * @param y - y position.
 *
 * @param s - the surface to get pixel.
 *
 * @return the color.
 *
 */
Color LPPG_GetSurfacePixel(int x, int y, LPP_Surface *dest);

/**
 * Get a color value from the screen.
 *
 * @param x - x position.
 *
 * @param y - y position.
 *
 * @return the color.
 *
 */
Color LPPG_GetScreenPixel(int x, int y);

/**
 * Save a LPP_Surface to an image file.
 *
 * @param filename - the image path.
 *
 * @param format - the image format (One of LPP_ImageFormat).
 *
 * @param s - the surface to save.
 *
 */
void LPPG_SaveSurface(L_CONST char *filename, u32 format, LPP_Surface *s);

/**
 * Save the screen to an image file.
 *
 * @param filename - the image path.
 *
 * @param format - the image format (One of LPP_ImageFormat).
 *
 */
void LPPG_SaveScreen(L_CONST char *filename, u32 format);

/**
 * Draw a LPP_Surface to the screen.
 *
 * @param x - the x cordinate of the surface.
 *
 * @param y - the y cordinate of the surface.
 *
 * @param source - the surface to draw.
 *
 * @param alpha - the image transparency (0 - 255).
 *
 * @param sx - the source x.
 *
 * @param sy - the source y.
 *
 * @param w - the image width.
 *
 * @param h - the image height.
 *
 */
void LPPG_BlitSurfaceScreen(float x, float y, LPP_Surface *source, u8 alpha, float angle, float sx, float sy, float w, float h);

/**
 * Draw a LPP_Surface to an other LPP_Surface.
 *
 * @param x - the x cordinate of the surface.
 *
 * @param y - the y cordinate of the surface.
 *
 * @param source - the surface to draw.
 *
 * @param sx - the source x.
 *
 * @param sy - the source y.
 *
 * @param w - the image width.
 *
 * @param h - the image height.
 *
 * @param dest - the surface to draw source to.
 *
 */
void LPPG_BlitSurfaceSurface(int x, int y, LPP_Surface *source, int sx,int sy, int w,int h, LPP_Surface *dest);

/**
 * Print some text to the screen.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 */
void LPPG_PrintTextScreen(int x, int y, L_CONST char *text, u32 color);

/**
 * Print some text to a LPP_Surface.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 * @param dest - the surface to print text to.
 *
 */
void LPPG_PrintTextSurface(int x, int y, L_CONST char *text, u32 color, LPP_Surface *dest);

/**
 * Check if a file is a PGF Font.
 *
 * @param filename - the file path.
 *
 * @return 1 if the file is PGF.
 *
 */
int LPPG_IsPGF(L_CONST char *filename);

/**
 * Load an intraFont file.
 *
 * @param filename - the font path.
 *
 * @return a pointer to the loaded LPP_intraFont or null on error.
 *
 */
LPP_intraFont *LPPG_LoadIntraFont(L_CONST char *filename);

/**
 * Set the angle of a LPP_intraFont.
 *
 * @param f - the font to set angle.
 *
 * @param angle - the font angle.
 *
 */
void LPPG_SetIntraFontAngle(LPP_intraFont *f, float angle);

/**
 * Set the size of a LPP_intraFont.
 *
 * @param f - the font to set size.
 *
 * @param size - the font size.
 *
 */
void LPPG_SetIntraFontSize(LPP_intraFont *f, float size);

/**
 * Set the style of a LPP_intraFont.
 *
 * @param f - the font to set style.
 *
 * @param size - the font size.
 *
 * @param textColor - the font textColor.
 *
 * @param shadowColor - the font shadowColor.
 *
 * @param angle - the font angle.
 *
 * @param options - the font flags.
 *
 */
void LPPG_SetIntraFontStyle(LPP_intraFont *f, float size, Color textColor, Color shadowColor, float angle, u32 options);

/**
 * Delete a LPP_intraFont from the memory.
 *
 * @param f - the font to delete.
 *
 */
void LPPG_FreeIntraFont(LPP_intraFont *f);

/**
 * Set the encoding of a LPP_intraFont.
 *
 * @param f - the LPP_intraFont.
 *
 * @param encoding - the encoding.
 *
 */
void LPPG_IntraFontSetEncoding(LPP_intraFont *f, u32 encoding);

/**
 * Measure a lenght of text if it were to be drawn
 *
 * @param f - a valid LPP_intraFont.
 *
 * @param text - text to measure.
 *
 */
float LPPG_IntraFontMeasureText(LPP_intraFont *f, L_CONST char *text);

/**
 * Measure a lenght of text if it were to be drawn
 *
 * @param f - a valid LPP_intraFont.
 *
 * @param text - text to measure.
 *
 * @param len - char lenght of text to measure.
 *
 */
float LPPG_IntraFontMeasureTextEx(LPP_intraFont *f, L_CONST char *text, int len);

/**
 * Measure a lenght of UCS2 text if it were to be drawn
 *
 * @param f - a valid LPP_intraFont.
 *
 * @param text - text to measure.
 *
 */
float LPPG_IntraFontMeasureTextUCS2(LPP_intraFont *f, L_CONST u16 *text);

/**
 * Measure a lenght of UCS2 text if it were to be drawn
 *
 * @param f - a valid LPP_intraFont.
 *
 * @param text - text to measure.
 *
 * @param len - char lenght of text to measure.
 *
 */
float LPPG_IntraFontMeasureTextUCS2Ex(LPP_intraFont *f, L_CONST u16 *text, int len);

/**
 * Set alternative font.
 *
 * @param f - a valid LPP_intraFont.
 *
 * @param a - the LPP_intraFont that's to be used if font doesn't contain a character.
 *
 */
void LPPG_IntraFontSetAltFont(LPP_intraFont *f, LPP_intraFont *a);


/**
 * Print some text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 */
void LPPG_IntraFontPrint(short x, short y, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor);

/**
 * Print some text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 * @param len - char lenght of text to print.
 *
 */
void LPPG_IntraFontPrintEx(short x, short y, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor, int len);

/**
 * Print some text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param width - column width for automatic line breaking.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 */
void LPPG_IntraFontPrintColumn(short x, short y, short width, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor);

/**
 * Print some text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param width - column width for automatic line breaking.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 * @param len - char lenght of text to print.
 *
 */
void LPPG_IntraFontPrintColumnEx(short x, short y, short width, LPP_intraFont *f, L_CONST char *text, Color textColor, Color shadowColor, int len);

/**
 * Print some UCS2 text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 */
void LPPG_IntraFontPrintUCS2(short x, short y, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor);

/**
 * Print some UCS2 text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 * @param len - char lenght of text to print.
 *
 */
void LPPG_IntraFontPrintUCS2Ex(short x, short y, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor, int len);

/**
 * Print some UCS2 text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param width - column width for automatic line breaking.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 */
void LPPG_IntraFontPrintColumnUCS2(short x, short y, short width, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor);

/**
 * Print some UCS2 text to screen using a LPP_intraFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param width - column width for automatic line breaking.
 *
 * @param f - the intraFont to using for print text.
 *
 * @param text - the text to print.
 *
 * @param textColor - the text color.
 *
 * @param shadowColor - the intraFont shadow color.
 *
 * @param len - char lenght of text to print.
 *
 */
void LPPG_IntraFontPrintColumnUCS2Ex(short x, short y, short width, LPP_intraFont *f, L_CONST u16 *text, Color textColor, Color shadowColor, int len);

/**
 * Load TrueType font.
 *
 * @param filename - the path of the font.
 *
 * @param fontSize - the font size.
 *
 * @param fontSizeType - the font size type (Pixels or Points).
 *
 * @return a pointer to the loaded font.
 *
 */
LPP_TrueTypeFont *LPPG_LoadTrueType(L_CONST char *filename, u32 fontSize, u32 fontsizeType);

/**
 * Delete a TrueType font from the memory.
 *
 * @param f - the font to delete.
 *
 */
void LPPG_FreeTrueType(LPP_TrueTypeFont *f);

/**
 * Print text to the screen using a LPP_TrueTypeFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the TrueType font.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 */
void LPPG_PrintTTFScreen(short x, short y, LPP_TrueTypeFont *f, L_CONST char *text, Color color);

/**
 * Print text on a surface using a LPP_TrueTypeFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the TrueType font.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 * @param dest - the surface to print text to.
 *
 */
void LPPG_PrintTTFSurface(short x, short y, LPP_TrueTypeFont *f, L_CONST char *text, Color color, LPP_Surface *dest);

/**
 * Print text to the screen using a LPP_TrueTypeFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the TrueType font.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 */
void LPPG_PrintTTFScreenFixed(short x, short y, LPP_TrueTypeFont *font, L_CONST char *text, Color color);

/**
 * Print text on a surface using a LPP_TrueTypeFont.
 *
 * @param x - the x cordinate of the text.
 *
 * @param y - the y cordinate of the text.
 *
 * @param f - the TrueType font.
 *
 * @param text - the text.
 *
 * @param color - the text color.
 *
 * @param dest - the surface to print text to.
 *
 */
void LPPG_PrintTTFSurfaceFixed(short x, short y, LPP_TrueTypeFont *font, L_CONST char *text, Color color, LPP_Surface *dest);

/**
 * Set the angle of a TrueType font.
 *
 * @param f - the TrueType font.
 *
 * @param angle - the angle (in degrees).
 *
 */
void LPPG_TTFSetAngle(LPP_TrueTypeFont *f, float angle);

/**
 * Create a Vera proportional LPP_TrueTypeFont.
 *
 * @param fontSize - the font size.
 *
 * @param fontsizeType - the font sizeType (Pixels or Points).
 *
 * @return a pointer to the created font.
 *
 */
LPP_TrueTypeFont *LPPG_TTFCreateProportional(u32 fontSize, u32 fontSizeType);

/**
 * Create a Vera mono LPP_TrueTypeFont.
 *
 * @param fontSize - the font size.
 *
 * @param fontsizeType - the font sizeType (Pixels or Points).
 *
 * @return a pointer to the created font.
 *
 */
LPP_TrueTypeFont *LPPG_TTFCreateMonoSpaced(u32 fontSize, u32 fontSizeType);

/**
 * Set the size of a LPP_TrueTypeFont.
 *
 * @param fontSize - the font size.
 *
 * @param fontSizeType - the font sizeType (Pixels or Points).
 *
 */
void LPPG_TTFSetSize(LPP_TrueTypeFont *f, u32 fontSize, u32 fontSizeType);

/**
 * Get the size of a LPP_TrueTypeFont.
 *
 * @param f - a valid LPP_TrueTypeFont.
 *
 * @return the font size.
 *
 */
u32 LPPG_TTFGetSize(LPP_TrueTypeFont *f);

/**
 * Set the characters size of a LPP_TrueTypeFont.
 *
 * @param width - character width.
 *
 * @param height - character height.
 *
 * @param dpiX - dpiX.
 *
 * @param dpiY - dpiY.
 *
 * @param f - the font to set characters size.
 *
 * @return the font size.
 *
 */
u32 LPPG_TTFSetCharSize(u32 width, u32 height, u32 dpiX, u32 dpiY, LPP_TrueTypeFont *f);

/**
 * Initialize the LPP Graphics library.
 *
 */
void LPPG_Init(void);

/**
 * Shutdown the LPP Graphics library.
 *
 */
void LPPG_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* __GRAPHICSL_H_ */
