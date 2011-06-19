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

#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspgum.h>
#include "include/luaplayer.h"

#include "libs/graphics/graphics.h"

PspGeContext __attribute__((aligned(16))) geContext;

extern Color* toColor(lua_State *L, int arg);
extern Image** toImage(lua_State *L, int arg);

static int lua_sceGuClearColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClearColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGuClearDepth(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClearDepth(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGuClear(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClear(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGumMatrixMode(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGumMatrixMode(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGumLoadIdentity(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	sceGumLoadIdentity();
	return 0;
}
static int lua_sceGumPerspective(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 4) return luaL_error(L, "wrong number of arguments"); 
	sceGumPerspective(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return 0;
}
	
static int lua_sceGumTranslate(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumTranslate(&v);
	return 0;
}

static int lua_sceGumRotateXYZ(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumRotateXYZ(&v);
	return 0;
}

static int lua_sceGuTexImage(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	Image* image = *toImage(L, 1);
	sceGuTexImage(0, image->textureWidth, image->textureHeight, image->textureWidth, image->data);

	return 0;
}

static int lua_sceGuTexFunc(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexFunc(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return 0;
}

static int lua_sceGuTexEnvColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexEnvColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGuTexFilter(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexFilter(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return 0;
}

static int lua_sceGuTexScale(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexScale(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 0;
}

static int lua_sceGuTexOffset(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexOffset(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 0;
}

static int lua_sceGuAmbientColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuAmbientColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGuAmbient(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuAmbient(*toColor(L, 1));
	return 0;
}

static int lua_sceGuEnable(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuEnable(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGuDisable(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuDisable(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGuBlendFunc(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 5) return luaL_error(L, "wrong number of arguments"); 
	sceGuBlendFunc(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5));
	return 0;
}

static int lua_sceGuLight(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 6) return luaL_error(L, "wrong number of arguments");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 4);
	v.y = luaL_checknumber(L, 5);
	v.z = luaL_checknumber(L, 6);
	sceGuLight(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), &v);
	return 0;
}

static int lua_sceGuLightAtt(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 4) return luaL_error(L, "wrong number of arguments");
	sceGuLightAtt(luaL_checkint(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return 0;
}

static int lua_sceGuLightColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
	sceGuLightColor(luaL_checkint(L, 1), luaL_checkint(L, 2), *toColor(L, 3));
	return 0;
}

static int lua_sceGuLightMode(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	sceGuLightMode(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGuLightSpot(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 6) return luaL_error(L, "wrong number of arguments");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 2);
	v.y = luaL_checknumber(L, 3);
	v.z = luaL_checknumber(L, 4);
	sceGuLightSpot(luaL_checkint(L, 1), &v, luaL_checknumber(L, 5), luaL_checknumber(L, 6));
	return 0;
}

static int lua_sceGumDrawArray(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 3) return luaL_error(L, "wrong number of arguments");

	int prim = luaL_checkint(L, 1);
	int vtype = luaL_checkint(L, 2);
	if (lua_type(L, 3) != LUA_TTABLE) return luaL_error(L, "vertices table missing");
	int n = luaL_getn(L, 3);

	int quads = 0;
	int colorLuaIndex = -1;
	if (vtype & GU_TEXTURE_32BITF) quads += 2;
	if (vtype & GU_COLOR_8888) {
		quads++;
		colorLuaIndex = quads;
	}
	if (vtype & GU_NORMAL_32BITF) quads += 3;
	if (vtype & GU_VERTEX_32BITF) quads += 3;

	void* vertices = memalign(16, n * quads*4);
	float* vertex = (float*) vertices;
	int i;
	for (i = 1; i <= n; ++i) {
		// get vertice table
		lua_rawgeti(L, 3, i);
		int n2 = luaL_getn(L, -1);
		if (n2 != quads) {
			free(vertices);
			return luaL_error(L, "wrong number of vertex components");
		}
		int j;
		for (j = 1; j <= n2; ++j) {
			lua_rawgeti(L, -1, j);
			if (j != colorLuaIndex) {
				*vertex = luaL_checknumber(L, -1);
			} else {
				*((Color*) vertex) = *toColor(L, -1);
			}
			lua_pop(L, 1);  // removes 'value'
			vertex++;
		}

		// remove vertice table
		lua_pop(L, 1);
	}
	
	sceKernelDcacheWritebackInvalidateAll();
	sceGumDrawArray(prim, vtype, n, NULL, vertices);
	free(vertices);
	return 0;
}

static int lua_start3d(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	sceGeSaveContext(&geContext);
	guStart();
	return 0;
}

static int lua_end3d(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	guEnd();
	sceGeRestoreContext(&geContext);
	return 0;
}

static const luaL_reg Gu_functions[] = {
	{"clearColor", lua_sceGuClearColor},
	{"clearDepth", lua_sceGuClearDepth},
	{"clear", lua_sceGuClear},
	{"texImage", lua_sceGuTexImage},
	{"texFunc", lua_sceGuTexFunc},
	{"texEnvColor", lua_sceGuTexEnvColor},
	{"texFilter", lua_sceGuTexFilter},
	{"texScale", lua_sceGuTexScale},
	{"texOffset", lua_sceGuTexOffset},
	{"ambientColor", lua_sceGuAmbientColor},
	{"ambient", lua_sceGuAmbient},
	{"enable", lua_sceGuEnable},
	{"disable", lua_sceGuDisable},
	{"blendFunc", lua_sceGuBlendFunc},
	{"light", lua_sceGuLight},
	{"lightAtt", lua_sceGuLightAtt},
	{"lightColor", lua_sceGuLightColor},
	{"lightMode", lua_sceGuLightMode},
	{"lightSpot", lua_sceGuLightSpot},
	{"start3d", lua_start3d},
	{"end3d", lua_end3d},
  {0, 0}
};

static const luaL_reg Gum_functions[] = {
	{"matrixMode", lua_sceGumMatrixMode},
	{"loadIdentity", lua_sceGumLoadIdentity},
	{"perspective", lua_sceGumPerspective},
	{"translate", lua_sceGumTranslate},
	{"rotateXYZ", lua_sceGumRotateXYZ},
	{"drawArray", lua_sceGumDrawArray},
  {0, 0}
};

void lua3D_init(lua_State *L) {
	luaL_openlib(L, "Gu", Gu_functions, 0);
	luaL_openlib(L, "Gum", Gum_functions, 0);

#define GU_CONSTANT(name)\
	lua_pushstring(L, #name);\
	lua_pushnumber(L, GU_##name);\
	lua_settable(L, -3);

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
	//GU_CONSTANT(COLOR_RES1)
	//GU_CONSTANT(COLOR_RES2)
	//GU_CONSTANT(COLOR_RES3)
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
}
