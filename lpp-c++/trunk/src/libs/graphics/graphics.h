/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for the Swizzle/UnSwizzle code (taken from oslib). 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Benhur for IntraFont.
 * Thanks to Jono for the moveToVram code.
 * Thanks to Raphael for the Vram manager code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 *
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <psptypes.h>

#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))

typedef struct
{
	int textureWidth;  // the real width of data, 2^n with n>=0
	int textureHeight;  // the real height of data, 2^n with n>=0
	int imageWidth;  // the image width
	int imageHeight;
	int swizzled; //If the image is swizzled or not
	int inVram; //If the image is in Vram or not
	u16 width;
	u16 height;
	u16 stride;
	u32 size;
	u8 bits;
	Color* data;
} Image;

/**
 * Swizzle an image.
 *
 * @param Image - Image to swizzle
 */
void swizzle(Image *img);

/**
 * UnSwizzle an image.
 *
 * @param Image - Image to Unswizzle
 */
void unSwizzleFast(Image *img);

/**
 * Move an image to Vram.
 *
 * @param Iamge - Image to move to Vram
 */
void moveImageToVram(Image* img);

/**
 * Load a PNG or JPEG image (depends on the filename suffix).
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
Image* loadImage(const char* filename);

//extern void swizzle_fast(unsigned char *dest, unsigned char *source, int width, int height);
//extern void swizzle( Image* img );
//extern void un

/**
 * Load a PNG image.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
Image* loadPngImage(const char* filename);

/**
 * Load a JPEG image.
 *
 * @pre filename != NULL
 * @param filename - filename of the JPEG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
Image* loadJpegImage(const char* filename);

/**
 * Load a PNG or JPEG image from in-memory data (with auto detection of the format).
 *
 * @pre data != NULL && len >0
 * @param data - the in-memory bytes of the image
 * @param len - the number of valid bytes at the data pointer
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
Image* loadImageFromMemory(const unsigned char* data, int len, const char *filename);

/**
 * Blit a rectangle part of an image to another image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= destination->width && dy + height <= destination->height
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 * @param destination - pointer to Image struct of the destination image
 */
void blitImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination);

/**
 * Blit a rectangle part of an image to screen.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= SCREEN_WIDTH && dy + height <= SCREEN_HEIGHT
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 */
void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);

/**
 * Blit a rectangle part of an image to another image without alpha pixels in source image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= destination->width && dy + height <= destination->height
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 * @param destination - pointer to Image struct of the destination image
 */
void blitAlphaImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination);

/**
 * Blit a rectangle part of an image to screen without alpha pixels in source image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= SCREEN_WIDTH && dy + height <= SCREEN_HEIGHT
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 */
void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy, u32 alpha);

/**
 * Blit a rectangle part of an image to screen with alpha pixels in source image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= SCREEN_WIDTH && dy + height <= SCREEN_HEIGHT
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 */


/**
 * Create an empty image.
 *
 * @pre width > 0 && height > 0 && width <= 512 && height <= 512
 * @param width - width of the new image
 * @param height - height of the new image
 * @return pointer to a new allocated Image struct, all pixels initialized to color 0, or NULL on failure
 */
Image* createImage(int width, int height);

/**
 * Frees an allocated image.
 *
 * @pre image != null
 * @param image a pointer to an image struct
 */
void freeImage(Image* image);

/**
 * Initialize all pixels of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param image - image to clear
 */
void clearImage(Color color, Image* image);

/**
 * Initialize all pixels of the screen with a color. Fast but not as effective as slowClearScreen
 *
 * @param color - new color for the pixels
 */
void clearScreen(Color color);

/**
 * Initialize all pixels of the screen with a color.
 *
 * @param color - new color for the pixels
 */
void slowClearScreen(Color color);

/**
 * Fill a rectangle of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param x0 - left position of rectangle in image
 * @param y0 - top position of rectangle in image
 * @param width - width of rectangle in image
 * @param height - height of rectangle in image
 * @param image - image
 */
void fillImageRect(Color color, int x0, int y0, int width, int height, Image* image);

/**
 * Fill a rectangle of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param x0 - left position of rectangle in image
 * @param y0 - top position of rectangle in image
 * @param width - width of rectangle in image
 * @param height - height of rectangle in image
 */
void fillScreenRect(Color color, int x0, int y0, int width, int height);

/**
 * Set a pixel on screen to the specified color.
 *
 * @pre x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT
 * @param color - new color for the pixels
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 */
void putPixelScreen(Color color, int x, int y);

/**
 * Set a pixel in an image to the specified color.
 *
 * @pre x >= 0 && x < image->imageWidth && y >= 0 && y < image->imageHeight && image != NULL
 * @param color - new color for the pixels
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 */
void putPixelImage(Color color, int x, int y, Image* image);

/**
 * Get the color of a pixel on screen.
 *
 * @pre x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 * @return the color of the pixel
 */
Color getPixelScreen(int x, int y);

/**
 * Get the color of a pixel of an image.
 *
 * @pre x >= 0 && x < image->imageWidth && y >= 0 && y < image->imageHeight && image != NULL
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 * @return the color of the pixel
 */
Color getPixelImage(int x, int y, Image* image);

/**
 * Print a text (pixels out of the screen or image are clipped).
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 */
void printTextScreen(int x, int y, const char* text, Color color);

/**
 * Print a text (pixels out of the screen or image are clipped).
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 * @param image - image
 */
void printTextImage(int x, int y, const char* text, Color color, Image* image);

/**
 * Save an image or the screen in PNG or JPEG format (depends on the filename suffix).
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 * @param saveAlpha - if 0, image is saved without alpha channel
 */
void saveImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);

/**
 * Save an image or the screen in PNG format.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 * @param saveAlpha - if 0, image is saved without alpha channel
 */
void savePngImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);

/**
 * Save an image or the screen in JPEG format.
 *
 * @pre filename != NULL
 * @param filename - filename of the JPEG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 */
void saveJpegImage(const char* filename, Color* data, int width, int height, int lineSize);

/**
 * Exchange display buffer and drawing buffer.
 */
void flipScreen();

/**
 * Initialize the graphics.
 */
void initGraphics();

/**
 * Disable graphics, used for debug text output.
 */
void disableGraphics();

/**
 * Draw a line to screen.
 *
 * @pre x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT &&
 *      x1 >= 0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT
 * @param x0 - x line start position
 * @param y0 - y line start position
 * @param x1 - x line end position
 * @param y1 - y line end position
 */
void drawLineScreen(int x0, int y0, int x1, int y1, Color color);

/**
 * Draw a line to screen.
 *
 * @pre x0 >= 0 && x0 < image->imageWidth && y0 >= 0 && y0 < image->imageHeight &&
 *      x1 >= 0 && x1 < image->imageWidth && y1 >= 0 && y1 < image->imageHeight
 * @param x0 - x line start position
 * @param y0 - y line start position
 * @param x1 - x line end position
 * @param y1 - y line end position
 */
void drawLineImage(int x0, int y0, int x1, int y1, Color color, Image* image);

/**
 * Get the current draw buffer for fast unchecked access.
 *
 * @return the start address of the current draw buffer
 */
Color* getVramDrawBuffer();

/**
 * Get the current display buffer for fast unchecked access.
 *
 * @return the start address of the current display buffer
 */
Color* getVramDisplayBuffer();

void guStart();
void guEnd();

#endif
