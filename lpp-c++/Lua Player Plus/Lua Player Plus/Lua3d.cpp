/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
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

#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspgum.h>
#include "LuaPlayer.h"

PspGeContext __attribute__((aligned(16))) geContext;
unsigned int __attribute__((aligned(16))) list3d[262144];

extern unsigned int ToColor(lua_State* L, int a);
extern OSL_IMAGE *toImage(lua_State *L, int arg);

static int lua_SceGuClearColor(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.ClearColor(color) takes one argument.");
	sceGuClearColor(ToColor(L, 1));
	return(1);
}

static int lua_SceGuClearDepth(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.ClearDepth(depth) takes one argument.");
	sceGuClearDepth(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGuClear(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.Clear(flags) takes one argument.");
	sceGuClear(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGumMatrixMode(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gum.MatrixMode(mode) takes one argument.");
	sceGumMatrixMode(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGumLoadIdentity(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Gum.LoadIdentity() takes no arguments.");
	sceGumLoadIdentity();
	return(1);
}

static int lua_SceGumPerspective(lua_State *L)
{
	if(lua_gettop(L) != 4)
		return luaL_error(L, "Gum.Perspective(fovy, aspect, near , far) takes 4 arguments.");
	sceGumPerspective(luaL_checknumber(L, 1), luaL_checknumber(L, 2),
		luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return(1);
}

static int lua_SceGumTranslate(lua_State *L)
{
	if(lua_gettop(L) != 3)
		return luaL_error(L, "Gum.Translate(x, y, z) takes 3 arguments.");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumTranslate(&v);
	return(1);
}

static int lua_SceGumRotateXYZ(lua_State *L)
{
	if(lua_gettop(L) != 3)
		return luaL_error(L, "Gum.RotateXYZ(x, y, z) takes 3 arguments.");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumRotateXYZ(&v);
	return(1);
}

static int lua_SceGuTexImage(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.TexImage(image) take one argument.");
	oslSetTexture(toImage(L, 1));
	return(1);
}

static int lua_SceGuTexFunc(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Gu.TexFunc(tfx, tcc) takes 2 arguments.");
	sceGuTexFunc(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return(1);
}

static int lua_SceGuEnvColor(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.EnvColor(Color) takes 1 argument.");
	sceGuTexEnvColor(ToColor(L, 1));
	return(1);
}

static int lua_SceGuTexFilter(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Gu.TexFilter(min, mag) takes 2 arguments.");
	sceGuTexFilter(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return(1);
}

static int lua_SceGuTexScale(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Gu.TexScale(u, v) takes 2 arguments.");
	sceGuTexScale(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return(1);
}

static int lua_SceGuTexOffset(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Gu.TexOffset(u, v) takes 2 arguments.");
	sceGuTexOffset(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return(1);
}

static int lua_SceGuAmbientColor(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.AmbientColor(Color) takes one argument.");
	sceGuAmbientColor(ToColor(L, 1));
	return(1);
}

static int lua_SceGuAmbient(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.Ambient(Color) takes one argument.");
	sceGuAmbient(ToColor(L, 1));
	return(1);
}

static int lua_SceGuEnable(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.Enable(flags) takes one argument.");
	sceGuEnable(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGuDisable(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.Disable(flags) takes one argument.");
	sceGuDisable(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGuBlendFunc(lua_State *L)
{
	if(lua_gettop(L) != 5)
		return luaL_error(L, "Gu.BlendFunc(op, src, dst, srcfix, dstfix) takes 5 arguments.");
	sceGuBlendFunc(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3),
		luaL_checkint(L, 4), luaL_checkint(L, 5));
	return(1);
}

static int lua_SceGuLight(lua_State *L)
{
	if(lua_gettop(L) != 6)
		return luaL_error(L, "Gu.Light() takes 6 arguments.");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 4);
	v.y = luaL_checknumber(L, 5);
	v.z = luaL_checknumber(L, 6);
	sceGuLight(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), &v);
	return(1);
}

static int lua_SceGuLightAtt(lua_State *L)
{
	if(lua_gettop(L) != 4)
		return luaL_error(L, "");
	sceGuLightAtt(luaL_checkint(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return(1);
}

static int lua_SceGuLightColor(lua_State *L)
{
	if(lua_gettop(L) != 3)
		return luaL_error(L, "Gu.LightColor(light, component, color) takes 3 arguments.");
	sceGuLightColor(luaL_checkint(L, 1), luaL_checkint(L, 2), ToColor(L, 3));
	return(1);
}

static int lua_SceGuLightMode(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Gu.LightMode(mode) takes one argument.");
	sceGuLightMode(luaL_checkint(L, 1));
	return(1);
}

static int lua_SceGuLightSpot(lua_State *L)
{
	if(lua_gettop(L) != 6)
		return luaL_error(L, "Gu.LightSpot(light, directionX, directionY, directionZ, exponent, cutoff) takes 6 arguments.");
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 2);
	v.y = luaL_checknumber(L, 3);
	v.z = luaL_checknumber(L, 4);
	sceGuLightSpot(luaL_checkint(L, 1), &v, luaL_checknumber(L, 5), luaL_checknumber(L, 6));
	return(1);
}

static int lua_SceGuDrawArray(lua_State *L)
{
	if(lua_gettop(L) != 3)
		return luaL_error(L, "Gu.DrawArray() takes 3 arguments.");
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
				*((unsigned int*) vertex) = ToColor(L, -1);
			}
			lua_pop(L, 1);
			vertex++;
		}
		lua_pop(L, 1);
	}

	sceKernelDcacheWritebackInvalidateAll();
	sceGumDrawArray(prim, vtype, n, NULL, vertices);
	free(vertices);
	return(1);
}

static int lua_Start3D(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Gu.Start3D() takes no arguments.");
	sceGeSaveContext(&geContext);
	sceGuStart(GU_DIRECT, list3d);
	return(1);
}

static int lua_End3D(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Gu.End3D() takes no arguments.");
	sceGeRestoreContext(&geContext);
	return(1);
}

static const luaL_reg Gu_functions[] = {
	{"ClearColor", lua_SceGuClearColor},
	{"ClearDepth", lua_SceGuClearDepth},
	{"Clear", lua_SceGuClear},
	{"TexImage", lua_SceGuTexImage},
	{"TexFunc", lua_SceGuTexFunc},
	{"TexEnvColor", lua_SceGuEnvColor},
	{"TexFilter", lua_SceGuTexFilter},
	{"TexScale", lua_SceGuTexScale},
	{"TexOffset", lua_SceGuTexOffset},
	{"AmbientColor", lua_SceGuAmbientColor},
	{"Ambient", lua_SceGuAmbient},
	{"Enable", lua_SceGuEnable},
	{"Disable", lua_SceGuDisable},
	{"BlendFunc", lua_SceGuBlendFunc},
	{"Light", lua_SceGuLight},
	{"LightAtt", lua_SceGuLightAtt},
	{"LightColor", lua_SceGuLightColor},
	{"LightMode", lua_SceGuLightMode},
	{"LightSpot", lua_SceGuLightSpot},
	{"Start3D", lua_Start3D},
	{"End3D", lua_End3D},
	{0, 0}
};

static const luaL_reg Gum_functions[] = {
	{"MatrixMode", lua_SceGumMatrixMode},
	{"LoadIdentity", lua_SceGumLoadIdentity},
	{"Perspective", lua_SceGumPerspective},
	{"Translate", lua_SceGumTranslate},
	{"RotateXYZ", lua_SceGumRotateXYZ},
	{"DrawArray", lua_SceGuDrawArray},
	{0, 0}
};

void lua3D_init(lua_State *L)
{
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
