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

#include "LuaPlayer.h"


#define pi 3.14159265358979323846

static int lua_MathAtan(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.atan(value) takes one argument.");
	float x = luaL_checknumber(L, 1);
	lua_pushnumber(L, atan(x));
	return 1;
}

static int lua_MathAtan2(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.atan2(y, x) takes two arguments.");

	float y = luaL_checknumber(L, 1);
	float x = luaL_checknumber(L, 2);
	lua_pushnumber(L, atan2(y, x));
	return 1;
}

static int lua_MathCos(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.cos(radians) takes one argument.");

	float radians = luaL_checknumber(L, 1);

	lua_pushnumber(L, cos(radians));

	return 1;
}

static int lua_MathSin(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.sin(radians) takes one argument.");

	float radians = luaL_checknumber(L, 1);

	lua_pushnumber(L, sin(radians));

	return 1;
}

static int lua_MathAcos(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.acos(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, acos(x));

	return 1;
}

static int lua_MathAsin(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.asin(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, asin(x));

	return 1;
}

static int lua_MathCosh(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.cosh(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, cosh(x));

	return 1;
}

static int lua_MathExp(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.exp(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, exp(x));

	return 1;
}

static int lua_MathFmax(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmax(x, y) takes two arguments.");

	float x = luaL_checknumber(L, 1);

	float y = luaL_checknumber(L, 2);

	lua_pushnumber(L, fmax(x, y));

	return 1;
}

static int lua_MathFmin(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmin(x, y) takes two arguments.");

	float x = luaL_checknumber(L, 1);

	float y = luaL_checknumber(L, 2);

	lua_pushnumber(L, fmin(x, y));

	return 1;
}

static int lua_MathFmod(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmod(x, y) takes two arguments.");

	float x = luaL_checknumber(L, 1);

	float y = luaL_checknumber(L, 2);

	lua_pushnumber(L, fmod(x, y));

	return 1;
}

static int lua_MathLog(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.log(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, log(x));

	return 1;
}

static int lua_MathPow(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.pow(x, y) takes two arguments.");

	float x = luaL_checknumber(L, 1);

	float y = luaL_checknumber(L, 2);

	lua_pushnumber(L, pow(x, y));

	return 1;
}

void vfpu_srand(unsigned int x) {
	__asm__ volatile ( "mtv %0, S000\n vrnds.s S000" : "=r"(x));
}

float vfpu_randf(float min, float max) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vsub.s   S001, S001, S000\n"
		"vrndf1.s S002\n"
		"vone.s   S003\n"
		"vsub.s   S002, S002, S003\n"
		"vmul.s   S001, S002, S001\n"
		"vadd.s   S000, S000, S001\n"
		"mfv      %0, S000\n"
		: "=r"(result) : "r"(min), "r"(max));
	return result;
}

unsigned int vfpu_rand_8888(int min, int max) {
	unsigned int result;
	__asm__ volatile (
		"mtv      %1, S020\n"
		"mtv      %2, S021\n"
		"vmov.t   C000, C020[x, x, x]\n"
		"vmov.t   C010, C020[y, y, y]\n"
		"vi2f.t   C000, C000, 0\n"
		"vi2f.t   C010, C010, 0\n"
		"vsub.t   C010, C010, C000\n"
		"vrndf1.t C020\n"
		"vsub.t   C020, C020, C020[1, 1, 1]\n"
		"vmul.t   C020, C020, C010\n"
		"vadd.t   C020, C020, C000\n"
		"vf2iz.t  C020, C020, 23\n"
		"viim.s   S023, 255\n"
		"vf2iz.s  S023, S023, 23\n"
		"vi2uc.q  S000, C020\n"
		"mfv      %0, S000\n"
		:"=r"(result): "r"(min), "r"(max));
	return result;
}

static int lua_MathSrand(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.srand(seed) takes one argument.");

	unsigned int seed = luaL_checkint(L, 1);
	vfpu_srand(seed);
	lua_pushnumber(L, seed);
	return 1;
}

static int lua_MathRandFloat(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.randfloat(min, max) takes two arguments.");

	float min = luaL_checknumber(L, 1);

	float max = luaL_checknumber(L, 2);
	lua_pushnumber(L, vfpu_randf(min, max));
	return 1;
}

static int lua_MathSinCos(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.sincos(radians) takes one argument.");

	float radians = luaL_checknumber(L, 1);

	float Sin = 0.0f;

	float Cos = 0.0f;

	sincos((double)radians, (double*)&Sin, (double*)&Cos);

	lua_pushnumber(L, Sin);

	lua_pushnumber(L, Cos);

	return 2;
}

static int lua_MathSinh(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.sinh(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, sinh(x));

	return 1;
}

static int lua_MathTan(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.tan(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, tan(x));

	return 1;
}

static int lua_MathTanh(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.tanh(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, tanh(x));

	return 1;
}

static int lua_MathSqrt(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.sqrt(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, sqrt(x));

	return 1;
}

static int lua_MathRandInt(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 2)
		return luaL_error(L, "Argument error: math.randint(min, max) takes two arguments.");

	float min = luaL_checknumber(L, 1);

	float max = luaL_checknumber(L, 2);

	lua_pushnumber(L, vfpu_rand_8888(min, max));
	return 1;
}

static int lua_MathAbs(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.abs(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, fabs(x));

	return 1;
}

static int lua_MathCeil(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.ceil(value) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, ceil(x));

	return 1;
}

static int lua_MathFloor(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.floor(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, floor(x));

	return 1;
}

static int lua_MathLog2(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.log2(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, log2(x));

	return 1;
}

static int lua_MathLog10(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.log10(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, log10(x));

	return 1;
}

static int lua_MathPow2(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.pow2(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, pow(x, 2));
	return 1;
}

static int lua_MathRound(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.round(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, round(x));

	return 1;
}

static int lua_MathTrunc(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.trunc(x) takes one argument.");

	float x = luaL_checknumber(L, 1);

	lua_pushnumber(L, trunc(x));

	return 1;
}


static int lua_MathInvSqrt(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.invsqrt(x) takes one argument.");

	float x = luaL_checknumber(L, 1);
	lua_pushnumber(L, sqrt(x));
	return 1;
}

static int lua_MathDegToRad(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.rad(x) takes one argument.");

	float x = luaL_checknumber(L, 1);
	//DA INSERIRE
	lua_pushnumber(L, x);
	return 1;
}

static int lua_MathRadToDeg(lua_State *L)
{
	int argc = lua_gettop(L);

	if(argc != 1)
		return luaL_error(L, "Argument error: math.deg(x) takes one argument.");

	float x = luaL_checknumber(L, 1);
	float result = x * 57.2957805f;
	lua_pushnumber(L, result);
	return 1;
}

static int lua_MathPi(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Argument error: math.pi() takes no argument.");
	lua_pushnumber(L, pi);
	return 1;
}

static const luaL_reg lua_Math_functions[] =
{
	{"pi",          lua_MathPi},
	{"atan",		lua_MathAtan},
	{"atan2",		lua_MathAtan2},
	{"cos",			lua_MathCos},
	{"sin",			lua_MathSin},
	{"acos",		lua_MathAcos},
	{"asin",		lua_MathAsin},
	{"cosh",		lua_MathCosh},
	{"exp",			lua_MathExp},
	{"fmax",		lua_MathFmax},
	{"fmin",		lua_MathFmin},
	{"fmod",		lua_MathFmod},
	{"log",			lua_MathLog},
	{"pow",			lua_MathPow},
	{"srand",		lua_MathSrand},
	{"randfloat",	lua_MathRandFloat},
	{"sinh",		lua_MathSinh},
	{"tan",			lua_MathTan},
	{"tanh",		lua_MathTanh},
	{"sqrt",		lua_MathSqrt},
	{"sincos",		lua_MathSinCos},
	{"randint",		lua_MathRandInt},
	{"rand",		lua_MathRandInt},
	{"abs",			lua_MathAbs},
	{"ceil",		lua_MathCeil},
	{"floor",		lua_MathFloor},
	{"log2",		lua_MathLog2},
	{"log10",		lua_MathLog10},
	{"pow2",		lua_MathPow2},
	{"invsqrt",		lua_MathInvSqrt},
	{"round",		lua_MathRound},
	{"trunc",		lua_MathTrunc},
	{"deg",			lua_MathRadToDeg},
	{"rad",			lua_MathDegToRad},
	{0, 0}
};

void luaMath_init(lua_State *L) {
	luaL_openlib(L, "math", lua_Math_functions, 0);
}