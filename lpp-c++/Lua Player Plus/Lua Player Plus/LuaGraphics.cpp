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
#- For help using LuaPlayerPlus, code help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php --#
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "LuaPlayer.h"
#include "Common.h"

/* Buffers */
#include "Vera.cpp"
#include "VeraMono.cpp"

#define RGBA_A(u) (((u) >> 24) & 0x000000FF)  
#define RGBA_R(u) (((u) >> 16) & 0x000000FF)
#define RGBA_G(u) (((u) >> 8)  & 0x000000FF)
#define RGBA_B(u) (((u) >> 0)  & 0x000000FF)

unsigned int ToColor(lua_State* L, int a)
{
	const char *Colorstr = luaL_checkstring(L, a);
	unsigned int color;
	sscanf(Colorstr, "%x", &color);
	return(color);
}

FT_Library _FtLibrary;

struct Font {
	char *name;
	FT_Face face;
	u8 *data;
};

enum colors {
	RED =   0xFF0000FF,
	GREEN = 0xFF00FF00,
	BLUE =  0xFFFF0000,
	WHITE = 0xFFFFFFFF,
	LITEGRAY = 0xFFBFBFBF,
	GRAY =  0xFF7F7F7F,
	DARKGRAY = 0xFF3F3F3F,
	BLACK = 0xFF000000,
	PURPLE = 0xFF800080,
	YELLOW = 0xFFFF00FF,
	ORANGE = 0xFF0066FF,
	TRANSPARENT = 0x7FFFFFFF,
};

UserdataStubs(Font, Font*)
UserdataStubs(Image, OSL_IMAGE*)
UserdataStubs(intraFont, OSL_FONT*)

/******************************************************************************
 ** intraFont *******************************************************************
 *******************************************************************************/

static int IntraFont_Load(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "intraFont.load(filename) takes 1 argument.");
	const char *filename = luaL_checkstring(L, 1);
	OSL_FONT *ifont = oslLoadFontFile(filename);
	if(ifont == NULL) return luaL_error(L, "Error in loading font '%s'.", filename);
	OSL_FONT **luaifont = pushintraFont(L);
	*luaifont = ifont;
	return(1);
}

static int IntraFont_Unload(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "IntraFont:unload() takes no arguments and must be called with a colon.");
	oslDeleteFont(*tointraFont(L, 1));
	return(1);
}

static int IntraFont_Print(lua_State *L)
{
	if (lua_gettop(L) != 7)
		return luaL_error(L, "IntraFont:print(x, y, textSize, foreColor, BackGrounColor, text) takes 6 arguments and must be called with a colon.");
	unsigned int textcolorCheck1 = lua_tonumber(L, 5);
	unsigned int textcolorCheck2 = lua_tonumber(L, 6);
	float size = lua_tonumber(L, 4);
	unsigned int color1 = WHITE;
	unsigned int color2 = BLACK;

	switch(textcolorCheck1)
	{
	case 0 : color1 = BLACK; break;
	case 1 : color1 = RED; break;
	case 2 : color1 = BLUE; break;
	case 3 : color1 = WHITE; break;
	case 4 : color1 = LITEGRAY; break;
	case 5 : color1 = GRAY; break;
	case 6 : color1 = DARKGRAY; break;
	case 7 : color1 = PURPLE; break;
	case 8 : color1 = YELLOW; break;
	case 9 : color1 = ORANGE; break;
	case 10 : color1 = TRANSPARENT; break;
	}

	switch(textcolorCheck2)
	{
	case 0 : color2 = BLACK; break;
	case 1 : color2 = RED; break;
	case 2 : color2 = BLUE; break;
	case 3 : color2 = WHITE; break;
	case 4 : color2 = LITEGRAY; break;
	case 5 : color2 = GRAY; break;
	case 6 : color2 = DARKGRAY; break;
	case 7 : color2 = PURPLE; break;
	case 8 : color2 = YELLOW; break;
	case 9 : color2 = ORANGE; break;
	case 10 : color2 = TRANSPARENT; break;
	}
	oslIntraFontSetStyle(*tointraFont(L, 1), size, color1, color2, INTRAFONT_ALIGN_LEFT);
	oslSetFont(*tointraFont(L, 1));
	oslDrawString(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 7));
	return(1);
}

static int IntraFont_Shutdown(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "intraFont.shutdown() takes no arguments.");
	oslIntraFontShutdown();
	return(1);
}

static const luaL_reg IntraFont_methods[] = {
	{"load", IntraFont_Load},
	{"unload", IntraFont_Unload},
	{"print", IntraFont_Print},
	{"shutdown", IntraFont_Shutdown},
	{0, 0}
};

static const luaL_reg IntraFont_meta[] = {
	{0, 0}
};

UserdataRegister(intraFont, IntraFont_methods, IntraFont_meta)

/******************************************************************************
 ** Font **********************************************************************
 *******************************************************************************/

static int Font_load(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L,"Font.load(filename) takes two argument.");
	lua_gc(L, LUA_GCCOLLECT,0);
	Font *font = (Font*)malloc(sizeof(Font));
	const char *_Filename = luaL_checkstring(L,1);
	FILE*_FontFile=fopen(_Filename,"rb");
	if(!_FontFile)
		return luaL_error(L,"Font.load() : Can't open font file.");
	fseek(_FontFile,0,SEEK_END);
	int _Filesize = ftell(_FontFile);
	u8* _FontData = (u8*)malloc(_Filesize);
	if(!_FontData)
	{
		fclose(_FontFile);
		return luaL_error(L,"Font.load(): not enough memory to load font.");
	}
	rewind(_FontFile);
	fread(_FontData, _Filesize, 1, _FontFile);
	fclose(_FontFile);
	int _Error = FT_New_Memory_Face(_FtLibrary, _FontData, _Filesize, 0, &font->face);
	if(_Error)
	{
		free(font);
		free(_FontData);
		return luaL_error(L,"Font.load(): Error loading font.");
	}
	font->data = _FontData;
	font->name = strdup(_Filename);
	Font** luaFont = pushFont(L);
	*luaFont = font;
	return(1);
}

static int lua_CreateMonoSpaced(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Font.CreateMonoSpaced() takes no arguments.");
	lua_gc(L,LUA_GCCOLLECT,0);
	Font*font=(Font*)malloc(sizeof(Font));
	const char * _Filename = "Vera mono spaced";
	int error = FT_New_Memory_Face(_FtLibrary, ttfVeraMono, size_ttfVeraMono, 0, &font->face);
	if (error) {
		free(font);
		return luaL_error(L, "Font.load: Error loading font.");
	}
	font->data = NULL;
	font->name = strdup(_Filename);
	Font** luaFont = pushFont(L);
	*luaFont = font;
	return(1);
}

static int lua_CreateProportional(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Font.CreateProportional() takes no arguments.");
	lua_gc(L, LUA_GCCOLLECT, 0);
	Font* font = (Font*) malloc(sizeof(Font));
	const char* filename = "Vera proportional";
	int error = FT_New_Memory_Face(_FtLibrary, ttfVera, size_ttfVera, 0, &font->face);
	if (error) {
		free(font);
		return luaL_error(L, "Font.load: Error loading font.");
	}
	font->data = NULL;
	font->name = strdup(filename);
	Font** luaFont = pushFont(L);
	*luaFont = font;
	return(1);
}

static int lua_FontSetCharSize(lua_State *L)
{
	if(lua_gettop(L) != 5)
		return luaL_error(L, "Font:setCharSize(width, height, dpix, dpiy) takes 4 arguments and must be called with a colon.");
	Font *font = *toFont(L, 1);
	lua_pushnumber(L, FT_Set_Char_Size(font->face, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)));
	return(1);
}

static int lua_FontSetPixelSize(lua_State *L)
{
	if(lua_gettop(L) != 3)
		return luaL_error(L, "Font:setPixelSize(width, height) takes 2 arguments and must be called with a colon.");
	Font *font = *toFont(L, 1);
	lua_pushnumber(L, FT_Set_Pixel_Sizes(font->face, luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
	return(1);
}

static int lua_FontGetTextSize(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 2) return luaL_error(L, "Font:getTextSize(text) takes 1 argument and must be called with a colon.");
	Font* font = *toFont(L, 1);
	const char* text = luaL_checkstring(L, 2);
	int num_chars = strlen(text);
	FT_GlyphSlot slot = font->face->glyph;
	int x = 0;
	int y = 0;
	int maxHeight = 0;
	int n;
	for (n = 0; n < num_chars; n++) {
		FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
		int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT );
		if (error) continue;
		error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal );
		if (error) continue;
		if (slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;
		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}
	lua_newtable(L);
	lua_pushstring(L, "width"); lua_pushnumber(L, x); lua_settable(L, -3);
	lua_pushstring(L, "height"); lua_pushnumber(L, maxHeight); lua_settable(L, -3);

	return 1;
}

static int Font_Free(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Font:free() takes no argument.");
	Font* font = *toFont(L, 1);
	FT_Done_Face(font->face);
	free(font->name);
	if (font->data)	free(font->data);
	free(font);
	return(1);
}

static int Font__tostring (lua_State *L) {
	lua_pushstring(L, (*toFont(L, 1))->name);
	return(1);
}

static const luaL_reg Font_methods[] = {
	{"load", Font_load},
	{"free", Font_Free},
	{"setCharSize", lua_FontSetCharSize},
	{"setPixelSize", lua_FontSetPixelSize},
	{"getTextSize", lua_FontGetTextSize},
	{"createMonoSpaced", lua_CreateMonoSpaced},
	{"createProportional", lua_CreateProportional},
	{0, 0}
};

static const luaL_reg Font_Meta[] = {
	{"__tostring", Font__tostring},
	{0, 0}
};

UserdataRegister(Font, Font_methods, Font_Meta)

/******************************************************************************
 ** Image *********************************************************************
 *******************************************************************************/

#define SETDEST \
	OSL_IMAGE *dest = NULL; \
{ \
	int type = lua_type(L, 1); \
	if(type == LUA_TTABLE) dest = OSL_SECONDARY_BUFFER; \
	   else if(type == LUA_TUSERDATA) { \
	   dest = *toImage(L, 1); \
} else return luaL_error(L, "Method must be called with a Colon !"); \
}

static int ImageCreateEmpty(lua_State* L)
{
	if(lua_gettop(L) != 2)
	{
		return luaL_error(L, "Image.createEmpty(w, h) takes two arguments.");
	}
	OSL_IMAGE *image = oslCreateImage(luaL_checknumber(L, 1), luaL_checknumber(L, 2), OSL_IN_RAM, OSL_PF_8888);
	OSL_IMAGE ** luaImage = pushImage(L);
	*luaImage = image;
	return(1);
}

static int ImageLoad(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc > 2)
		return luaL_error(L, "Image.load(filename, [TransparentColor]) takes one or two arguments.");
	const char *imagename = luaL_checkstring(L,1);
	if(argc == 2) oslSetTransparentColor(ToColor(L, 2));
	OSL_IMAGE*image=NULL;
	if(strstr(imagename,".jpg") || strstr(imagename,".jpeg"))
		image = oslLoadImageFileJPG((char*)imagename, OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	else if(strstr(imagename,".png"))
		image = oslLoadImageFilePNG((char*)imagename, OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	else if(strstr(imagename,".gif"))
		image = oslLoadImageFileGIF((char*)imagename, OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	else
		return luaL_error(L, "%s : Image Format not supported.", imagename);
	if(image==NULL)
		return luaL_error(L, "Error in image loading.");
	OSL_IMAGE** luaImage = pushImage(L);
	*luaImage = image;
	if(argc == 2) oslDisableTransparentColor();
	return(1);
}

static int ImageBlit(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 4 || argc > 9)
	{
		return luaL_error(L, "Image:blit(x, y, Image, [Alpha], [srcx1], [srcy1], [srcx2], [srcy2]) takes 3 or more arguments.");
	}
	SETDEST
	OSL_IMAGE *source = *toImage(L, 4);
	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);
	int a = (argc >= 5) ? luaL_checknumber(L, 5) : 255;
	int srcx1 = (argc >= 6) ? luaL_checknumber(L, 6) : 0;
	int srcy1 = (argc >= 7) ? luaL_checknumber(L, 7) : 0;
	int srcx2 = (argc >= 8) ? luaL_checknumber(L, 8) : source->stretchX;
	int srcy2 = (argc >= 9) ? luaL_checknumber(L, 9) : source->stretchY;
	oslSetAlpha(OSL_FX_ALPHA, a);
	oslSetImageTileSize(source, srcx1, srcy1, srcx2, srcy2);
	oslDrawImageXY(source, dest->x + x, dest->y + y);
	oslSetAlpha(OSL_FX_ALPHA, 255);
	return(1);
}

static int lua_ImageSetTileSize(lua_State *L)
{
	if(lua_gettop(L) != 5)
		return luaL_error(L, "Image:setTileSize(x1, y1, x2, y2) takes 4 arguments and must be called with a colon.");
	oslSetImageTile(*toImage(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
	return(1);
}

static void fontPrintTextImpl(FT_Bitmap* bitmap, int xofs, int yofs, unsigned int color, unsigned int* framebuffer, int width, int height, int lineSize)
{
	u8 rf = color & 0xff;
	u8 gf = (color >> 8) & 0xff;
	u8 bf = (color >> 16) & 0xff;

	u8* line = bitmap->buffer;
	unsigned int* fbLine = framebuffer + xofs + yofs * lineSize;
	int y;
	int x;
	for (y = 0; y < bitmap->rows; y++) {
		u8* column = line;
		unsigned int* fbColumn = fbLine;
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

void fontPrintTextImage(FT_Bitmap* bitmap, int x, int y, unsigned int color, OSL_IMAGE* image)
{
	fontPrintTextImpl(bitmap, x, y, color, (unsigned int*)image->data, image->stretchX, image->stretchY, image->realSizeX);
}

static int lua_FontPrint(lua_State *L)
{
	if(lua_gettop(L) != 6)
		return luaL_error(L, "Image:fontPrint(font, x, y, text, color) takes 4 arguments.");
	SETDEST
	Font *tFont = *toFont(L, 2);
	int x = luaL_checknumber(L, 3);
	int y = luaL_checknumber(L, 4);
	const char *Text = luaL_checkstring(L, 5);
	unsigned int color = ToColor(L, 6);
	int num_chars = strlen(Text);
	FT_GlyphSlot slot = tFont->face->glyph;
	for (int n = 0; n < num_chars; n++) {
		FT_UInt glyph_index = FT_Get_Char_Index(tFont->face, Text[n]);
		int error = FT_Load_Glyph(tFont->face, glyph_index, FT_LOAD_DEFAULT);
		if (error) continue;
		error = FT_Render_Glyph(tFont->face->glyph, ft_render_mode_normal);
		if (error) continue;
		fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}
	return(1);
}

static int Lua_Printf(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 4 && argc != 5)
		return luaL_error(L, "Image:print(x,y,text,[color]) takes a minimum of 4 arguments");
	SETDEST
	int x = luaL_checkint(L, 2) + dest->x;
	int y = luaL_checkint(L, 3) + dest->y;
	const char* text = luaL_checkstring(L, 4);
	unsigned int color = (argc == 5) ? ToColor(L, 5) : RGB(255,255,255);
	oslSetBkColor(RGB(1,0,0));
	oslSetTransparentColor(RGB(1,0,0));
	oslSetTextColor(color);
	oslPrintf_xy(x , y, text);
	oslDisableTransparentColor();
	return(1);
}

static int ImageFillRect(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5 && argc != 6)
		return luaL_error(L, "Image:fillRect(x, y, w, h, [Color]) takes a minimum of 4 arguments.");
	SETDEST
	unsigned int color = RGBA(0, 0, 0, 0);
	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);
	int w = luaL_checknumber(L, 4);
	int h = luaL_checknumber(L, 5);
	if(argc == 6) color = ToColor(L, 6);
	oslDrawFillRect(x, y, w, h, color);
	return(1);
}

static int ImageResize(lua_State *L)
{
	if(lua_gettop(L) != 3)
	{
		return luaL_error(L, "Image:resize(x, y) takes 2 arguments.");
	}
	SETDEST
	oslLockImage(dest);
	dest->stretchX = luaL_checknumber(L, 2);
	dest->stretchY = luaL_checknumber(L, 3);
	oslUnlockImage(dest);
	return(1);
}

static int lua_ScreenFlip(lua_State *L)
{
	if(lua_gettop(L) != 0)
	{
		return luaL_error(L, "Error : Screen.flip() takes no arguments.");
	}
	oslEndFrame();
	oslSyncFrame();
	return(1);
}

static int lua_ScreenWaitVBlanks(lua_State *L)
{
	if(lua_gettop(L)==1){
		int i;
		for (i = 1; i < luaL_checknumber(L,1); ++i){
			sceDisplayWaitVblankStart();
		}
	}else{
		sceDisplayWaitVblankStart();
	}
	return(1);
}

static int lua_DrawLine(lua_State *L)
{
	if(lua_gettop(L) != 5 && lua_gettop(L) > 6)
	{
		return luaL_error(L, "image:drawLine(x1, y1, x2, y2, [Color]) takes 4 or 5 arguments.");
	}
	SETDEST
	unsigned int color = RGBA(0, 0, 0, 0);
	if(lua_gettop(L) == 6) color = ToColor(L, 6);
	float x0 = luaL_checkinteger(L, 2);
	float y0 = luaL_checkinteger(L, 3);
	float x1 = luaL_checkinteger(L, 4);
	float y1 = luaL_checkinteger(L, 5);
	float grad = ((y1-y0)/(x1-x0));
	float i;
	for (i = 0; i < x1; i=i+1)
		oslSetImagePixel(dest, i+x0,y0+(i*grad), color);
	return(1);
}

static int lua_PutPixel(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 3 && argc != 4)
		return luaL_error(L, "Image:putPixel(x, y, [Color]) takes two or three arguments.");
	SETDEST
	unsigned int color = RGBA(0, 0, 0, 0);
	if(argc == 4) color = ToColor(L, 4);
	oslSetImagePixel(dest, luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), color);
	return(1);
}

static int lua_GetPixel(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 3)
		return luaL_error(L, "Image:getPixel(x, y) takes two arguments.");
	SETDEST
	char buffer[50];
	unsigned int color;
	color = oslConvertColorEx(dest->palette, OSL_PF_8888, dest->pixelFormat, oslGetImagePixel(dest, luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
	sprintf(buffer, "%x", color);
	lua_pushstring(L, buffer);
	return(1);
}

static int lua_ImageWidth(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:width() takes no arguments.");
	SETDEST
	lua_pushnumber(L, dest->stretchX);
	return(1);
}

static int lua_ImageHeight(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:height() takes no arguments.");
	SETDEST
	lua_pushnumber(L, dest->stretchY);
	return(1);
}

static int lua_ImageRotate(lua_State *L)
{
	if(lua_gettop(L) != 4)
		return luaL_error(L, "Image:rotate(centerx, centery, angle) takes 3 arguments.");
	SETDEST
	oslLockImage(dest);
	dest->centerX = luaL_checknumber(L, 2);
	dest->centerY = luaL_checknumber(L, 3);
	dest->angle = luaL_checknumber(L, 4);
	oslUnlockImage(dest);
	return(1);
}

static int lua_ImageSave(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Image:save(filename) takes 1 argument.");
	SETDEST
	oslWriteImageFilePNG(dest, luaL_checkstring(L, 2), 0);
	return(1);
}

static int lua_ImageFree(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:free() takes no arguments.");
	SETDEST
	oslDeleteImage(dest);
	return(1);
}

static int lua_ImageClear(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc > 2)
		return luaL_error(L, "Image:clear([Color]) takes 1 or 0 arguments.");
	SETDEST
	unsigned int color = RGBA(0, 0, 0, 0);
	if(argc == 2) color = ToColor(L, 2);
	oslClearImage(dest, color);
	return(1);
}

static int lua_ImageSwizzle(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:swizzle() takes no arguments and must be called with a colon.");
	SETDEST
	oslSwizzleImage(dest);
	return(1);
}

static int lua_ImageUnswizzle(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:unSwizzle() takes no arguments and must be called with a colon.");
	SETDEST
	oslUnswizzleImage(dest);
	return(1);
}

static int lua_ImageToVram(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:toVram() takes no arguments and must be called with a colon.");
	SETDEST
	oslMoveImageTo(dest, OSL_IN_VRAM);
	return(1);
}

static int lua_ImageToRam(lua_State *L)
{
	if(lua_GetPixel(L) != 1)
		return luaL_error(L, "Image:toRam() takes no arguments and must be called with a colon.");
	SETDEST
	oslMoveImageTo(dest, OSL_IN_RAM);
	return(1);
}

static int lua_ImageSetAlpha(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Image.SetAlpha(alpha) takes one argument.");
	oslSetAlpha(OSL_FX_ALPHA, luaL_checknumber(L, 1));
	return(1);
}

int sqrt2(unsigned int n){
	unsigned int a;
	for (a=0;n>=(2*a)+1;n-=(2*a++)+1);
	return a;
}

static int lua_DrawCircle(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc > 5 || argc == 0)
		return luaL_error(L, "image:drawCircle(centerX, centerY, Radius, [Color]) takes 4 or 5 arguments and must be called with a colon.");
	SETDEST
	float centerx = luaL_checknumber(L, 2) + dest->x;
	float centery = luaL_checknumber(L, 3) + dest->y;
	float r = luaL_checknumber(L, 4);
	unsigned int color = (argc == 5) ? ToColor(L, 5) : RGBA(255,255,255,255);
	int x, y;
	for(x = -1 * r; x <= r; x++) {
		y = sqrt2((r * r) - (x * x));
		oslDrawLine(x + centerx, y + centery, x + centerx + 1, y + centery + 1, color);
		y = sqrt2((r * r) - (x * x)) * -1;
		oslDrawLine(x + centerx, y + centery, x + centerx + 1, y + centery + 1, color);
	}
	for(y = -1 * r; y <= r; y++)
	{
		x = sqrt2((r * r) - (y * y));
	    oslDrawLine(x + centerx, y + centery, x + centerx + 1, y + centery + 1, color);
		x = sqrt2((r * r) - (y * y)) * -1;
		oslDrawLine(x + centerx, y + centery, x + centerx + 1, y + centery + 1, color);
	}
	return(1);
}

static int lua_FillCircle(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc > 5 || argc == 0)
		return luaL_error(L, "image:fillCircle(centerX, centerY, Radius, [Color]) takes 4 or 5 arguments and must be called with a colon.");
	SETDEST
		float centerx = luaL_checknumber(L, 2) + dest->x;
	float centery = luaL_checknumber(L, 3) + dest->y;
	float r = luaL_checknumber(L, 4);
	unsigned int color = (argc == 5) ? ToColor(L, 5) : RGBA(255,255,255,255);
	int x, y;
	for(x = -1 * r; x <= r; x++) {
		y = sqrt2((r * r) - (x * x));
		oslDrawLine(x + centerx, y + centery, x + centerx, y + centery, color);
		y = sqrt2((r * r) - (x * x)) * -1;
		oslDrawLine(x + centerx, y + centery, x + centerx + 1, y + centery + 1, color);
	}
	for(y = -1 * r; y <= r; y++)
	{
		x = sqrt2((r * r) - (y * y));
		oslDrawLine(x + centerx, y + centery, x + centerx, y + centery, color);
		x = sqrt2((r * r) - (y * y)) * -1;
		oslDrawLine(x + centerx, y + centery, x + centerx, y + centery , color);
	}
	return(1);
}

static int lua_ImageNegative(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Image:negative() takes no arguments and must be called with a colon.");
	SETDEST
	OSL_IMAGE *tmp = oslCreateImage(dest->stretchX, dest->stretchY, OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	unsigned int img_pixel, color;
	for(int i = 0; i < dest->stretchX; i++)
	{
		for(int k = 0; i < dest->stretchY; i++)
		{
			img_pixel = oslConvertColorEx(dest->palette, OSL_PF_8888, dest->pixelFormat, oslGetImagePixel(dest, i, k));
			color = RGBA(255 - RGBA_R(img_pixel),255 - RGBA_G(img_pixel), 255 - RGBA_B(img_pixel), 255 - RGBA_A(img_pixel));
			oslSetImagePixel(tmp, i , k, color);
		}
	}
	OSL_IMAGE **luaImg = pushImage(L);
	*luaImg = tmp;
	return(1);
}

static int Image_DrawImageZoom(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 5)
	{
		return luaL_error(L, "Image:drawImageZoom(x, y, Image, Zoom) takes 3 arguments and must be called with a colon.");
	}
	SETDEST
	int x = luaL_checknumber(L, 1) + dest->x;
	int y = luaL_checknumber(L, 2) + dest->y;
	OSL_IMAGE *img = *toImage(L, 3);
	int Zoom = luaL_checknumber(L, 4);
	OSL_UVFLOAT_VERTEX *vertices;
	oslSetTexture(img);
	if(oslImageGetAutoStrip(img)) {
		if(oslVerifyStripBlit(img))
			return(0x0);
	}
	vertices = (OSL_UVFLOAT_VERTEX *)sceGuGetMemory(2 * sizeof(OSL_UVFLOAT_VERTEX));
	vertices[0].u = img->offsetX0;
	vertices[0].v = img->offsetY0;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;

	vertices[1].u = img->offsetX1;
	vertices[1].v = img->offsetY1;
	vertices[1].x = x + img->stretchX * Zoom;
	vertices[1].y = y + img->stretchX * Zoom;
	vertices[1].z = 0;

	sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
	return(1);
}

static int Image_DrawGradientLine(lua_State *L) {
	if(lua_gettop(L) != 6)
		return luaL_error(L, "Image:drawGradientLine(x0, y0, x1, y1, color1, color2) takes 6 arguments and must be called with a colon.");
	SETDEST
	int x0 = luaL_checknumber(L, 2) + dest->x;
	int y0 = luaL_checknumber(L, 3) + dest->x;
	int x1 = luaL_checknumber(L, 4) + dest->y;
	int y1 = luaL_checknumber(L, 5) + dest->y;
	unsigned int color1 = ToColor(L, 6);
	unsigned int color2 = ToColor(L, 7);
	OSL_LINE_VERTEX *vertices;
	vertices = (OSL_LINE_VERTEX *)sceGuGetMemory(2 * sizeof(OSL_LINE_VERTEX));
	color1 = oslBlendColor(color1);
	color2 = oslBlendColor(color2);
	vertices[0].color = color1;
	vertices[0].x = x0;
	vertices[0].y = y0;
	vertices[0].z = 0;

	vertices[1].color = color2;
	vertices[1].x = x1;
	vertices[1].y = y1;
	vertices[1].z = 0;

	sceGuDrawArray(GU_LINES, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
	return(1);
}

static int lua_DrawGradientTriangle(lua_State *L) {
	if(lua_gettop(L) != 10)
		return luaL_error(L, "Image:drawGradientTriangle(x0, y0, x1, y1, x2, y2, color1, color2, color3) takes 9 arguments and must be called with a colon.");
	SETDEST
	int x0 = luaL_checknumber(L, 2) + dest->x;
	int y0 = luaL_checknumber(L, 3) + dest->y;
	int x1 = luaL_checknumber(L, 4) + dest->x;
	int y1 = luaL_checknumber(L, 5) + dest->y;
	int x2 = luaL_checknumber(L, 6) + dest->x;
	int y2 = luaL_checknumber(L, 7) + dest->y;
	unsigned int Color1 = oslBlendColor(ToColor(L, 8));
	unsigned int Color2 = oslBlendColor(ToColor(L, 9));
	unsigned int Color3 = oslBlendColor(ToColor(L, 10));
	OSL_LINE_VERTEX *vertices;
	vertices = (OSL_LINE_VERTEX *)sceGuGetMemory(4 * sizeof(OSL_LINE_VERTEX));
	
	vertices[0].color = Color1;
	vertices[0].x = x0;
	vertices[0].y = y0;
	vertices[0].z = 0;

	vertices[1].color = Color2;
	vertices[1].x = x1;
	vertices[1].y = y0;
	vertices[1].z = 0;

	vertices[2].color = Color3;
	vertices[2].x = x0;
	vertices[2].y = y1;
	vertices[2].z = 0;

	vertices[3].color = Color1;
	vertices[3].x = x0;
	vertices[3].y = y0;
	vertices[3].z = 0;

	sceGuDrawArray(GU_TRIANGLE_STRIP, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 4, 0, vertices);
	return(1);
}

static int Image_DrawGradientRect(lua_State *L) 
{
	if(lua_gettop(L) != 9)
		return luaL_error(L, "Image:drawGradientRect(x0, y0, x1, y1, color1, color2, color3, color4) takes 8 arguments and must be called with a colon.");
	SETDEST
	int x0 = luaL_checknumber(L, 2) + dest->x;
	int y0 = luaL_checknumber(L, 3) + dest->y;
	int x1 = luaL_checknumber(L, 4) + dest->x;
	int y1 = luaL_checknumber(L, 5) + dest->y;
	unsigned int color1 = ToColor(L, 6);
	unsigned int color2 = ToColor(L, 7);
	unsigned int color3 = ToColor(L, 8);
	unsigned int color4 = ToColor(L, 9);
	if (osl_currentAlphaEffect == OSL_FX_ALPHA)		{
		color1 = (color1 & 0xffffff) | (((((color1 & 0xff000000) >> 24) * osl_currentAlphaCoeff) >> 8) << 24);
		color2 = (color2 & 0xffffff) | (((((color2 & 0xff000000) >> 24) * osl_currentAlphaCoeff) >> 8) << 24);
		color3 = (color3 & 0xffffff) | (((((color3 & 0xff000000) >> 24) * osl_currentAlphaCoeff) >> 8) << 24);
		color4 = (color4 & 0xffffff) | (((((color4 & 0xff000000) >> 24) * osl_currentAlphaCoeff) >> 8) << 24);
	}
	oslDrawGradientRect(x0, y0, x1, y1, color1, color2, color3, color4);
	return(1);
}

static int Lua_PrintVertical(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 5 && argc != 6)
		return luaL_error(L, "Image:printVertical(x,y,text, distance, [color]) takes a minimum of 5 arguments");
	SETDEST
	int x = luaL_checkint(L, 2) + dest->x;
	int y = luaL_checkint(L, 3) + dest->y;
	const char* text = luaL_checkstring(L, 4);
	int distance = luaL_checknumber(L, 5);
	unsigned int color = (argc == 6) ? ToColor(L, 6) : RGB(255,255,255);
	int taille = strlen(text);
	oslSetBkColor(RGB(1,0,0));
	oslSetTransparentColor(RGB(1,0,0));
	oslSetTextColor(color);
	for(int i = 0; i <= taille; i++)
	{
		oslPrintf_xy(x , y + i * distance, &text[i]);
	}
	oslDisableTransparentColor();
	return(1);
}

static int lua_PrintUnderline(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 4 && argc > 6)
		return luaL_error(L, "Image:printUnderline(x,y,text,[textColor], [lineColor]) takes a minimum of 4 arguments");
	SETDEST
	int x = luaL_checkint(L, 2) + dest->x;
	int y = luaL_checkint(L, 3) + dest->y;
	const char* text = luaL_checkstring(L, 4);
	unsigned int color = (argc >= 5) ? ToColor(L, 5) : RGB(255,255,255);
	unsigned int colorl = (argc == 6) ? ToColor(L, 6) : RGB(255, 255, 255);
	oslSetBkColor(RGB(1,0,0));
	oslSetTransparentColor(RGB(1,0,0));
	oslSetTextColor(color);
	oslPrintf_xy(x , y, text);
	oslDrawLine(x - 2, y + 8 + 1, x + 8 * strlen(text), y + 8 + 1, colorl);
	oslDisableTransparentColor();
	return(1);
}

static const luaL_Reg Image_methods[] = {
	{"printUnderline", lua_PrintUnderline},
	{"printVertical", Lua_PrintVertical},
	{"drawGradientTriangle", lua_DrawGradientTriangle},
	{"drawGradientRect", Image_DrawGradientRect},
	{"drawGradientLine", Image_DrawGradientLine},
	{"drawImageZoom", Image_DrawImageZoom},
	{"negative", lua_ImageNegative},
	{"fillCircle", lua_FillCircle},
	{"drawCircle", lua_DrawCircle},
	{"SetAlpha", lua_ImageSetAlpha},
	{"setTileSize", lua_ImageSetTileSize},
	{"toRam", lua_ImageToRam},
	{"toVram", lua_ImageToVram},
	{"unSwizzle", lua_ImageUnswizzle},
	{"swizzle", lua_ImageSwizzle},
	{"free", lua_ImageFree},
	{"clear", lua_ImageClear},
	{"createEmpty", ImageCreateEmpty},
	{"putPixel", lua_PutPixel},
	{"getPixel", lua_GetPixel},
	{"save", lua_ImageSave},
	{"width", lua_ImageWidth},
	{"height", lua_ImageHeight},
	{"rotate", lua_ImageRotate},
	{"drawLine", lua_DrawLine},
	{"resize", ImageResize},
	{"load", ImageLoad},
	{"blit", ImageBlit},
	{"print", Lua_Printf},
	{"fillRect", ImageFillRect},
	{"fontPrint", lua_FontPrint},
	{"intraFontPrint", IntraFont_Print},
	{0,0}
};

static const luaL_Reg Image_meta[] = {
	{0, 0}
};

UserdataRegister(Image, Image_methods, Image_meta);

/******************************************************************************
 ** Color *********************************************************************
 *******************************************************************************/

static int Color_new(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 3 && argc != 4) return luaL_error(L, "Color.new(r, g, b, [a]) takes either three color arguments or three color arguments and an alpha value.");
	unsigned int r = CLAMP(luaL_checkint(L, 1), 0, 255);
	unsigned int g = CLAMP(luaL_checkint(L, 2), 0, 255);
	unsigned int b = CLAMP(luaL_checkint(L, 3), 0, 255);
	unsigned int a = 255;
	if(argc == 4)
		a = CLAMP(luaL_checkint(L, 4), 0, 255);
	if(r < 16) r = 16;
	if(g < 16) g = 16;
	if(b < 16) b = 16;
	if(a < 16) a = 16;
	char buffer[50];
	sprintf(buffer, "%x", RGBA(r, g, b, a));
	lua_pushstring(L, buffer);
	return(1);
}

static int Color_colors (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "color:colors() takes no arguments, and it must be called from an instance with a colon.");
	unsigned int color = ToColor(L, 1);
	int r = RGBA_R(color);
	int g = RGBA_G(color);
	int b = RGBA_B(color);
	int a = RGBA_A(color);
	lua_newtable(L);
	lua_pushstring(L, "r"); lua_pushnumber(L, r); lua_settable(L, -3);
	lua_pushstring(L, "g"); lua_pushnumber(L, g); lua_settable(L, -3);
	lua_pushstring(L, "b"); lua_pushnumber(L, b); lua_settable(L, -3);
	lua_pushstring(L, "a"); lua_pushnumber(L, a); lua_settable(L, -3);
	return(1);
}

static int Color_tostring (lua_State *L) {
	Color_colors(L);
	lua_pushstring(L, "r"); lua_gettable(L, -2); int r = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "g"); lua_gettable(L, -2); int g = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "b"); lua_gettable(L, -2); int b = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "a"); lua_gettable(L, -2); int a = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pop(L, 1);
	lua_pushfstring(L, "Color (r %d, g %d, b %d, a %d)", r, g, b, a);
	return(1);
}

static int Color_equal(lua_State *L) {
	unsigned int a = ToColor(L, 1);
	unsigned int b = ToColor(L, 2);
	lua_pushboolean(L, a == b);
	return(1);
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

static int lua_screenStartDraw(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "screen.startDraw() takes no arguments.");
	oslStartDrawing();
	return(1);
}

static int lua_screenEndDraw(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "screen.endDraw() takes no arguments.");
	oslEndDrawing();
	return(1);
}

static const luaL_Reg Screen_functions[] = {
	{"startDraw", lua_screenStartDraw},
	{"endDraw", lua_screenEndDraw},
	{"flip", lua_ScreenFlip},
	{"waitVblankStart", lua_ScreenWaitVBlanks},
	{0,0}
};

UserdataRegister(Color, Color_methods, Color_meta);

void luaGraphics_init(lua_State *L) {
	Image_register(L);
	Color_register(L);
	Font_register(L);
	intraFont_register(L);
	luaL_openlib(L, "screen", Screen_functions, 0);
	luaL_openlib(L, "screen", Image_methods, 0);
}
