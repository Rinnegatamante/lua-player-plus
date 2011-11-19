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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspgu.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspgum.h>
#include "include/luaplayer.h"

#include "libs/graphics/graphics.h"
#include "libs/intraFont/intraFont.h"

static const void* theScreen;
static Image theScreenImage;

UserdataStubs(Color, Color);

//Wait for Vsync
static int lua_WaitVblankStart(lua_State *L)
{
	int argc = lua_gettop(L), t = 0;
	if (argc != 0 && argc != 1 && argc != 2)
	{
		return luaL_error(L, "Screen.waitVblankStart() takes 0, 1 or 2 arguments only");
	}
	
	if (argc) t = lua_type(L, 1);
	if (argc == 0 || t != LUA_TNUMBER) 
	{
		sceDisplayWaitVblankStart();
	} 
	else 
	{
		int count = (t == LUA_TNUMBER)?luaL_checkint(L, 1):luaL_checkint(L, 2);
		int i;
		for (i = 0; i < count; i++) sceDisplayWaitVblankStart();
	}
	
	return 0;
}

static int lua_FlipScreen(lua_State *L)
{
	flipScreen();
	
	return 0;
}

/* // old intraFont functions
// Colors
enum colors {
	RED =	0xFF0000FF,
	GREEN =	0xFF00FF00,
	BLUE =	0xFFFF0000,
	WHITE =	0xFFFFFFFF,
	LITEGRAY = 0xFFBFBFBF,
	GRAY =  0xFF7F7F7F,
	DARKGRAY = 0xFF3F3F3F,		
	BLACK = 0xFF000000,
	PURPLE = 0xFF800080,
	YELLOW = 0xFFFF00FF,
	ORANGE = 0xFF0066FF,
	TRANSPARENT = 0x7FFFFFFF,
};


intraFont *font;

//Load a font using IntraFont
static int lua_IntraFontLoad(lua_State *L)
{

	int argc = lua_gettop(L);
	if (argc != 1)
	{
		return luaL_error(L, "IntraFont.load(fileName) takes 1 argument (the filename & path");
	}
	
	const char *path = luaL_checkstring(L, 1);
	
	char userFont[512];
	getcwd(userFont, 256);
	strcat(userFont, "/");
	strcat(userFont, path);
	
	font = intraFontLoad(userFont, 0);
	
	if (intraFontLoad(userFont, 0) == 0)
	{
		return luaL_error(L, "Could not load Font, please check your path, and ensure the file exists");
	}
	
	return 1;
}

//Print text using IntraFont
static int lua_IntraFontPrint(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc !=1 && argc != 2 && argc != 3 && argc != 4 && argc != 5 && argc != 6 && argc != 7)
	{	
		return luaL_error(L, "IntraFont.print() takes 7 arguments : font, x, y, textSize, foreGround color, backGroundColor, text");
	}

	const char *path = luaL_checkstring(L, 1);
	char userFont[512];
	getcwd(userFont, 256);
	strcat(userFont, "/");
	strcat(userFont, path);

	pspDebugScreenSetXY(15,2);
	
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);
	float size = lua_tonumber(L, 4);
	unsigned int textColorCheck1 = lua_tonumber(L, 5);
	unsigned int textColorCheck2 = lua_tonumber(L, 6);
	const char *text = luaL_checkstring(L, 7);
	
	unsigned int textColor1 = WHITE;
	unsigned int textColor2 = BLACK;
	
	//Text colors
	switch(textColorCheck1)
	{
		case 0 : 
			textColor1 = BLACK;
		break;
		
		case 1 :
			textColor1 = RED;
		break;
		
		case 2 :
			textColor1 = BLUE;
		break;
		
		case 3 : 
			textColor1 = WHITE;
		break;
		
		case 4 :
			textColor1 = LITEGRAY;
		break;
		
		case 5 : 
			textColor1 = GRAY;
		break;
		
		case 6 :
			textColor1 = DARKGRAY;
		break;
		
		case 7 : 
			textColor1 = PURPLE;
		break;
		
		case 8 :
			textColor1 = YELLOW;
		break;
		
		case 9 :
			textColor1 = ORANGE;
		break;
		
		case 10 :
			textColor1 = TRANSPARENT;
		break;
	}
	
	//Text colors 2
	switch(textColorCheck2)
	{
		case 0 : 
			textColor2 = BLACK;
		break;
		
		case 1 :
			textColor2 = RED;
		break;
		
		case 2 :
			textColor2 = BLUE;
		break;
		
		case 3 : 
			textColor2 = WHITE;
		break;
		
		case 4 :
			textColor2 = LITEGRAY;
		break;
		
		case 5 : 
			textColor2 = GRAY;
		break;
		
		case 6 :
			textColor2 = DARKGRAY;
		break;
		
		case 7 : 
			textColor2 = PURPLE;
		break;
		
		case 8 :
			textColor2 = YELLOW;
		break;
		
		case 9 :
			textColor2 = ORANGE;
		break;
		
		case 10 :
			textColor2 = TRANSPARENT;
		break;
	}
	
	intraFontSetStyle(font, size, textColor1, textColor2, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, x, y, text);
	
	return 1;
}

//Unload a font using IntraFont
static int lua_IntraFontUnload(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0)
	{
		return luaL_error(L, "IntraFont.unLoad() takes no arguments");
	}

	intraFontUnload(font);	
	
	return 1;
}

//Register IntraFont Functions
static const luaL_reg Font_functions[] = {
	{"load", 		lua_IntraFontLoad},
	{"print", 		lua_IntraFontPrint},
	{"unLoad", 		lua_IntraFontUnload},
	{0,0}
};
*/

UserdataStubs(Image, Image*);

//Create an Empty Image
static int lua_ImageCreateEmpty(lua_State *L)
{
	if (lua_gettop(L) != 2)
	{
		return luaL_error(L, "Image.createEmpty(w, h) takes two arguments.");
	}
	
	unsigned int w = luaL_checkint(L, 1);
	unsigned int h = luaL_checkint(L, 2);
	
	if (w > 512 || h > 512) return luaL_error(L, "invalid size");
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	Image* image = createImage(w, h);
	
	if (!image) return luaL_error(L, "Can't create image");
	Image** luaImage = pushImage(L);
	*luaImage = image;
	
	return 1;
}

//Load an image from file (png and jpeg supported)
static int lua_ImageLoad(lua_State *L) 
{
	if (lua_gettop(L) != 1)
	{	
		return luaL_error(L, "Image.load(filename) takes one argument (the filename and path)");
	}
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	Image* image = loadImage(luaL_checkstring(L, 1));
	
	if(!image) return luaL_error(L, "Image.load: Error loading image.");
	Image** luaImage = pushImage(L);
	*luaImage = image;
	
	return 1;
}

#define SETDEST \
	Image *dest = NULL; \
	{ \
		int type = lua_type(L, 1); \
		if (type == LUA_TTABLE) lua_remove(L, 1); \
		else if (type == LUA_TUSERDATA) { \
			dest = *toImage(L, 1); \
			lua_remove(L, 1); \
		} else return luaL_error(L, "Method must be called with a colon!"); \
	}

//Blit an image to an image or the screen
static int lua_ImageBlit(lua_State *L) 
{
	int argc = lua_gettop(L);
	
	if ((argc < 3) | (argc > 5 && argc < 7) | (argc > 7 && argc < 9)) 
	{
		return luaL_error(L, "image:blit() takes 3, 4, 6 or 8 arguments \n\n3 arguments : (x, y, image) \n4 arguments : (x, y, image, alpha) \n6 arguments : (x, y, image, alpha, source x, source y) \n 8 arguments : (x, y, image, alpha, source x, source y, width, height)");
	}
	 
	if (argc==6 || argc== 10) lua_pop(L, 1);
	
	SETDEST
		
	int dx = luaL_checkint(L, 1);
	int dy = luaL_checkint(L, 2);
	
	Image* source;
	if (lua_topointer(L, 3) == theScreen) 
	{
		theScreenImage.data = getVramDrawBuffer();
		source = &theScreenImage;
	} 
	else 
	{
		source = *toImage(L, 3);
	}
	
	bool rect = (argc == 9 || argc == 10);
	bool alpha = (argc == 5);
	int setAlpha = alpha? luaL_checkint(L, 4) : 255;
	int sx = rect? luaL_checkint(L, 5) : 0;
	int sy = rect? luaL_checkint(L, 6) : 0;
	int width = rect? luaL_checkint(L, 7) : source->imageWidth;
	int height = rect? luaL_checkint(L, 8) : source->imageHeight;
	
	if (dest)
		blitAlphaImageToImage(sx, sy, width, height, source, dx, dy, dest);
	else
		blitAlphaImageToScreen(sx, sy, width, height, source, dx, dy, setAlpha);
	
	return 0;
}
	
//Clears an image or the screen to a color (Fast, but may not work with debug text)
static int lua_ImageClear(lua_State *L) 
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2)
	{
		return luaL_error(L, "Image:clear([color]) takes zero or one argument.");
	}
	
	Color color = (argc==2)?*toColor(L, 2):0;

	SETDEST
	if(dest)
		clearImage(color, dest);
	else
		clearScreen(color);
	return 0;
}

//Clears an image or the screen to a color (slow, but works with debug text)
static int lua_ImageSlowClear(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1 && argc != 2)
	{
		return luaL_error(L, "Image:clear([color]) takes zero or one argument.");
	}
	
	Color color = (argc==2)?*toColor(L, 2):0;

	SETDEST
	if(dest)
		clearImage(color, dest);
	else
		slowClearScreen(color);
	return 0;
}

//Fills an image or the screen with a Rectangle
static int lua_ImageFillRect(lua_State *L) 
{
	int argc = lua_gettop(L);
	
	if (argc != 5 && argc != 6)
	{
		return luaL_error(L, "Image.fillRect() takes a minimum of 4 arguments");
	}
	
	SETDEST

	int x0 = luaL_checkint(L, 1);
	int y0 = luaL_checkint(L, 2);
	unsigned int width = luaL_checkint(L, 3);
	unsigned int height = luaL_checkint(L, 4);
	Color color = (argc==6)?*toColor(L, 5):0;
	
	if (width <= 0 || height <= 0) return 0;
	
	if (x0 < 0) 
	{
		width += x0;
		if (width <= 0) return 0;
		x0 = 0;
	}
	if (y0 < 0) 
	{
		height += y0;
		if (height <= 0) return 0;
		y0 = 0;
	}
	if (!dest) 
	{
		if (width <= 0 || height <= 0) return 0;
		if (x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT) return 0;
		if (x0 + width >= SCREEN_WIDTH)
		{
			width = SCREEN_WIDTH - x0;
			if (width <= 0) return 0;
		}
		if (y0 + height >= SCREEN_HEIGHT) 
		{
			height = SCREEN_HEIGHT - y0;
			if (height <= 0) return 0;
		}
		fillScreenRect(color, x0, y0, width, height);
	} 
	else
	{
		if (x0 >= dest->imageWidth || y0 >= dest->imageHeight) return 0;
		if (x0 + width >= dest->imageWidth) 
		{
			width = dest->imageWidth - x0;
			if (width <= 0) return 0;
		}
		if (y0 + height >= dest->imageHeight)
		{
			height = dest->imageHeight - y0;
			if (height <= 0) return 0;
		}
		fillImageRect(color, x0, y0, width, height, dest);
	}
	
	return 0;
}

//Draws a line
static int lua_ImageDrawLine(lua_State *L) 
{
	int argc = lua_gettop(L); 
	
	if (argc != 5 && argc != 6) 
	{
		return luaL_error(L, "Image.drawLine() takes a minimum of 4 arguments"); 
	}
	
	SETDEST
	unsigned int x0 = luaL_checkint(L, 1); 
	unsigned int y0 = luaL_checkint(L, 2); 
	unsigned int x1 = luaL_checkint(L, 3); 
	unsigned int y1 = luaL_checkint(L, 4); 
	Color color = (argc==6) ? *toColor(L, 5) : 0;
	
	// TODO: better clipping
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	
	if (!dest) 
	{
		if (x0 >= SCREEN_WIDTH) x0 = SCREEN_WIDTH - 1;
		if (x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
		if (y0 >= SCREEN_HEIGHT) y0 = SCREEN_HEIGHT - 1;
		if (y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
		drawLineScreen(x0, y0, x1, y1, color);
	} else {

		if (x0 >= dest->imageWidth) x0 = dest->imageWidth - 1;
		if (x1 >= dest->imageWidth) x1 = dest->imageWidth - 1;
		if (y0 >= dest->imageHeight) y0 = dest->imageHeight - 1;
		if (y1 >= dest->imageHeight) y1 = dest->imageHeight - 1;
		drawLineImage(x0, y0, x1, y1, color, dest);
	}
	return 0;
}

//Get the color from a pixel
static int lua_ImagePixel(lua_State *L) 
{
	int argc = lua_gettop(L);
	if(argc != 3 && argc != 4)
	{
		return luaL_error(L, "Image:pixel(x, y, [color]) takes two or three arguments, and must be called with a colon.");
	}
	
	SETDEST
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	Color color = (argc == 4)?*toColor(L, 3):0;
	
	if(dest) 
	{
		if (x >= 0 && y >= 0 && x < dest->imageWidth && y < dest->imageHeight) 
		{
			if(argc==3) 
			{
				*pushColor(L) = getPixelImage(x, y, dest);
				return 1;
			} 
			else 
			{
				putPixelImage(color, x, y, dest);
				return 0;
			}
		}
	} 
	else 
	{
		if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) 
		{
			if(argc==3) 
			{
				*pushColor(L) = getPixelScreen(x, y);
				return 1;
			} 
			else 
			{
				putPixelScreen(color, x, y);
				return 0;
			}
		}
	}

	return luaL_error(L, "Image:pixel() : An argument was incorrect.");
}

//Print text to the sceen or image
static int lua_ImagePrint(lua_State *L) 
{
	int argc = lua_gettop(L);
	if (argc != 4 && argc != 5)
	{
		return luaL_error(L, "image:print takes a minimum of 4 arguments");
	}
	
	SETDEST
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	const char* text = luaL_checkstring(L, 3);
	Color color = (argc == 5)?*toColor(L, 4):0xFF000000;
	
	if (!dest) 
	{
		printTextScreen(x, y, text, color);
	} 
	else
	{
		printTextImage(x, y, text, color, dest);
	}
	
	return 0;
}

//Returns an images width
static int lua_ImageWidth(lua_State *L) 
{
	int argc = lua_gettop(L);
	if(argc != 1)
	{
		return luaL_error(L, "Image:width() must be called with a colon, and takes no arguments");
	}
	
	SETDEST
	if(dest) lua_pushnumber(L, dest->imageWidth);
	else lua_pushnumber(L, SCREEN_WIDTH);
	
	return 1;
}

//Returns an image's height
static int lua_ImageHeight(lua_State *L) 
{
	int argc = lua_gettop(L);
	if(argc != 1)
	{
		return luaL_error(L, "Image:width() must be called with a colon, and takes no arguments");
	}
	
	SETDEST
	if(dest) lua_pushnumber(L, dest->imageHeight);
	else lua_pushnumber(L, SCREEN_HEIGHT);
	
	return 1;
}

//Saves an image to file
static int lua_ImageSave(lua_State *L) 
{
	if (lua_gettop(L) != 2)
	{
		return luaL_error(L, "image:save(filename) takes 1 argument (the filename and path)");
	}
	
	const char *filename = luaL_checkstring(L, 2);
	
	SETDEST
	if (dest) 
	{
		saveImage(filename, dest->data, dest->imageWidth, dest->imageHeight, dest->textureWidth, 1);
	} 
	else 
	{
		saveImage(filename, getVramDisplayBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE, 0);
	}
	return 0;
}

static int lua_ImageToString (lua_State *L) 
{
	lua_ImageWidth(L);
	int w = luaL_checkint(L, 2); lua_pop(L, 1);
	lua_ImageHeight(L);
	int h = luaL_checkint(L, 2); lua_pop(L, 1);

	char buff[32];
	sprintf(buff, "%p", *toImage(L, 1));
	lua_pushfstring(L, "Image (%s) [%d, %d]", buff, w, h);
	return 1;
}

//Free an Image
static int lua_ImageFree(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc !=1)
	{	
		return luaL_error(L, "Image.free() takes 1 arguments (the image to free)");
	}

	lua_gc(L, LUA_GCCOLLECT, 0);			
	freeImage(*toImage(L, 1));
	
	return 0;
}

//Swizzle an Image
static int lua_ImageSwizzle(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc !=1)
	{	
		return luaL_error(L, "Image.swizzle() takes 1 argument (the image to swizzle)");
	}
	
	swizzle(*toImage(L, 1));
	
	return 0;
}

//UnSwizzle an Image
static int lua_ImageUnSwizzle(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc !=1)
	{	
		return luaL_error(L, "Image.unSwizzle() takes 1 argument (the image to unSwizzle)");
	}
	
	unSwizzleFast(*toImage(L, 1));
	
	return 0;
}

//Move an Image to Vram
static int lua_ImageToVram(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc !=1)
	{	
		return luaL_error(L, "Image.unSwizzle() takes 1 argument (the image to unSwizzle)");
	}
	
	moveImageToVram(*toImage(L, 1));
	
	return 1;
}

//Register our Image Functions
static const luaL_reg Image_methods[] = {
	{"swizzle",			lua_ImageSwizzle},
	{"unSwizzle",		lua_ImageUnSwizzle},
	{"toVram",			lua_ImageToVram},
	{"free", 			lua_ImageFree},
	{"createEmpty", 	lua_ImageCreateEmpty},
	{"load", 			lua_ImageLoad},
	{"blit", 			lua_ImageBlit},
	{"clear", 			lua_ImageClear},
	{"slowClear", 		lua_ImageSlowClear},
	{"fillRect", 		lua_ImageFillRect},
	{"drawLine", 		lua_ImageDrawLine},
	{"pixel", 			lua_ImagePixel},
	{"print", 			lua_ImagePrint},
	{"width", 			lua_ImageWidth},
	{"height", 			lua_ImageHeight},
	{"save", 			lua_ImageSave},
	{0,0}
};

static const luaL_reg Image_meta[] = {
	{"__tostring", lua_ImageToString},
	{0,0}
};

UserdataRegister(Image, Image_methods, Image_meta)

/******************************************************************************
 ** intraFont *******************************************************************
 *******************************************************************************/
 
UserdataStubs(IntraFont, intraFont*);

// Init intraFont
static int lua_intraFontInit(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0) return luaL_error(L, "intraFont.init() takes no arguments");
	
	int ret = intraFontInit();
	
	lua_pushnumber(L, ret);
	
	return 0;
}

// Shutdown intraFont
static int lua_intraFontShutdown(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0) return luaL_error(L, "intraFont.shutdown() takes no arguments");
	
	intraFontShutdown();
	
	return 0;
}

// Load a pgf font
static int lua_intraFontLoad(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) return luaL_error(L, "intraFont.load() takes 2 arguments");
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	int options = luaL_checkint(L, 2);
	intraFont* ifont = intraFontLoad(luaL_checkstring(L, 1), options);
	
	if(!ifont) return luaL_error(L, "intraFont.load: Error loading font.");
	intraFont** luaintraFont = pushIntraFont(L);
	*luaintraFont = ifont;
	
	return 1;
}

// Free the specified font.
static int lua_intraFontUnload(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "intraFont.unload() takes 0 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	
	intraFontUnload(ifont);
	
	return 0;
}

// Activate the specified font
static int lua_intraFontActivate(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "intraFont.activate() takes 0 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	
	intraFontActivate(ifont);
	
	return 0;
}

// Set font style
static int lua_intraFontSetStyle(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5) return luaL_error(L, "intraFont.setStyle() takes 4 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float size = luaL_checknumber(L, 2);
	Color color = *toColor(L, 3);
	Color shadowColor = *toColor(L, 4);
	int options = luaL_checkint(L, 5);
	
	intraFontSetStyle(ifont, size, color, shadowColor, options);
	
	return 0;
}

// Set type of string encoding to be used in intraFontPrint[f]
static int lua_intraFontSetEncoding(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) return luaL_error(L, "intraFont.setEncoding() takes 1 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	int options = luaL_checkint(L, 2);
	
	intraFontSetEncoding(ifont, options);
	
	return 0;
}

// Set alternative font
static int lua_intraFontSetAltFont(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) return luaL_error(L, "intraFont.setAltFont() takes 1 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	intraFont* alt_ifont = *toIntraFont(L, 2);
	
	intraFontSetAltFont(ifont, alt_ifont);
	
	return 0;
}

// Measure a length of text if it were to be drawn
static int lua_intraFontMeasureText(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) return luaL_error(L, "intraFont.measureText() takes 1 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	
	float ret = intraFontMeasureText(ifont, luaL_checkstring(L, 2));
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Measure a length of text if it were to be drawn
static int lua_intraFontMeasureTextEx(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 3) return luaL_error(L, "intraFont.measureTextEx() takes 2 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	int len = luaL_checkint(L, 3);
	
	float ret = intraFontMeasureTextEx(ifont, luaL_checkstring(L, 2), len);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Measure a length of UCS-2 encoded text if it were to be drawn
static int lua_intraFontMeasureTextUCS2(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) return luaL_error(L, "intraFont.measureTextUCS2() takes 1 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 2) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");

	intraFont* ifont = *toIntraFont(L, 1);
	
	int n = luaL_getn(L, 2);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 2, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	float ret = intraFontMeasureTextUCS2(ifont, text);
	
	lua_pushnumber(L, ret);
	
	free(text);
	
	return 1;
}

// Measure a length of UCS-2 encoded text if it were to be drawn
static int lua_intraFontMeasureTextUCS2Ex(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 3) return luaL_error(L, "intraFont.measureTextUCS2Ex() takes 2 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 2) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");

	intraFont* ifont = *toIntraFont(L, 1);
	int len = luaL_checkint(L, 3);
	
	int n = luaL_getn(L, 2);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 2, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	float ret = intraFontMeasureTextUCS2Ex(ifont, text, len);
	
	lua_pushnumber(L, ret);
	
	free(text);
	
	return 1;
}

// Draw text 
static int lua_intraFontPrint(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 4) return luaL_error(L, "intraFont.print() takes 3 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	
	float ret = intraFontPrint(ifont, x, y, luaL_checkstring(L, 4));
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw text 
static int lua_intraFontPrintEx(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5) return luaL_error(L, "intraFont.printEx() takes 4 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	int len = luaL_checkint(L, 5);
	
	float ret = intraFontPrintEx(ifont, x, y, luaL_checkstring(L, 4), len);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw text 
static int lua_intraFontPrintColumn(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5) return luaL_error(L, "intraFont.printColumn() takes 4 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float w = luaL_checknumber(L, 4);
	
	float ret = intraFontPrintColumn(ifont, x, y, w, luaL_checkstring(L, 5));
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw text 
static int lua_intraFontPrintColumnEx(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 6) return luaL_error(L, "intraFont.printColumnEx() takes 5 arguments, and it must be called from an instance with a colon.");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float w = luaL_checknumber(L, 4);
	int len = luaL_checkint(L, 6);
	
	float ret = intraFontPrintColumnEx(ifont, x, y, w, luaL_checkstring(L, 5), len);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw UCS-2 encoded text
static int lua_intraFontPrintUCS2(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 4) return luaL_error(L, "intraFont.printUCS2() takes 3 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 4) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");

	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	
	int n = luaL_getn(L, 4);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 4, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	
	float ret = intraFontPrintUCS2(ifont, x, y, text);
	
	free(text);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw UCS-2 encoded text
static int lua_intraFontPrintUCS2Ex(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5) return luaL_error(L, "intraFont.printUCS2Ex() takes 4 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 4) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");

	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	int len = luaL_checkint(L, 5);
	
	int n = luaL_getn(L, 4);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 4, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	
	float ret = intraFontPrintUCS2Ex(ifont, x, y, text, len);
	
	free(text);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw UCS-2 encoded text
static int lua_intraFontPrintColumnUCS2(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5) return luaL_error(L, "intraFont.printColumnUCS2() takes 4 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 5) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float w = luaL_checknumber(L, 4);
	
	int n = luaL_getn(L, 5);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 5, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	float ret = intraFontPrintColumnUCS2(ifont, x, y, w, text);
	
	free(text);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

// Draw UCS-2 encoded text
static int lua_intraFontPrintColumnUCS2Ex(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 6) return luaL_error(L, "intraFont.printColumnUCS2Ex() takes 5 arguments, and it must be called from an instance with a colon.");
	if (lua_type(L, 5) != LUA_TTABLE) return luaL_error(L, "UCS-2 table missing");
	
	intraFont* ifont = *toIntraFont(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float w = luaL_checknumber(L, 4);
	int len = luaL_checkint(L, 6);
	
	int n = luaL_getn(L, 5);
	unsigned short *text = (unsigned short*)memalign(16, (n+1) * sizeof(unsigned short));
	
	int i;
	for (i=0; i<n; i++)
	{
		lua_rawgeti(L, 5, i+1);
		text[i] = (unsigned short)luaL_checknumber(L, -1);
	}	
	
	text[n] = 0;
	
	float ret = intraFontPrintColumnUCS2Ex(ifont, x, y, w, text, len);
	
	free(text);
	
	lua_pushnumber(L, ret);
	
	return 1;
}

//Register our intraFont Functions
static const luaL_reg intraFont_methods[] = {
	// {"init",				lua_intraFontInit},	
	// {"shutdown",			lua_intraFontShutdown},
	{"load",				lua_intraFontLoad},
	{"unload",				lua_intraFontUnload},
	{"activate",			lua_intraFontActivate},
	{"setStyle",			lua_intraFontSetStyle},
	{"setEncoding",			lua_intraFontSetEncoding},
	{"setAltFont",			lua_intraFontSetAltFont},
	{"measureText",			lua_intraFontMeasureText},
	{"measureTextEx",		lua_intraFontMeasureTextEx},
	{"measureTextUCS2",		lua_intraFontMeasureTextUCS2},
	{"measureTextUCS2Ex",	lua_intraFontMeasureTextUCS2Ex},
	{"print",				lua_intraFontPrint},
	{"printEx",				lua_intraFontPrintEx},
	{"printColumn",			lua_intraFontPrintColumn},
	{"printColumnEx",		lua_intraFontPrintColumnEx},
	{"printUCS2",			lua_intraFontPrintUCS2},
	{"printUCS2Ex",			lua_intraFontPrintUCS2Ex},
	{"printColumnUCS2",		lua_intraFontPrintColumnUCS2},
	{"printColumnUCS2Ex",	lua_intraFontPrintColumnUCS2Ex},
	{0,0}
};

static const luaL_reg intraFont_meta[] = {
	{0,0}
};

UserdataRegister(IntraFont, intraFont_methods, intraFont_meta);

static int Color_new (lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3 && argc != 4) return luaL_error(L, "Color.new(r, g, b, [a]) takes either three color arguments or three color arguments and an alpha value."); 
	
	Color *color = pushColor(L);
	
	unsigned r = CLAMP(luaL_checkint(L, 1), 0, 255); 
	unsigned g = CLAMP(luaL_checkint(L, 2), 0, 255); 
	unsigned b = CLAMP(luaL_checkint(L, 3), 0, 255);
	unsigned a;
	if (argc == 4) {
		a = CLAMP(luaL_checkint(L, 4), 0, 255);
	} else {
		a = 255;
	}

	//*color = ((b>>3)<<10) | ((g>>3)<<5) | (r>>3) | (a == 255 ? 0x8000 : 0);
	*color = a << 24 | b << 16 | g << 8 | r;
	
	return 1;
}

static int Color_colors (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "color:colors() takes no arguments, and it must be called from an instance with a colon.");
	Color color = *toColor(L, 1);
	int r = R(color); 
	int g = G(color);
	int b = B(color);
	int a = A(color);
	
	lua_newtable(L);
	lua_pushstring(L, "r"); lua_pushnumber(L, r); lua_settable(L, -3);
	lua_pushstring(L, "g"); lua_pushnumber(L, g); lua_settable(L, -3);
	lua_pushstring(L, "b"); lua_pushnumber(L, b); lua_settable(L, -3);
	lua_pushstring(L, "a"); lua_pushnumber(L, a); lua_settable(L, -3);

	return 1;
}

static int Color_tostring (lua_State *L) {
	Color_colors(L);
	lua_pushstring(L, "r"); lua_gettable(L, -2); int r = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "g"); lua_gettable(L, -2); int g = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "b"); lua_gettable(L, -2); int b = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "a"); lua_gettable(L, -2); int a = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pop(L, 1); // pop the table
	lua_pushfstring(L, "Color (r %d, g %d, b %d, a %d)", r, g, b, a);
	return 1;
}

static int Color_equal(lua_State *L) {
	Color a = *toColor(L, 1);
	Color b = *toColor(L, 2);
	lua_pushboolean(L, a == b);
	return 1;
}

static const luaL_reg Color_methods[] = {
	{"new", Color_new},
	{"colors", Color_colors},
	{0,0}
};

static const luaL_reg Color_meta[] = {
	{"__tostring", Color_tostring},
	{"__eq", Color_equal},
	{0,0}
};

UserdataRegister(Color, Color_methods, Color_meta)

static const luaL_reg Screen_functions[] = {
	{"flip", lua_FlipScreen},
	{"waitVblankStart", lua_WaitVblankStart},
	{0,0}
};

void luaGraphics_init(lua_State *L) {

	IntraFont_register(L);
	Image_register(L);
	Color_register(L);
	
	// luaL_openlib(L, "IntraFont", Font_functions, 0);
	luaL_openlib(L, "screen", Screen_functions, 0);
	luaL_openlib(L, "screen", Image_methods, 0); 	

	theScreen = lua_topointer(L, -1);
	theScreenImage.textureWidth = 512;
	theScreenImage.textureHeight = 512;
	theScreenImage.imageWidth = 480;
	theScreenImage.imageHeight = 272;
	
#define INTRAFONT_CONSTANT(name)\
	lua_pushstring(L, #name);\
	lua_pushnumber(L, INTRAFONT_##name);\
	lua_settable(L, -3);
	
	lua_pushstring(L, "IntraFont");
	lua_gettable(L, LUA_GLOBALSINDEX);
	
	INTRAFONT_CONSTANT(ADVANCE_H)
	INTRAFONT_CONSTANT(ADVANCE_V)
	INTRAFONT_CONSTANT(ALIGN_LEFT)
	INTRAFONT_CONSTANT(ALIGN_RIGHT)
	INTRAFONT_CONSTANT(ALIGN_CENTER)
	INTRAFONT_CONSTANT(ALIGN_FULL)
	INTRAFONT_CONSTANT(SCROLL_LEFT)
	INTRAFONT_CONSTANT(SCROLL_SEESAW)
	INTRAFONT_CONSTANT(SCROLL_RIGHT)
	INTRAFONT_CONSTANT(SCROLL_THROUGH)
	INTRAFONT_CONSTANT(WIDTH_VAR)
	INTRAFONT_CONSTANT(WIDTH_FIX)
	INTRAFONT_CONSTANT(ACTIVE)
	INTRAFONT_CONSTANT(CACHE_MED)
	INTRAFONT_CONSTANT(CACHE_LARGE)
	INTRAFONT_CONSTANT(CACHE_ASCII)
	INTRAFONT_CONSTANT(CACHE_ALL)
	INTRAFONT_CONSTANT(STRING_ASCII)
	INTRAFONT_CONSTANT(STRING_CP437)
	INTRAFONT_CONSTANT(STRING_CP850)
	INTRAFONT_CONSTANT(STRING_CP850)
	INTRAFONT_CONSTANT(STRING_CP866)
	INTRAFONT_CONSTANT(STRING_SJIS)
	INTRAFONT_CONSTANT(STRING_GBK)
	INTRAFONT_CONSTANT(STRING_KOR)
	INTRAFONT_CONSTANT(STRING_BIG5)
	INTRAFONT_CONSTANT(STRING_CP1251)
	INTRAFONT_CONSTANT(STRING_CP1252)
	INTRAFONT_CONSTANT(STRING_UTF8)
	
}
