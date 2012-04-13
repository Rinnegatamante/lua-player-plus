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

#include "Graphics.h"

#include "../Libs/Graphics/Graphics.h"

#include <malloc.h>

PspGeContext __attribute__((aligned(16))) geContext;

UserdataStubs(Image, LPP_Surface*);
UserdataStubs(Color, Color);

/******************************************************************************
 ** Color *********************************************************************
 *******************************************************************************/

static int luaColor_new(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 3 && arguments != 4)
        return luaL_error(L, "Color.new(Red, Green, Blue, [Alpha]) takes 3 or 4 arguments.");
    *pushColor(L) = RGBA(CLAMP(luaL_checkint(L, 1), 0, 255), CLAMP(luaL_checkint(L, 2), 0, 255),
    CLAMP(luaL_checkint(L, 3), 0, 255), (arguments == 4) ? CLAMP(luaL_checkint(L, 4), 0, 255) : 255);
    return 1;
}

static int luaColor_colors(lua_State *L)
{
    if(lua_gettop(L) != 1) return luaL_error(L, "Color:colors() takes no arguments and must be called with a colon.");
    Color s = *toColor(L, 1);
    lua_newtable(L);
    lua_pushstring(L, "r"); lua_pushnumber(L, R(s)); lua_settable(L, -3);
    lua_pushstring(L, "g"); lua_pushnumber(L, G(s)); lua_settable(L, -3);
    lua_pushstring(L, "b"); lua_pushnumber(L, B(s)); lua_settable(L, -3);
    lua_pushstring(L, "a"); lua_pushnumber(L, A(s)); lua_settable(L, -3);

    return 1;
}

static int luaColor_R(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Color:R() takes no arguments and must be called with a colon.");
    lua_pushnumber(L, R(*toColor(L, 1)));
    return 1;
}

static int luaColor_G(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Color:G() takes no arguments and must be called with a colon.");
    lua_pushnumber(L, G(*toColor(L, 1)));
    return 1;
}

static int luaColor_B(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Color:B() takes no arguments and must be called with a colon.");
    lua_pushnumber(L, B(*toColor(L, 1)));
    return 1;
}

static int luaColor_A(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Color:A() takes no arguments and must be called with a colon.");
    lua_pushnumber(L, A(*toColor(L, 1)));
    return 1;
}

static int luaColor__tostring(lua_State *L)
{
    luaColor_colors(L);

    lua_pushstring(L, "r"); lua_gettable(L, -2); int r = luaL_checkint(L, -1); lua_pop(L, 1);
    lua_pushstring(L, "g"); lua_gettable(L, -2); int g = luaL_checkint(L, -1); lua_pop(L, 1);
    lua_pushstring(L, "b"); lua_gettable(L, -2); int b = luaL_checkint(L, -1); lua_pop(L, 1);
    lua_pushstring(L, "a"); lua_gettable(L, -2); int a = luaL_checkint(L, -1); lua_pop(L, 1);
    lua_pop(L, 1);
    lua_pushfstring(L, "Color (r %d, g %d, b %d, a %d)", r, g, b, a);
    return 1;
}

static int luaColor__eq(lua_State *L)
{
    lua_pushboolean(L, *toColor(L, 1) == *toColor(L, 2));
    return 1;
}

static L_CONST luaL_Reg luaColor_methods[] = {
    { "new", luaColor_new },
    { "colors", luaColor_colors },
    { "R", luaColor_R },
    { "G", luaColor_G },
    { "B", luaColor_B },
    { "A", luaColor_A },
    { 0, 0 }
};

static L_CONST luaL_Reg luaColor_meta[] = {
    { "__tostring", luaColor__tostring },
    { "__eq", luaColor__eq },
    { 0, 0 }
};

/*******************************************************************************
 ** intraFont ******************************************************************
 *******************************************************************************/

UserdataStubs(intraFont, LPP_intraFont*);

static int luaIntraFont_load(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Font.load(filename) takes 1 argument.");
    L_CONST char *filename = luaL_checkstring(L, 1);
    if(!LPPG_IsPGF(filename)) return luaL_error(L, "'%s' is a not valid PGF file.");
    LPP_intraFont *f = LPPG_LoadIntraFont(filename);
    if(f == null) return luaL_error(L ,"Cannot load the font '%s'.", filename);
    LPPG_SetIntraFontStyle(f, 0.8f, RGB(255,255,255), RGBA(0,0,0,0), 0.0f, 0x0);
    *pushintraFont(L) = f;
    return 1;
}

static int luaIntraFont_setStyle(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 5 && arguments != 6)
        return luaL_error(L, "intraFont:setStyle(size, textColor, shadowColor, angle, [Options]) takes 4 or 5 arguments and must be called with a colon.");
    LPPG_SetIntraFontStyle(*tointraFont(L, 1), luaL_checknumber(L, 2), *toColor(L, 3), *toColor(L, 4), luaL_checknumber(L, 5), (arguments == 6) ? luaL_checknumber(L, 6) : 0x0);
    return 0;
}

static int luaIntraFont_angle(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 1 && arguments != 2)
        return luaL_error(L, "intraFont:angle([angle]) takes 1 or no arguments and must be called with a colon.");
    LPP_intraFont *f = *tointraFont(L, 1);
    if(arguments == 2) LPPG_SetIntraFontAngle(f, luaL_checknumber(L, 2));
    lua_pushnumber(L, f->angle);
    return 1;
}

static int luaIntraFont_size(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 1 && arguments != 2)
        return luaL_error(L, "intraFont:size([size]) takes 1 or no arguments and must be called with a colon.");
    LPP_intraFont *f = *tointraFont(L, 1);
    if(arguments == 2) f->size = luaL_checknumber(L, 2);
    lua_pushnumber(L, f->size);
    return 1;
}

static int luaIntraFont_setEncoding(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "intraFont:setEncoding(encoding) takes 1 argument and must be called with a colon.");
    LPPG_IntraFontSetEncoding(*tointraFont(L, 1), luaL_checknumber(L, 2));
    return 0;
}

static int luaIntraFont_measureText(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "intraFont:measureText(text) takes 1 argument and must be called with a colon.");
    lua_pushnumber(L, LPPG_IntraFontMeasureText(*tointraFont(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

static int luaIntraFont_measureTextEx(lua_State *L)
{
    if(lua_gettop(L) != 3)
        return luaL_error(L, "intraFont:measureTextEx(text, len) takes 2 arguments and must be called with a colon.");
    lua_pushnumber(L, LPPG_IntraFontMeasureTextEx(*tointraFont(L, 1), luaL_checkstring(L, 2), luaL_checknumber(L, 3)));
    return 1;
}

static int luaIntraFont_measureTextUCS2(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "intraFont:measureTextUCS2(text) takes 1 argument and must be called with a colon.");

    int i, n = luaL_getn(L, 2);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 2, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    float r = LPPG_IntraFontMeasureTextUCS2(*tointraFont(L, 1), text);

    free(text);

    lua_pushnumber(L, r);

    return 1;
}

static int luaIntraFont_setAltFont(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "intraFont:setAltFont(font) takes 1 arguments and must be called with a colon.");
    LPPG_IntraFontSetAltFont(*tointraFont(L, 1), *tointraFont(L, 2));
    return 0;
}

static int luaIntraFont_measureTextUCS2Ex(lua_State *L)
{
    if(lua_gettop(L) != 3)
        return luaL_error(L, "intraFont:measureTextUCS2Ex(text, len) takes 2 argument and must be called with a colon.");

    int i, n = luaL_getn(L, 2);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 2, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    float r = LPPG_IntraFontMeasureTextUCS2Ex(*tointraFont(L, 1), text, luaL_checknumber(L, 3));

    free(text);

    lua_pushnumber(L, r);

    return 1;
}

static int luaIntraFont__gc(lua_State *L)
{
    LPPG_FreeIntraFont(*tointraFont(L, 1));
    return 0;
}

static L_CONST luaL_reg luaIntraFont_methods[] = {
    { "load", luaIntraFont_load },
    { "setStyle", luaIntraFont_setStyle },
    { "angle", luaIntraFont_angle },
    { "size", luaIntraFont_size },
    { "setEncoding", luaIntraFont_setEncoding },
    { "setAltFont", luaIntraFont_setAltFont },
    { "measureText", luaIntraFont_measureText },
    { "measureTextEx", luaIntraFont_measureTextEx },
    { "measureTextUCS2", luaIntraFont_measureTextUCS2 },
    { "measureTextUCS2Ex", luaIntraFont_measureTextUCS2Ex },
    { 0, 0 }
};

static L_CONST luaL_reg luaIntraFont_meta[] = {
    { "__gc", luaIntraFont__gc },
    { 0, 0 }
};

UserdataRegister(intraFont, luaIntraFont_methods, luaIntraFont_meta);

/*******************************************************************************
 ** Font ***********************************************************************
 *******************************************************************************/

UserdataStubs(Font, LPP_TrueTypeFont*);

static int luaFont_load(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 2 && arguments != 3)
        return luaL_error(L, "Font.load(filename, size, [SizeType]) takes 2 or 3 arguments.");
    LPP_TrueTypeFont *f = LPPG_LoadTrueType(luaL_checkstring(L, 1), luaL_checknumber(L, 2), (arguments == 3) ? CLAMP(luaL_checkint(L, 3), 0, 1) : LPP_FONT_SIZE_POINTS);
    if(f == null) return luaL_error(L, "Cannot load the font '%s'.", luaL_checknumber(L, 2));
    *pushFont(L) = f;
    return 1;
}

static int luaFont_createProportional(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 1 && arguments != 2)
        return luaL_error(L, "Font.createProportional(size, [SizeType]) takes 1 or 2 arguments.");
    LPP_TrueTypeFont *f = LPPG_TTFCreateProportional(luaL_checknumber(L, 1), (arguments == 2) ? CLAMP(luaL_checkint(L, 2), 0, 1) : LPP_FONT_SIZE_POINTS);
    if(f == null) return luaL_error(L, "Cannot create the proportional.");
    *pushFont(L) = f;
    return 1;
}

static int luaFont_createMonoSpaced(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 1 && arguments != 2)
        return luaL_error(L, "Font.createMonoSpaced(size, [SizeType]) takes 1 or 2 arguments.");
    LPP_TrueTypeFont *f = LPPG_TTFCreateMonoSpaced(luaL_checknumber(L, 1), (arguments == 2) ? CLAMP(luaL_checkint(L, 2), 0, 1) : LPP_FONT_SIZE_POINTS);
    if(f == null) return luaL_error(L, "Cannot create the mono spaced.");
    *pushFont(L) = f;
    return 1;
}

static int luaFont_angle(lua_State *L)
{
    u8 args = lua_gettop(L);
    if(args != 1 && args != 2)
        return luaL_error(L, "Font:angle([angle]) takes 1 or no arguments and must be called with a colon.");
    LPP_TrueTypeFont *f = *toFont(L, 1);
    if(args == 2) LPPG_TTFSetAngle(f, luaL_checknumber(L, 2));
    lua_pushnumber(L, f->angle);
    return 1;
}

static int luaFont_size(lua_State *L)
{
    u8 args = lua_gettop(L);
    if(args != 1 && args > 2)
        return luaL_error(L, "Font:size([size], [sizeType]) takes a maximum of 2 arguments and must be called with a colon.");
    LPP_TrueTypeFont *f = *toFont(L, 1);
    if(args >= 2) LPPG_TTFSetSize(f, luaL_checknumber(L, 2), (args == 3) ? CLAMP(luaL_checkint(L, 3), 0, 1) : LPP_FONT_SIZE_POINTS);
    lua_pushnumber(L, LPPG_TTFGetSize(f));
    return 1;
}

static int luaFont_setCharSize(lua_State *L)
{
    if(lua_gettop(L) != 5)
        return luaL_error(L, "Font:setCharSize(width, height, dpiX, dpiY) takes 4 arguments and must be called with a colon.");
    lua_pushnumber(L, LPPG_TTFSetCharSize(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), *toFont(L, 1)));
    return 1;
}

static int luaFont_getTextSize(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "Font:getTextSize(text) takes 1 argument and must be called with a colon.");

    LPP_TrueTypeFont *f = *toFont(L, 1);
    L_CONST char *t = luaL_checkstring(L, 2);

    u32 tl = strlen(t);

    FT_GlyphSlot slot = f->face->glyph;

    u32 x = 0, y = 0, maxHeight = 0, n;
    for(n = 0; n < tl; n++)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(f->face, t[n]);
        FT_Error error = FT_Load_Glyph(f->face, glyph_index, FT_LOAD_DEFAULT);
        if(error) continue;
        error = FT_Render_Glyph(slot, ft_render_mode_normal);
        if(error) continue;
        if(slot->bitmap.rows > maxHeight) maxHeight = slot->bitmap.rows;

        x += slot->advance.x >> 6;
        y += slot->advance.y >> 6;
    }

    lua_newtable(L);
    lua_pushstring(L, "width");  lua_pushnumber(L, x); lua_settable(L, -3);
    lua_pushstring(L, "height"); lua_pushnumber(L, maxHeight); lua_settable(L, -3);

    return 1;
}

static int luaFont__gc(lua_State *L)
{
    LPPG_FreeTrueType(*toFont(L, 1));
    return 0;
}

static int luaFont__tostring(lua_State *L)
{
    LPP_TrueTypeFont *f = *toFont(L, 1);
    lua_pushstring(L, f->name);
    return 1;
}

static L_CONST luaL_Reg luaFont_methods[] = {
    { "load", luaFont_load },
    { "createProportional", luaFont_createProportional },
    { "createMonoSpaced", luaFont_createMonoSpaced },
    { "angle", luaFont_angle },
    { "size", luaFont_size },
    { "setCharSize", luaFont_setCharSize },
    { "getTextSize", luaFont_getTextSize },
    { 0, 0 }
};

 static L_CONST luaL_Reg luaFont_meta[] = {
    { "__gc", luaFont__gc },
    { "__tostring", luaFont__tostring },
    { 0, 0 }
 };

UserdataRegister(Font, luaFont_methods, luaFont_meta);

/******************************************************************************
 ** Image *********************************************************************
 *******************************************************************************/

static int luaImage_load(lua_State *L)
{
    if(lua_gettop(L) != 1) return luaL_error(L, "Image.load(filename) takes 1 argument.");
    L_CONST char *filename = luaL_checkstring(L, 1);
    LPP_Surface *img = null;
    if(LPPG_IsInPsar(filename))
        img = LPPG_LoadImageFPsar(filename);
    else
        img = LPPG_LoadImage(filename);
    if(img == null) return luaL_error(L, "Cannot load the image '%s'.", filename);
    *pushImage(L) = img;
    return 1;
}

static int luaImage_loadGifFrame(lua_State *L)
{
    if(lua_gettop(L) != 2)
        return luaL_error(L, "Image.loadGifFrame(filename, n) takes 2 arguments.");
    LPP_Surface *img = LPPG_LoadImageGIFframe(luaL_checkstring(L, 1), luaL_checknumber(L, 2));
    if(img == null) return luaL_error(L, "Cannot load the image '%s'.", luaL_checkstring(L, 1));
    *pushImage(L) = img;
    return 1;
}

static int luaImage_gifFramesCount(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image.gifFramesCount(filename) takes 1 argument.");
    u32 count = LPPG_GifFramesCount(luaL_checkstring(L,1));
    lua_pushnumber(L, count);
    return 1;
}

static int luaImage_createEmpty(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments > 3 || arguments < 2) return luaL_error(L, "Image.createEmpty(width, height, [Color]) takes 2 arguments");
    LPP_Surface *img = LPPG_CreateSurface(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    if(arguments == 3) LPPG_ClearSurface(img, *toColor(L, 3));
	if(img == null) return luaL_error(L, "Cannot create the image.");
    *pushImage(L) = img;
    return 1;
}

static int luaImage_clear(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments < 1 || arguments > 2)
        return luaL_error(L, "Image:clear([Color]) takes 1 or no arguments ad must be called with a colon.");

    if(lua_istable(L, 1)) {
        LPPG_ClearScreen((arguments == 2) ? *toColor(L, 2) : 0);
    } else {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_ClearSurface(dest, (arguments == 1) ? RGB(0,0,0) : *toColor(L, 2));
    }
    return 0;
}

static int luaImage_fillRect(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments < 5 || arguments > 6)
        return luaL_error(L, "Image:fillRect(x, y, width, height, [Color]) takes 4 or 5 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_FillScreenRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), (arguments == 6) ? *toColor(L, 6) : RGB(0,0,0));
    else {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_FillSurfaceRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), (arguments == 6) ? *toColor(L, 6) : RGB(0,0,0), dest);
    }
    return 0;
}

static int luaImage_blit(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments < 4 || arguments > 10)
        return luaL_error(L, "Image:blit(x, y, image, [Alpha], [Angle], [srcX], [srcY], [srcW], [srcH]) takes a minimum of 3 arguments and must be called with a colon.");
    LPP_Surface *source = *toImage(L, 4);
	float angle = (arguments >= 6) ? luaL_checknumber(L, 6) : 0.0f;
	if(angle > 360.0f) angle -= (360.0f * (int)(angle / 360.0f));
	
    if(lua_istable(L, 1)) {
        LPPG_BlitSurfaceScreen(luaL_checknumber(L, 2), luaL_checknumber(L, 3), source,
                    (arguments >= 5) ? CLAMP(luaL_checknumber(L, 5), 0, 255) : 255, angle,
                    (arguments >= 7) ? luaL_checknumber(L, 7) : 0,
                    (arguments >= 8) ? luaL_checknumber(L, 8) : 0,
                    (arguments >= 9) ? luaL_checknumber(L, 9) : source->width,
                    (arguments == 10) ? luaL_checknumber(L, 10) : source->height);
    } else {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_BlitSurfaceSurface(luaL_checknumber(L, 2), luaL_checknumber(L, 3), source,
                           (arguments >= 7) ? luaL_checknumber(L, 7) : 0,
                           (arguments >= 8) ? luaL_checknumber(L, 8) : 0,
                           (arguments >= 9) ? luaL_checknumber(L, 9) : source->width,
                           (arguments >= 10) ? luaL_checknumber(L, 10) : source->height, dest);
    }

    return 0;
}

static int luaImage_resized(lua_State *L)
{
    if(lua_gettop(L) != 3)
        return luaL_error(L, "Image:resized(width, height) takes 2 arguments and must be called with a colon.");
    *pushImage(L) = LPPG_SurfaceResized(*toImage(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 1;
}

static int luaImage_toVram(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:toVram() takes no arguments and must be called with a colon.");
    LPPG_SurfaceToVram(*toImage(L, 1));
    return 0;
}

static int luaImage_toRam(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:toRam() takes no arguments and must be called with a colon.");
    LPPG_SurfaceToRam(*toImage(L, 1));
    return 0;
}

static int luaImage_width(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:w() takes no arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        lua_pushnumber(L, PSP_SCREEN_WIDTH);
    else {
        LPP_Surface *source = *toImage(L, 1);
        lua_pushnumber(L, source->width);
    }
    return 1;
}

static int luaImage_height(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:h() takes no arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        lua_pushnumber(L, PSP_SCREEN_HEIGHT);
    else {
        LPP_Surface *source = *toImage(L, 1);
        lua_pushnumber(L, source->height);
    }
    return 1;
}

static int luaImage_swizzle(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:swizzle() takes no arguments and must be called with a colon.");
    LPP_Surface *source = *toImage(L, 1);
    LPPG_SwizzleSurface(source);
    return 0;
}

static int luaImage_unSwizzle(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:unSwizzle() takes no arguments and must be called with a colon.");
    LPP_Surface *source = *toImage(L, 1);
    LPPG_UnSwizzleSurface(source);
    return 0;
}

static int luaImage_dGradientRect(lua_State *L)
{
    if(lua_gettop(L) != 9)
        return luaL_error(L, "screen:gradientRect(x, y, width, height, Color1, Color2, Color3, Color4) takes 8 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_DrawGradientRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                              *toColor(L, 6), *toColor(L, 7), *toColor(L, 8), *toColor(L, 9));
    return 0;
}

static int luaImage_rect(lua_State *L)
{
    if(lua_gettop(L) != 6)
        return luaL_error(L, "Image:rect(x, y, width, height, Color) takes 5 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_ScreenRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4) - 1, luaL_checknumber(L, 5) - 1, *toColor(L, 6));
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_SurfaceRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4) - 1, luaL_checknumber(L, 5) - 1, *toColor(L, 6), dest);
    }
    return 0;
}

static int luaImage_circle(lua_State *L)
{
    if(lua_gettop(L) != 5)
        return luaL_error(L, "Image:circle(x, y, radius, Color) takes 4 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_ScreenCircle(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *toColor(L, 5));
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_SurfaceCircle(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *toColor(L, 5), dest);
    }
    return 0;
}

static int luaImage_fillCircle(lua_State *L)
{
    if(lua_gettop(L) != 5)
        return luaL_error(L, "Image:fillCircle(x, y, radius, Color) takes 4 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_FillScreenCircle(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *toColor(L, 5));
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_FillSurfaceCircle(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *toColor(L, 5), dest);
    }
    return 0;
}

static int luaImage_putPixel(lua_State *L)
{
    if(lua_gettop(L) != 4)
        return luaL_error(L, "Image:putPixel(x, y, Color) takes 3 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_PutScreenPixel(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toColor(L, 4));
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_PutSurfacePixel(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toColor(L, 4), dest);
    }
    return 0;
}

static int luaImage_getPixel(lua_State *L)
{
    if(lua_gettop(L) != 3)
        return luaL_error(L, "Image:getPixel(x, y) takes 2 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        *pushColor(L) = LPPG_GetScreenPixel(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    else
    {
        LPP_Surface *dest = *toImage(L ,1);
        *pushColor(L) = LPPG_GetSurfacePixel(luaL_checknumber(L, 2), luaL_checknumber(L, 3), dest);
    }
    return 1;
}

static int luaImage_drawLine(lua_State *L)
{
    if(lua_gettop(L) != 6)
        return luaL_error(L, "Image:drawLine(x1, y1, x2, y2, Color) takes 5 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_ScreenLine(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), *toColor(L, 6));
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_SurfaceLine(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), *toColor(L, 6), dest);
    }
    return 0;
}

static int luaImage_negative(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:negative() takes no arguments and must be called with a colon.");
    if(lua_istable(L, 1))
        *pushImage(L) = LPPG_ScreenNegative();
    else
    {
        LPP_Surface *dest = *toImage(L ,1);
        *pushImage(L) = LPPG_SurfaceNegative(dest);
    }
    return 1;
}

static int luaImage_save(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 2 && arguments != 3)
        return luaL_error(L, "Image:save(filename, [Format]) takes 1 argument and must be called with a colon.");
    if(lua_istable(L, 1))
        LPPG_SaveScreen(luaL_checkstring(L, 2), (arguments == 3) ? luaL_checknumber(L, 3) : LPP_Image_BMP);
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_SaveSurface(luaL_checkstring(L, 2), (arguments == 3) ? luaL_checknumber(L, 3) : LPP_Image_BMP, dest);
    }
    return 0;
}

static int luaImage_clone(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:clone() takes no arguments and must be called with a colon.");
    LPP_Surface *dest = *toImage(L, 1);
    *pushImage(L) = LPPG_CopySurface(dest);
    return 1;
}

static int luaImage_flipVertical(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Image:flipVertical() takes no arguments and must be called with a colon.");
    LPP_Surface *dest = *toImage(L, 1);
    *pushImage(L) = LPPG_FlipSurfaceVertical(dest);
    return 1;
}

static int luaImage_print(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 4 && arguments != 5)
        return luaL_error(L, "Image:print(x, y, Text, [Color]) takes 3 or 4 arguments and must be called with a colon.");
    if(lua_istable(L, 1))
    {
        LPPG_PrintTextScreen(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4), (arguments == 5) ? *toColor(L, 5) : RGB(255,255,255));
    } else {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_PrintTextSurface(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4), (arguments == 5) ? *toColor(L, 5) : RGB(255,255,255), dest);
    }
    return 0;
}

static int luaImage_printUnderline(lua_State *L)
{
    short arguments = lua_gettop(L);
	if(arguments != 4 && arguments > 6)
	{
	    return luaL_error(L, "Image:printUnderline(x, y, text, [Color], [LineColor]) takes a maximum of 5 arguments and must be called with a colon.");
	}
	if(lua_istable(L,1))
	{
	    LPPG_PrintTextScreen(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4), (arguments >= 5) ? *toColor(L, 5) : RGB(255,255,255));
		LPPG_ScreenLine(luaL_checknumber(L, 2) - 2, luaL_checknumber(L, 3) + 9, 8 * strlen(luaL_checkstring(L, 4)), luaL_checknumber(L, 3) + 9, (arguments == 6) ? *toColor(L, 6) : RGB(255,255,255));
	}
	else
	{
	    LPP_Surface *dest = *toImage(L, 1);
		LPPG_PrintTextSurface(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4), (arguments >= 5) ? *toColor(L, 5) : RGB(255,255,255), dest);
		LPPG_SurfaceLine(luaL_checknumber(L, 2) - 2, luaL_checknumber(L, 3) + 9, 8 * strlen(luaL_checkstring(L, 4)), luaL_checknumber(L, 3) + 9, (arguments == 6) ? *toColor(L, 6) : RGB(255,255,255), dest);
	}
	return 0;
}

static int luaImage_printColumn(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments != 5 && arguments > 6)
	{
	    return luaL_error(L, "Image:printColumn(x, y, text, width, [Color]) takes a maximum of 5 arguments and must be called with a colon.");
	}
	if(lua_istable(L,1))
	{
	    int x = luaL_checknumber(L, 2);
		int y = luaL_checknumber(L, 3);
		int width = luaL_checknumber(L, 5);
	    L_CONST char *text = luaL_checkstring(L, 4);
	    size_t len = strlen(text);
		size_t i;
		for(i = 0; i < len; i++) {
		    char tp[2] = { text[i], '\0' };
		    LPPG_PrintTextScreen(x, y + i * width, tp, (arguments == 6) ? *toColor(L,6) : RGB(255,255,255));
		}
	}
	else
	{
	    LPP_Surface *dest = *toImage(L, 1);
		int x = luaL_checknumber(L, 2);
		int y = luaL_checknumber(L, 3);
		int width = luaL_checknumber(L, 5);
	    L_CONST char *text = luaL_checkstring(L, 4);
	    size_t len = strlen(text);
		size_t i;
		for(i = 0; i < len; i++) {
		    char tp[2] = { text[i], '\0' };
		    LPPG_PrintTextSurface(x, y + i * width, tp, (arguments == 6) ? *toColor(L,6) : RGB(255,255,255), dest);
		}
	}
	return 0;
}

static int luaImage_intraFontPrint(lua_State *L)
{
    if(lua_gettop(L) != 7)
            return luaL_error(L, "screen:intraFontPrint(x, y, font, text, textColor, shadowColor) takes 6 arguments and must be called with a colon.");
    LPPG_IntraFontPrint(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *tointraFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6), *toColor(L, 7));
    return 0;
}

static int luaImage_intraFontPrintEx(lua_State *L)
{
    if(lua_gettop(L) != 8)
        return luaL_error(L, "screen:intraFontPrintEx(x, y, font, text, textColor, shadowColor, len) takes 7 arguments and must be called with a colon.");
    LPPG_IntraFontPrintEx(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *tointraFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6), *toColor(L, 7), luaL_checknumber(L, 8));
    return 0;
}

static int luaImage_intraFontPrintUCS2(lua_State *L)
{
    if(lua_gettop(L) != 7)
        return luaL_error(L, "screen:intraFontPrintUCS2(x, y, font, text, textColor, shadowColor) takes 6 arguments and must be called with a colon.");

    int i, n = luaL_getn(L, 5);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 5, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    LPPG_IntraFontPrintUCS2(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *tointraFont(L, 4), text, *toColor(L, 6), *toColor(L, 7));

    free(text);

    return 0;
}

static int luaImage_intraFontPrintUCS2Ex(lua_State *L)
{
    if(lua_gettop(L) != 8)
        return luaL_error(L, "screen:intraFontPrintUCS2(x, y, font, text, textColor, shadowColor, len) takes 7 arguments and must be called with a colon.");

    int i, n = luaL_getn(L, 5);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 5, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    LPPG_IntraFontPrintUCS2Ex(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *tointraFont(L, 4), text, *toColor(L, 6), *toColor(L, 7), luaL_checknumber(L, 8));

    free(text);

    return 0;
}

static int luaImage_intraFontPrintColumn(lua_State *L)
{
    if(lua_gettop(L) != 8)
        return luaL_error(L, "screen:intraFontPrintColumn(x, y, width, font, text, textColor, shadowColor) takes 7 arguments and must be called with a colon.");
    LPPG_IntraFontPrintColumn(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *tointraFont(L, 5), luaL_checkstring(L, 6), *toColor(L, 7), *toColor(L, 8));
    return 0;
}

static int luaImage_intraFontPrintColumnEx(lua_State *L)
{
    if(lua_gettop(L) != 9)
        return luaL_error(L, "screen:intraFontPrintColumnEx(x, y, width, font, text, textColor, shadowColor, len) takes 8 arguments and must be called with a colon.");
    LPPG_IntraFontPrintColumnEx(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *tointraFont(L, 5), luaL_checkstring(L, 6), *toColor(L, 7), *toColor(L, 8), luaL_checknumber(L, 9));
    return 0;
}

static int luaImage_intraFontPrintColumnUCS2(lua_State *L)
{
    if(lua_gettop(L) != 8)
        return luaL_error(L, "screen:intraFontPrintUCS2(x, y, width, font, text, textColor, shadowColor) takes 7 arguments and must be called with a colon.");

    int i, n = luaL_getn(L, 6);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 6, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    LPPG_IntraFontPrintColumnUCS2(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *tointraFont(L, 5), text, *toColor(L, 7), *toColor(L, 8));

    free(text);

    return 0;
}

static int luaImage_intraFontPrintColumnUCS2Ex(lua_State *L)
{
    if(lua_gettop(L) != 9)
        return luaL_error(L, "screen:intraFontPrintUCS2Ex(x, y, width, font, text, textColor, shadowColor, len) takes 8 arguments and must be called with a colon.");

    int i, n = luaL_getn(L, 6);
    u16 *text = (u16*)memalign(16, (n + 1) * sizeof(u16));

    for(i = 0; i < n; i++) {
        lua_rawgeti(L, 6, i + 1);
        text[i] = (u16)luaL_checknumber(L, -1);
    }

    text[n] = 0;

    LPPG_IntraFontPrintColumnUCS2Ex(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *tointraFont(L, 5), text, *toColor(L, 7), *toColor(L, 8), luaL_checknumber(L, 9));

    free(text);

    return 0;
}

static int luaImage_trueTypePrint(lua_State *L)
{
    if(lua_gettop(L) != 6)
    {
        return luaL_error(L, "Image:fontPrint(x, y, font, text, color) takes 5 arguments and must be called with a colon.");
    }

    if(lua_istable(L, 1))
    {
        LPPG_PrintTTFScreen(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6));
    }
    else
    {
        LPP_Surface *dest = *toImage(L, 1);
        LPPG_PrintTTFSurface(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6), dest);
    }

    return 0;
}

static int luaImage_trueTypePrintFixed(lua_State *L)
{
    if(lua_gettop(L) != 6)
	{
	    return luaL_error(L, "screen:fontPrintFixed(x, y, font, text, color) takes 5 arguments and must be called with a colon.");
	}
	
	if(lua_istable(L, 1))
	{
	    LPPG_PrintTTFScreenFixed(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6));
	}
	else
	{
	    LPP_Surface *dest = *toImage(L, 1);
		LPPG_PrintTTFSurfaceFixed(luaL_checknumber(L, 2), luaL_checknumber(L, 3), *toFont(L, 4), luaL_checkstring(L, 5), *toColor(L, 6), dest);
	}
	
	return 0;
}

static int luaImage_checkFormat(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Image.checkFormat(filename, format) takes 2 arguments.");
    }

    u32 format = CLAMP(luaL_checkint(L, 2), 1, 5);
    L_CONST char *fname = luaL_checkstring(L, 1);

    u8 r = 0;

    switch(format)
    {
        case LPP_Image_PNG : r = LPPG_IsPNG(fname); break;
        case LPP_Image_JPG : r = LPPG_IsJPG(fname); break;
        case LPP_Image_BMP : r = LPPG_IsBMP(fname); break;
        case LPP_Image_TGA : r = LPPG_IsTGA(fname); break;
        case LPP_Image_GIF : r = LPPG_IsGIF(fname); break;
    }

    lua_pushboolean(L, r);

    return 1;
}

static int luaImage__gc(lua_State *L)
{
    LPPG_FreeSurface(*toImage(L, 1));
    return 0;
}

static int luaImage__eq(lua_State *L)
{
    LPP_Surface *a = *toImage(L, 1);
    LPP_Surface *b = *toImage(L, 2);
    lua_pushboolean(L, LPPG_SurfaceEquals(a, b));
    return 1;
}

static L_CONST luaL_Reg luaImage_methods[] = {
    { "load", luaImage_load },
    { "loadGifFrame", luaImage_loadGifFrame },
    { "gifFramesCount", luaImage_gifFramesCount },
    { "createEmpty", luaImage_createEmpty},
    { "blit", luaImage_blit },
    { "clear", luaImage_clear },
    { "fillRect", luaImage_fillRect },
    { "resized", luaImage_resized },
    { "toVram", luaImage_toVram },
    { "toRam", luaImage_toRam },
    { "w", luaImage_width },
    { "h", luaImage_height },
    { "swizzle", luaImage_swizzle },
    { "unSwizzle", luaImage_unSwizzle },
    { "gradientRect", luaImage_dGradientRect },
    { "print", luaImage_print },
	{ "printUnderline", luaImage_printUnderline },
	{ "printColumn", luaImage_printColumn },
    { "rect", luaImage_rect },
    { "drawLine", luaImage_drawLine },
    { "putPixel", luaImage_putPixel },
    { "getPixel", luaImage_getPixel },
    { "fillCircle", luaImage_fillCircle },
    { "circle", luaImage_circle },
    { "negative", luaImage_negative },
    { "save", luaImage_save },
    { "clone", luaImage_clone },
    { "flipVertical", luaImage_flipVertical },
    { "intraFontPrint", luaImage_intraFontPrint },
    { "intraFontPrintEx", luaImage_intraFontPrintEx },
    { "intraFontPrintUCS2", luaImage_intraFontPrintUCS2 },
    { "intraFontPrintUCS2Ex", luaImage_intraFontPrintUCS2Ex },
    { "intraFontPrintColumn", luaImage_intraFontPrintColumn },
    { "intraFontPrintColumnEx", luaImage_intraFontPrintColumnEx },
    { "intraFontPrintColumnUCS2", luaImage_intraFontPrintColumnUCS2 },
    { "intraFontPrintColumnUCS2Ex", luaImage_intraFontPrintColumnUCS2Ex },
    { "fontPrint", luaImage_trueTypePrint },
	{ "fontPrintFixed", luaImage_trueTypePrintFixed },
    { "checkFormat", luaImage_checkFormat },
    { 0, 0 }
};

static L_CONST luaL_Reg luaImage_meta[] = {
    { "__gc", luaImage__gc },
    { "__eq", luaImage__eq },
    { 0, 0 }
};

static int luaScreen_flip(lua_State *L)
{
    if(lua_gettop(L) != 0)
        return luaL_error(L, "screen.flip() takes no arguments.");
    LPPG_FlipScreen();
    return 0;
}

static int luaScreen_waitvbs(lua_State *L)
{
    short arguments = lua_gettop(L);
    if(arguments > 1)
        return luaL_error(L, "screen.waitVblankStart([Milliseconds]) takes 0 or 1 arguments.");
    if(arguments == 1) {
        int i;
        int ms = luaL_checknumber(L, 1);
        for(i = 0; i < ms; i++) sceDisplayWaitVblankStart();
    } else sceDisplayWaitVblankStart();
    return 0;
}

static L_CONST luaL_Reg luaScreen_methods[] = {
    { "flip", luaScreen_flip },
    { "waitVblankStart", luaScreen_waitvbs },
    { 0, 0 }
};

UserdataRegister(Image, luaImage_methods, luaImage_meta);
UserdataRegister(Color, luaColor_methods, luaColor_meta);

/******************************************************************************
 ** 3D ************************************************************************
 *******************************************************************************/

static int luaGu_clearColor(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.clearColor(Color) takes 1 argument.");
    }

    sceGuClearColor(*toColor(L, 1));
    return 0;
}

static int luaGu_clearDepth(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.clearDepth(Depth) takes 1 argument.");
    }

    sceGuClearDepth(luaL_checkint(L, 1));
    return 0;
}

static int luaGu_clear(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.clear(flags) takes 1 argument.");
    }

    sceGuClear(luaL_checkint(L, 1));
    return 0;
}

static int luaGum_matrixMode(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gum.matrixMode(mode) takes 1 argument.");
    }

    sceGumMatrixMode(luaL_checkint(L, 1));
    return 0;
}

static int luaGum_loadIdentity(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "Gum.loadIdentity() takes no arguments.");
    }

    sceGumLoadIdentity();
    return 0;
}

static int luaGum_perspective(lua_State *L)
{
    if(lua_gettop(L) != 4)
    {
        return luaL_error(L, "Gum.perspective(fovy, aspect, near, far) takes 4 arguments.");
    }

    sceGumPerspective(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    return 0;
}

static int luaGum_translate(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Gum.translate(x, y, x) takes 3 arguments.");
    }

    ScePspFVector3 v = {
        luaL_checknumber(L, 1),
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3)
    };

    sceGumTranslate(&v);
    return 0;
}

static int luaGumRotateXYZ(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Gum.rotateXYZ(x, y, z) takes 3 arguments.");
    }

    ScePspFVector3 v = {
        luaL_checknumber(L, 1),
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3)
    };

    sceGumRotateXYZ(&v);
    return 0;
}

static int luaGu_texImage(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.texImage(image) takes 1 argument.");
    }

    LPP_Surface *s = *toImage(L, 1);
    sceGuTexImage(0, s->realW, s->realH, s->realW, (void*)s->pixels);
    return 0;
}

static int luaGu_texFunc(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Gu.texFunc(tfx, tcc) takes 2 arguments.");
    }

    sceGuTexFunc(luaL_checkint(L, 1), luaL_checkint(L, 2));
    return 0;
}

static int luaGu_texEnvColor(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.texEnvColor(color) takes 1 argument.");
    }

    sceGuTexEnvColor(*toColor(L, 1));
    return 0;
}

static int luaGu_texFilter(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Gu.texFilter(min, mag) takes 2 arguments.");
    }

    sceGuTexFilter(luaL_checkint(L, 1), luaL_checkint(L, 2));
    return 0;
}

static int luaGu_texScale(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Gu.texScale(u, v) takes 2 arguments.");
    }

    sceGuTexScale(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    return 0;
}

static int luaGu_texOffset(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Gu.texOffset(u, v) takes 2 arguments.");
    }

    sceGuTexOffset(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    return 0;
}

static int luaGu_ambientColor(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.ambientColor(color) takes 1 argument.");
    }

    sceGuAmbientColor(*toColor(L, 1));
    return 0;
}

static int luaGu_ambient(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.ambient(color) takes 1 argument.");
    }

    sceGuAmbient(*toColor(L, 1));
    return 0;
}

static int luaGu_enable(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.enable(flags) takes 1 argument.");
    }

    sceGuEnable(luaL_checkint(L, 1));
    return 0;
}

static int luaGu_disable(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.disable(flags) takes 1 argument.");
    }

    sceGuDisable(luaL_checkint(L, 1));
    return 0;
}

static int luaGu_blendFunc(lua_State *L)
{
    if(lua_gettop(L) != 5)
    {
        return luaL_error(L, "Gu.blendFunc(op, src, dst, srcfix, dstfix) takes 5 arguments.");
    }

    sceGuBlendFunc(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5));
    return 0;
}

static int luaGu_light(lua_State *L)
{
    if(lua_gettop(L) != 6)
    {
        return luaL_error(L, "Gu.light(light, type, components, x, y, x) takes 6 arguments.");
    }

    ScePspFVector3 v = {
        luaL_checknumber(L, 4),
        luaL_checknumber(L, 5),
        luaL_checknumber(L, 6)
    };

    sceGuLight(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), &v);
    return 0;
}

static int luaGu_lightAtt(lua_State *L)
{
    if(lua_gettop(L) != 4)
    {
        return luaL_error(L, "Gu.lightAtt(light, attend0, attend1, attend2) takes 4 arguments.");
    }

    sceGuLightAtt(luaL_checkint(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    return 0;
}

static int luaGu_lightColor(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Gu.lightColor(light, component, color) takes 3 arguments.");
    }

    sceGuLightColor(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checknumber(L, 3));
    return 0;
}

static int luaGu_lightMode(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Gu.lightMod(mode) takes 1 arguments.");
    }

    sceGuLightMode(luaL_checkint(L, 1));
    return 0;
}

static int luaGu_lightSpot(lua_State *L)
{
    if(lua_gettop(L) != 6)
    {
        return luaL_error(L, "Gu.lightSpot(light, x, y, z, exponent, cutoff) takes 6 arguments.");
    }

    ScePspFVector3 v = {
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3),
        luaL_checknumber(L, 4)
    };

    sceGuLightSpot(luaL_checkint(L, 1), &v, luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    return 0;
}

static int luaGum_drawArray(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Gum.drawArray(prim, vtype, vertices) takes 3 arguments.");
    }

    int prim = luaL_checkint(L, 1);
    int vtype = luaL_checkint(L, 2);

    if(!lua_istable(L, 3))
    {
        return luaL_error(L, "Vertices must be a table.");
    }

    int n = luaL_getn(L, 3);

    int quads = 0;
    int colorLuaIndex = -1;

    if(vtype & GU_TEXTURE_32BITF) quads += 2;

    if(vtype & GU_COLOR_8888)
    {
        quads++;
        colorLuaIndex = quads;
    }

    if(vtype & GU_NORMAL_32BITF) quads += 3;
    if(vtype & GU_VERTEX_32BITF) quads += 3;

    void *vertices = memalign(16, n * quads*4);
    float *vertex = (float*)vertices;

    int i;
    for(i = 1; i <= n; ++i)
    {
        lua_rawgeti(L, 3, i);
        int n2 = luaL_getn(L, -1);

        if(n2 != quads)
        {
            free(vertices);
            return luaL_error(L, "Wrong number of vertex components.");
        }

        int j;
        for(j = 1; j <= n2; ++j)
        {
            lua_rawgeti(L, -1, j);

            if(j != colorLuaIndex) {
                *vertex = luaL_checknumber(L, -1);
            } else {
                *((Color*)vertex) = *toColor(L, -1);
            }

            lua_pop(L, 1);
            vertex++;
        }

        lua_pop(L, 1);
    }

    sceKernelDcacheWritebackInvalidateAll();
    sceGumDrawArray(prim, vtype, n, null, vertices);
    free(vertices);
    return 0;
}

static int luaGu_start3D(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "Gu.start3d() takes no arguments.");
    }

    sceGeSaveContext(&geContext);
    LPPG_StartDrawing();
    return 0;
}

static int luaGu_end3D(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "Gu.end3d() takes no arguments.");
    }

    LPPG_EndDrawing();
    sceGeRestoreContext(&geContext);
    return 0;
}

static L_CONST luaL_reg luaGu_methods[] = {
    { "clearColor", luaGu_clearColor },
    { "clearDepth", luaGu_clearDepth },
    { "clear", luaGu_clear },
    { "texImage", luaGu_texImage },
    { "texFunc", luaGu_texFunc },
    { "texEnvColor", luaGu_texEnvColor },
    { "texFilter", luaGu_texFilter },
    { "texScale", luaGu_texScale },
    { "texOffset", luaGu_texOffset },
    { "ambientColor", luaGu_ambientColor },
    { "ambient", luaGu_ambient },
    { "enable", luaGu_enable },
    { "disable", luaGu_disable },
    { "blendFunc", luaGu_blendFunc },
    { "light", luaGu_light },
    { "lightAtt", luaGu_lightAtt },
    { "lightColor", luaGu_lightColor },
    { "lightMode", luaGu_lightMode },
    { "lightSpot", luaGu_lightSpot },
    { "start3d", luaGu_start3D },
    { "end3d", luaGu_end3D },
    { 0, 0}
};

static L_CONST luaL_reg luaGum_methods[] = {
    { "matrixMode", luaGum_matrixMode },
    { "loadIdentity", luaGum_loadIdentity },
    { "perspective", luaGum_perspective },
    { "translate", luaGum_translate },
    { "rotateXYZ", luaGumRotateXYZ },
    { "drawArray", luaGum_drawArray },
    { 0, 0 }
};

int luaGraphics_Init(lua_State *L)
{
    Color_register(L);
    Image_register(L);

    lua_pushstring(L, "Image");
    lua_gettable(L, LUA_GLOBALSINDEX);

    lua_pushstring(L, "BMP");
    lua_pushnumber(L, LPP_Image_BMP);
    lua_settable(L, -3);

    lua_pushstring(L, "PNG");
    lua_pushnumber(L, LPP_Image_PNG);
    lua_settable(L, -3);

    lua_pushstring(L, "JPG");
    lua_pushnumber(L, LPP_Image_JPG);
    lua_settable(L, -3);

    lua_pushstring(L, "TGA");
    lua_pushnumber(L, LPP_Image_TGA);
    lua_settable(L, -3);

    lua_pushstring(L, "GIF");
    lua_pushnumber(L, LPP_Image_GIF);
    lua_settable(L, -3);

    intraFont_register(L);
    Font_register(L);

    lua_pushstring(L, "Font");
    lua_gettable(L, LUA_GLOBALSINDEX);

    lua_pushstring(L, "FONT_SIZE_POINTS");
    lua_pushnumber(L, LPP_FONT_SIZE_POINTS);
    lua_settable(L, -3);

    lua_pushstring(L, "FONT_SIZE_PIXELS");
    lua_pushnumber(L, LPP_FONT_SIZE_PIXELS);
    lua_settable(L, -3);

    luaL_openlib(L, "screen", luaImage_methods, 0);
    luaL_openlib(L, "screen", luaScreen_methods, 0);

    luaL_openlib(L, "Gu", luaGu_methods, 0);
    luaL_openlib(L, "Gum", luaGum_methods, 0);

    #define GU_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, GU_##name);\
    lua_settable(L, -3);

    #define INTRAFONT_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, INTRAFONT_##name);\
    lua_settable(L, -3);

    lua_pushstring(L, "intraFont");
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

    lua_pushstring(L, "Gu");
    lua_gettable(L, LUA_GLOBALSINDEX);
    GU_CONSTANT(PI)
    GU_CONSTANT(FALSE)
    GU_CONSTANT(TRUE)
    GU_CONSTANT(POINTS)
    GU_CONSTANT(LINES)
    GU_CONSTANT(LINE_STRIP)
    GU_CONSTANT(TRIANGLES)
    GU_CONSTANT(TRIANGLE_STRIP)
    GU_CONSTANT(TRIANGLE_FAN)
    GU_CONSTANT(SPRITES)
    GU_CONSTANT(ALPHA_TEST)
    GU_CONSTANT(DEPTH_TEST)
    GU_CONSTANT(SCISSOR_TEST)
    GU_CONSTANT(STENCIL_TEST)
    GU_CONSTANT(BLEND)
    GU_CONSTANT(CULL_FACE)
    GU_CONSTANT(DITHER)
    GU_CONSTANT(FOG)
    GU_CONSTANT(CLIP_PLANES)
    GU_CONSTANT(TEXTURE_2D)
    GU_CONSTANT(LIGHTING)
    GU_CONSTANT(LIGHT0)
    GU_CONSTANT(LIGHT1)
    GU_CONSTANT(LIGHT2)
    GU_CONSTANT(LIGHT3)
    GU_CONSTANT(LINE_SMOOTH)
    GU_CONSTANT(PATCH_CULL_FACE)
    GU_CONSTANT(COLOR_TEST)
    GU_CONSTANT(COLOR_LOGIC_OP)
    GU_CONSTANT(FACE_NORMAL_REVERSE)
    GU_CONSTANT(PATCH_FACE)
    GU_CONSTANT(FRAGMENT_2X)
    GU_CONSTANT(PROJECTION)
    GU_CONSTANT(VIEW)
    GU_CONSTANT(MODEL)
    GU_CONSTANT(TEXTURE)
    GU_CONSTANT(TEXTURE_8BIT)
    GU_CONSTANT(TEXTURE_16BIT)
    GU_CONSTANT(TEXTURE_32BITF)
    GU_CONSTANT(TEXTURE_BITS)
    GU_CONSTANT(COLOR_5650)
    GU_CONSTANT(COLOR_5551)
    GU_CONSTANT(COLOR_4444)
    GU_CONSTANT(COLOR_8888)
    GU_CONSTANT(COLOR_BITS)
    GU_CONSTANT(NORMAL_8BIT)
    GU_CONSTANT(NORMAL_16BIT)
    GU_CONSTANT(NORMAL_32BITF)
    GU_CONSTANT(NORMAL_BITS)
    GU_CONSTANT(VERTEX_8BIT)
    GU_CONSTANT(VERTEX_16BIT)
    GU_CONSTANT(VERTEX_32BITF)
    GU_CONSTANT(VERTEX_BITS)
    GU_CONSTANT(WEIGHT_8BIT)
    GU_CONSTANT(WEIGHT_16BIT)
    GU_CONSTANT(WEIGHT_32BITF)
    GU_CONSTANT(WEIGHT_BITS)
    GU_CONSTANT(INDEX_8BIT)
    GU_CONSTANT(INDEX_16BIT)
    GU_CONSTANT(INDEX_BITS)
    GU_CONSTANT(WEIGHTS_BITS)
    GU_CONSTANT(VERTICES_BITS)
    GU_CONSTANT(TRANSFORM_3D)
    GU_CONSTANT(TRANSFORM_2D)
    GU_CONSTANT(TRANSFORM_BITS)
    GU_CONSTANT(PSM_5650)
    GU_CONSTANT(PSM_5551)
    GU_CONSTANT(PSM_4444)
    GU_CONSTANT(PSM_8888)
    GU_CONSTANT(PSM_T4)
    GU_CONSTANT(PSM_T8)
    GU_CONSTANT(PSM_T16)
    GU_CONSTANT(PSM_T32)
    GU_CONSTANT(FLAT)
    GU_CONSTANT(SMOOTH)
    GU_CONSTANT(CLEAR)
    GU_CONSTANT(AND)
    GU_CONSTANT(AND_REVERSE)
    GU_CONSTANT(COPY)
    GU_CONSTANT(AND_INVERTED)
    GU_CONSTANT(NOOP)
    GU_CONSTANT(XOR)
    GU_CONSTANT(OR)
    GU_CONSTANT(NOR)
    GU_CONSTANT(EQUIV)
    GU_CONSTANT(INVERTED)
    GU_CONSTANT(OR_REVERSE)
    GU_CONSTANT(COPY_INVERTED)
    GU_CONSTANT(OR_INVERTED)
    GU_CONSTANT(NAND)
    GU_CONSTANT(SET)
    GU_CONSTANT(NEAREST)
    GU_CONSTANT(LINEAR)
    GU_CONSTANT(NEAREST_MIPMAP_NEAREST)
    GU_CONSTANT(LINEAR_MIPMAP_NEAREST)
    GU_CONSTANT(NEAREST_MIPMAP_LINEAR)
    GU_CONSTANT(LINEAR_MIPMAP_LINEAR)
    GU_CONSTANT(TEXTURE_COORDS)
    GU_CONSTANT(TEXTURE_MATRIX)
    GU_CONSTANT(ENVIRONMENT_MAP)
    GU_CONSTANT(POSITION)
    GU_CONSTANT(UV)
    GU_CONSTANT(NORMALIZED_NORMAL)
    GU_CONSTANT(NORMAL)
    GU_CONSTANT(REPEAT)
    GU_CONSTANT(CLAMP)
    GU_CONSTANT(CW)
    GU_CONSTANT(CCW)
    GU_CONSTANT(NEVER)
    GU_CONSTANT(ALWAYS)
    GU_CONSTANT(EQUAL)
    GU_CONSTANT(NOTEQUAL)
    GU_CONSTANT(LESS)
    GU_CONSTANT(LEQUAL)
    GU_CONSTANT(GREATER)
    GU_CONSTANT(GEQUAL)
    GU_CONSTANT(COLOR_BUFFER_BIT)
    GU_CONSTANT(STENCIL_BUFFER_BIT)
    GU_CONSTANT(DEPTH_BUFFER_BIT)
    GU_CONSTANT(TFX_MODULATE)
    GU_CONSTANT(TFX_DECAL)
    GU_CONSTANT(TFX_BLEND)
    GU_CONSTANT(TFX_REPLACE)
    GU_CONSTANT(TFX_ADD)
    GU_CONSTANT(TCC_RGB)
    GU_CONSTANT(TCC_RGBA)
    GU_CONSTANT(ADD)
    GU_CONSTANT(SUBTRACT)
    GU_CONSTANT(REVERSE_SUBTRACT)
    GU_CONSTANT(MIN)
    GU_CONSTANT(MAX)
    GU_CONSTANT(ABS)
    GU_CONSTANT(SRC_COLOR)
    GU_CONSTANT(ONE_MINUS_SRC_COLOR)
    GU_CONSTANT(SRC_ALPHA)
    GU_CONSTANT(ONE_MINUS_SRC_ALPHA)
    GU_CONSTANT(DST_COLOR)
    GU_CONSTANT(ONE_MINUS_DST_COLOR)
    GU_CONSTANT(DST_ALPHA)
    GU_CONSTANT(ONE_MINUS_DST_ALPHA)
    GU_CONSTANT(FIX)
    GU_CONSTANT(KEEP)
    GU_CONSTANT(ZERO)
    GU_CONSTANT(REPLACE)
    GU_CONSTANT(INVERT)
    GU_CONSTANT(INCR)
    GU_CONSTANT(DECR)
    GU_CONSTANT(AMBIENT)
    GU_CONSTANT(DIFFUSE)
    GU_CONSTANT(SPECULAR)
    GU_CONSTANT(AMBIENT_AND_DIFFUSE)
    GU_CONSTANT(DIFFUSE_AND_SPECULAR)
    GU_CONSTANT(UNKNOWN_LIGHT_COMPONENT)
    GU_CONSTANT(DIRECTIONAL)
    GU_CONSTANT(POINTLIGHT)
    GU_CONSTANT(SPOTLIGHT)
    GU_CONSTANT(DIRECT)
    GU_CONSTANT(CALL)
    GU_CONSTANT(SEND)
    GU_CONSTANT(TAIL)
    GU_CONSTANT(HEAD)

    return 0;
}
