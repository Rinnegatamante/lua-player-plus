#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspgum.h>
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
#include "include/luaplayer.h"
#include "vera.c"
#include "veraMono.c"
#include "libs/graphics/graphics.h"

UserdataStubs(Color, Color)

FT_Library  ft_library;

/// screen.*
/// ====================
static int lua_waitVblankStart(lua_State *L)
{
    int argc = lua_gettop(L), t = 0;
    if (argc != 0 && argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments"); // can be called as both screen.wait...() and screen:wait...()
    if (argc) t = lua_type(L, 1);
    if (argc == 0 || t != LUA_TNUMBER) {
        sceDisplayWaitVblankStart();
    } else {
        int count = (t == LUA_TNUMBER)?luaL_checkint(L, 1):luaL_checkint(L, 2);
        int i;
        for (i = 0; i < count; i++) sceDisplayWaitVblankStart();
    }
    return 0;
}

static int lua_flipScreen(lua_State *L)
{
    flipScreen();
    return 0;
}

/// Utility
/// ====================

// returns 0, if nothing to blit
static int adjustBlitRectangle(
    int sourceWidth, int sourceHeight, 
    int destinationWidth, int destinationHeight, 
    int* sx, int* sy, 
    int* width, int* height, 
    int* dx, int* dy) 
{ 
    if (*width <= 0 || *height <= 0) return 0;  // zero area, nothing to blit 
    if (*sx < 0 || *sy < 0) return 0;  // illegal, source is not clipped 
    if (*dx < 0) { 
        *width += *dx; 
        if (*width <= 0) return 0; 
        *sx -= *dx; 
        *dx = 0; 
        if (*sx >= destinationWidth) return 0; 
    } 
    if (*dy < 0) { 
        *height += *dy; 
        if (*height <= 0) return 0; 
        *sy -= *dy; 
        *dy = 0; 
        if (*sy >= destinationHeight) return 0; 
    } 
    if (*dx + *width > destinationWidth) { 
        *width = destinationWidth - *dx; 
        if (*width <= 0) return 0; 
    } 
    if (*dy + *height > destinationHeight) { 
        *height = destinationHeight - *dy; 
        if (*height <= 0) return 0; 
    } 
    return 1; 
}     

struct Font {
    char* name;
    FT_Face face;
    u8* data;
};
typedef struct Font Font;

UserdataStubs(Font, Font*) //==========================
static int Font_load(lua_State *L) {
    if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: Font.load(filename) takes one argument.");
    lua_gc(L, LUA_GCCOLLECT, 0);
    Font* font = (Font*) malloc(sizeof(Font));
    const char* filename = luaL_checkstring(L, 1);
    
    // cache font for faster access. This might be a bad idea for big fonts
    FILE* fontFile = fopen(filename, "rb");
    if (!fontFile) return luaL_error(L, "Font.load: can't open font file.");
    fseek(fontFile, 0, SEEK_END);
    int filesize = ftell(fontFile);
    u8* fontData = (u8*) malloc(filesize);
    if (!fontData) {
        fclose(fontFile);
        return luaL_error(L, "Font.load: not enough memory to cache font file.");
    }
    rewind(fontFile);
    fread(fontData, filesize, 1, fontFile);
    fclose(fontFile);
    int error = FT_New_Memory_Face(ft_library, fontData, filesize, 0, &font->face);
    if (error) {
        free(font);
        free(fontData);
        return luaL_error(L, "Font.load: Error loading font.");
    }
    font->data = fontData;
    font->name = strdup(filename);
    Font** luaFont = pushFont(L);
    *luaFont = font;
    return 1;
}

static int Font_createMonoSpaced(lua_State *L) {
    if (lua_gettop(L) != 0) return luaL_error(L, "Argument error: Font.createMonoSpaced() takes no arguments.");
    lua_gc(L, LUA_GCCOLLECT, 0);
    Font* font = (Font*) malloc(sizeof(Font));
    const char* filename = "Vera mono spaced";

    int error = FT_New_Memory_Face(ft_library, ttfVeraMono, size_ttfVeraMono, 0, &font->face);
    if (error) {
        free(font);
        return luaL_error(L, "Font.load: Error loading font.");
    }
    font->data = NULL;
    font->name = strdup(filename);
    Font** luaFont = pushFont(L);
    *luaFont = font;
    return 1;
}

static int Font_createProportional(lua_State *L) {
    if (lua_gettop(L) != 0) return luaL_error(L, "Argument error: Font.createProportional() takes no arguments.");
    lua_gc(L, LUA_GCCOLLECT, 0);
    Font* font = (Font*) malloc(sizeof(Font));
    const char* filename = "Vera proportional";

    int error = FT_New_Memory_Face(ft_library, ttfVera, size_ttfVera, 0, &font->face);
    if (error) {
        free(font);
        return luaL_error(L, "Font.load: Error loading font.");
    }
    font->data = NULL;
    font->name = strdup(filename);
    Font** luaFont = pushFont(L);
    *luaFont = font;
    return 1;
}

static int Font_setCharSize(lua_State *L) {
    int argc = lua_gettop(L); 
    if (argc != 5) return luaL_error(L, "wrong number of arguments"); 
    Font* font = *toFont(L, 1);
    int width = luaL_checkint(L, 2); 
    int height = luaL_checkint(L, 3); 
    int dpiX = luaL_checkint(L, 4); 
    int dpiY = luaL_checkint(L, 5); 
    lua_pushnumber(L, FT_Set_Char_Size(font->face, width, height, dpiX, dpiY));
    return 1;
}

static int Font_setPixelSizes(lua_State *L) {
    int argc = lua_gettop(L); 
    if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
    Font* font = *toFont(L, 1);
    int width = luaL_checkint(L, 2); 
    int height = luaL_checkint(L, 3); 
    lua_pushnumber(L, FT_Set_Pixel_Sizes(font->face, width, height));
    return 1;
}

static int Font_getTextSize(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
    Font* font = *toFont(L, 1);
    const char* text = luaL_checkstring(L, 2);

    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int x = 0;
    int y = 0;
    int maxHeight = 0;
    int n;
    for (n = 0; n < num_chars; n++) {
        // TODO: this can be done better with glyph bounding box
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

static int Font_free(lua_State *L) {
    Font* font = *toFont(L, 1);
    FT_Done_Face(font->face);
    free(font->name);
    if (font->data)    free(font->data);
    free(font);
    return 0;
}

static int Font_tostring (lua_State *L) {
    lua_pushstring(L, (*toFont(L, 1))->name);
    return 1;
}
static const luaL_reg Font_methods[] = {
    {"load", Font_load},
    {"createMonoSpaced", Font_createMonoSpaced},
    {"createProportional", Font_createProportional},
    {"setCharSize", Font_setCharSize},
    {"setPixelSizes", Font_setPixelSizes},
    {"getTextSize", Font_getTextSize},
    {0,0}
};
static const luaL_reg Font_meta[] = {
    {"__gc", Font_free},
    {"__tostring", Font_tostring},
    {0,0}
};
UserdataRegister(Font, Font_methods, Font_meta)




UserdataStubs(Image, Image*) //==========================
//Fast Blit Stuff!

static int Image_startBlit(lua_State *L)
{
    if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
    FastStartBlit();
    return 0;
}

static int Image_endBlit(lua_State *L)
{
    if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
    FastEndBlit();
    return 0;
}

//End Fast Blit

static int Image_createEmpty(lua_State *L)
{
    if (lua_gettop(L) != 2) return luaL_error(L, "Argument error: Image.createEmpty(w, h) takes two arguments.");
    int w = luaL_checkint(L, 1);
    int h = luaL_checkint(L, 2);
    if (w <= 0 || h <= 0 || w > 512 || h > 512) return luaL_error(L, "invalid size");
    lua_gc(L, LUA_GCCOLLECT, 0);
    Image* image = createImage(w, h);
    if (!image) return luaL_error(L, "can't create image");
    Image** luaImage = pushImage(L);
    *luaImage = image;
    return 1;
}
static int Image_load (lua_State *L) {
    if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: Image.load(filename) takes one argument.");
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

//More Fast Blit!
static int Image_fastClear (lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 1 && argc != 2) return luaL_error(L, "Argument error: Image:clear([color]) zero or one argument.");
    Color color = (argc==2)?*toColor(L, 2):0;

    SETDEST
    if(dest)
        clearImage(color, dest);
    else
        fastClearScreen(color);
    return 0;
}

static int Image_fastBlit (lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 4 && argc != 5 && argc != 8 && argc != 9) return luaL_error(L, "Argument error: image:blit() takes 3, 4, 7 or 8 arguments, and MUST be called with a colon.");
    
    bool alpha = (argc==5 || argc==9)?lua_toboolean(L, -1):true; 
    if(argc==5 || argc==9) lua_pop(L, 1);
    
    SETDEST
        
    int dx = luaL_checkint(L, 1);
    int dy = luaL_checkint(L, 2);
    Image* source = *toImage(L, 3);
    
    bool rect = (argc ==8 || argc == 9) ;
    int sx = rect? luaL_checkint(L, 4) : 0;
    int sy = rect? luaL_checkint(L, 5) : 0;
    int width = rect? luaL_checkint(L, 6) : source->imageWidth;
    int height = rect? luaL_checkint(L, 7) : source->imageHeight;
    
    if (!dest) {
        if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
        alpha?
            fastBlitAlphaImageToScreen(sx, sy, width, height, source, dx, dy) :
            fastBlitImageToScreen(sx, sy, width, height, source, dx, dy);
    } else {
        if (!adjustBlitRectangle(width, height, dest->imageWidth, dest->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
        alpha?
            blitAlphaImageToImage(sx, sy, width, height, source, dx, dy, dest) :
            blitImageToImage(sx, sy, width, height, source, dx, dy, dest);
    }
    return 0;
}

static int Image_blit (lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 4 && argc != 5 && argc != 8 && argc != 9) return luaL_error(L, "Argument error: image:blit() takes 3, 4, 7 or 8 arguments, and MUST be called with a colon.");
    
    bool alpha = (argc==5 || argc==9)?lua_toboolean(L, -1):true; 
    if(argc==5 || argc==9) lua_pop(L, 1);
    
    SETDEST
        
    int dx = luaL_checkint(L, 1);
    int dy = luaL_checkint(L, 2);
    Image* source = *toImage(L, 3);
    
    bool rect = (argc ==8 || argc == 9) ;
    int sx = rect? luaL_checkint(L, 4) : 0;
    int sy = rect? luaL_checkint(L, 5) : 0;
    int width = rect? luaL_checkint(L, 6) : source->imageWidth;
    int height = rect? luaL_checkint(L, 7) : source->imageHeight;
    
    if (!dest) {
        if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
        alpha?
            blitAlphaImageToScreen(sx, sy, width, height, source, dx, dy) :
            blitImageToScreen(sx, sy, width, height, source, dx, dy);
    } else {
        if (!adjustBlitRectangle(width, height, dest->imageWidth, dest->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
        alpha?
            blitAlphaImageToImage(sx, sy, width, height, source, dx, dy, dest) :
            blitImageToImage(sx, sy, width, height, source, dx, dy, dest);
    }
    return 0;
}
static int Image_clear (lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 1 && argc != 2) return luaL_error(L, "Argument error: Image:clear([color]) zero or one argument.");
    Color color = (argc==2)?*toColor(L, 2):0;

    SETDEST
    if(dest)
        clearImage(color, dest);
    else
        clearScreen(color);
    return 0;
}
static int Image_fillRect (lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
    SETDEST

    int x0 = luaL_checkint(L, 1);
    int y0 = luaL_checkint(L, 2);
    int width = luaL_checkint(L, 3);
    int height = luaL_checkint(L, 4);
    Color color = (argc==6)?*toColor(L, 5):0;
    
    if (width <= 0 || height <= 0) return 0;
    if (x0 < 0) {
        width += x0;
        if (width <= 0) return 0;
        x0 = 0;
    }
    if (y0 < 0) {
        height += y0;
        if (height <= 0) return 0;
        y0 = 0;
    }
    if (!dest) {
        if (width <= 0 || height <= 0) return 0;
        if (x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT) return 0;
        if (x0 + width >= SCREEN_WIDTH) {
            width = SCREEN_WIDTH - x0;
            if (width <= 0) return 0;
        }
        if (y0 + height >= SCREEN_HEIGHT) {
            height = SCREEN_HEIGHT - y0;
            if (height <= 0) return 0;
        }
        fillScreenRect(color, x0, y0, width, height);
    } else {
        if (x0 >= dest->imageWidth || y0 >= dest->imageHeight) return 0;
        if (x0 + width >= dest->imageWidth) {
            width = dest->imageWidth - x0;
            if (width <= 0) return 0;
        }
        if (y0 + height >= dest->imageHeight) {
            height = dest->imageHeight - y0;
            if (height <= 0) return 0;
        }
        fillImageRect(color, x0, y0, width, height, dest);
    }
    return 0;
    
}
static int Image_drawLine (lua_State *L) {
    int argc = lua_gettop(L); 
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments"); 
    SETDEST
    int x0 = luaL_checkint(L, 1); 
    int y0 = luaL_checkint(L, 2); 
    int x1 = luaL_checkint(L, 3); 
    int y1 = luaL_checkint(L, 4); 
    Color color = (argc==6) ? *toColor(L, 5) : 0;
    
    // TODO: better clipping
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (!dest) {
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
static int Image_pixel (lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 3 && argc != 4) return luaL_error(L, "Image:pixel(x, y, [color]) takes two or three arguments, and must be called with a colon.");
    SETDEST
    int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
    Color color = (argc == 4)?*toColor(L, 3):0;
    if(dest) {
        if (x >= 0 && y >= 0 && x < dest->imageWidth && y < dest->imageHeight) {
            if(argc==3) {
                *pushColor(L) = getPixelImage(x, y, dest);
                return 1;
            } else {
                putPixelImage(color, x, y, dest);
                return 0;
            }
        }
    } else {
        if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
            if(argc==3) {
                *pushColor(L) = getPixelScreen(x, y);
                return 1;
            } else {
                putPixelScreen(color, x, y);
                return 0;
            }
        }
    }

    return luaL_error(L, "An argument was incorrect.");
}
static int Image_print (lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
    SETDEST
    int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
    Color color = (argc == 5)?*toColor(L, 4):0xFF000000;
    if (!dest) {
        printTextScreen(x, y, text, color);
    } else {
        printTextImage(x, y, text, color, dest);
    }
    return 0;
}

static int Image_fontPrint(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int x = luaL_checkint(L, 2);
    int y = luaL_checkint(L, 3);
    const char* text = luaL_checkstring(L, 4);
    Color color = (argc == 6)?*toColor(L, 5):0xFF000000;

    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_fontPrintRight(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
    Color color = (argc == 5)?*toColor(L, 4):0xFF000000;
    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int x1 = 0;
    int maxHeight = 0;
    int n1;
    for (n1 = 0; n1 < num_chars; n1++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n1]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT );
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal );
        if (error) continue;
        if (slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;
        x1 += slot->advance.x >> 6;
    }
    int x = 480 - x1;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_fontPrintRightOutline(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
    Color color = (argc >= 5)?*toColor(L, 4):0xFF000000;
    Color color2 = (argc == 6)?*toColor(L, 5):0xFF000000;
    
    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int x1 = 0;
    int maxHeight = 0;
    int n1;
    for (n1 = 0; n1 < num_chars; n1++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n1]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT );
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal );
        if (error) continue;
        if (slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;
        x1 += slot->advance.x >> 6;
    }
    int x = 480-x1-2;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_fontPrintCenter(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
    Color color = (argc == 5)?*toColor(L, 4):0xFF000000;
    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int x1 = 0;
    int maxHeight = 0;
    int n1;
    for (n1 = 0; n1 < num_chars; n1++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n1]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT );
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal );
        if (error) continue;
        if (slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;
        x1 += slot->advance.x >> 6;
    }
    int x = 480/2 - x1/2;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_fontPrintOutline(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 6 && argc != 7) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int x = luaL_checkint(L, 2);
    int y = luaL_checkint(L, 3);
    const char* text = luaL_checkstring(L, 4);
    Color color = (argc >= 6)?*toColor(L, 5):0xFF000000;
    Color color2 = (argc == 7)?*toColor(L, 6):0xFF000000;
    
    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_fontPrintCenterOutline(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
    SETDEST
    Font* font = *toFont(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
    Color color = (argc >= 5)?*toColor(L, 4):0xFF000000;
    Color color2 = (argc == 6)?*toColor(L, 5):0xFF000000;
    
    int num_chars = strlen(text);
    FT_GlyphSlot slot = font->face->glyph;
    int x1 = 0;
    int maxHeight = 0;
    int n1;
    for (n1 = 0; n1 < num_chars; n1++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n1]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT );
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal );
        if (error) continue;
        if (slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;
        x1 += slot->advance.x >> 6;
    }
    int x = 480/2 - x1/2;
    int n;
    for (n = 0; n < num_chars; n++) {
        FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
        int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;
        error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
        if (error) continue;
        if (dest) {
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2, dest);
            fontPrintTextImage(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, dest);
        } else {
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left - 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top + 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left + 1, y - slot->bitmap_top - 1, color2);
            fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color);
        }
        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    return 0;
}

static int Image_width (lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 1) return luaL_error(L, "Argument error: Image:width() must be called with a colon, and takes no arguments.");
    SETDEST
    if(dest) lua_pushnumber(L, dest->imageWidth);
    else lua_pushnumber(L, SCREEN_WIDTH);
    return 1;
}
static int Image_height (lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 1) return luaL_error(L, "Argument error: Image:width() must be called with a colon, and takes no arguments.");
    SETDEST
    if(dest) lua_pushnumber(L, dest->imageHeight);
    else lua_pushnumber(L, SCREEN_HEIGHT);
    return 1;
}
static int Image_save (lua_State *L) {
    if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments");
    const char *filename = luaL_checkstring(L, 2);
    SETDEST
    if (dest) {
        saveImage(filename, dest->data, dest->imageWidth, dest->imageHeight, dest->textureWidth, 1);
    } else {
        saveImage(filename, getVramDisplayBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE, 0);
    }
    return 0;
}

static int Image_free(lua_State *L) {
    freeImage(*toImage(L, 1));
    return 0;
}

static int Image_tostring (lua_State *L) {
    Image_width(L);
    int w = luaL_checkint(L, 2); lua_pop(L, 1);
    Image_height(L);
    int h = luaL_checkint(L, 2); lua_pop(L, 1);

    char buff[32];
    sprintf(buff, "%p", *toImage(L, 1));
    lua_pushfstring(L, "Image (%s) [%d, %d]", buff, w, h);
    return 1;
}
static const luaL_reg Image_methods[] = {
    {"createEmpty", Image_createEmpty},
    {"load", Image_load},
    {"blit", Image_blit},
    {"clear", Image_clear},
    {"fillRect", Image_fillRect},
    {"drawLine", Image_drawLine},
    {"pixel", Image_pixel},
    {"print", Image_print},
    {"fontPrint", Image_fontPrint},
    {"fontPrintCenter", Image_fontPrintCenter},
    {"fontPrintRight", Image_fontPrintRight},
    {"fontPrintRightOutline", Image_fontPrintRightOutline},
    {"fontPrintOutline", Image_fontPrintOutline},
    {"fontPrintCenterOutline", Image_fontPrintCenterOutline},
    {"width", Image_width},
    {"height", Image_height},
    {"save", Image_save},
    {"startBlit", Image_startBlit},
    {"endBlit", Image_endBlit},
    {"fastClear", Image_fastClear},
    {"fastBlit", Image_fastBlit},
    {0,0}
};
static const luaL_reg Image_meta[] = {
    {"__gc", Image_free},
    {"__tostring", Image_tostring},
    {0,0}
};
UserdataRegister(Image, Image_methods, Image_meta)

typedef unsigned int pgeColor;

UserdataStubs(pgeColor, pgeColor)

static int Color_new (lua_State *L) {
    int argc = lua_gettop(L); 
    if (argc != 3 && argc != 4) return luaL_error(L, "Argument error: Color.new(r, g, b, [a]) takes either three color arguments or three color arguments and an alpha value."); 
    
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
    if(argc != 1) return luaL_error(L, "Argument error: color:colors() takes no arguments, and it must be called from an instance with a colon.");
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
    {"flip", lua_flipScreen},
    {"waitVblankStart", lua_waitVblankStart},
    {0,0}
};

void luaGraphics_init(lua_State *L) {
static bool ftInitialized = false;
    
    if (!ftInitialized) {
        FT_Init_FreeType(&ft_library);
        ftInitialized = true;
    }
    
    Image_register(L);
    Color_register(L);
    Font_register(L);
    
    luaL_openlib(L, "screen", Screen_functions, 0);
    luaL_openlib(L, "screen", Image_methods, 0);
    
}

