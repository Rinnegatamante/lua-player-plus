#include <pspkernel.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "include/mathfix.h"

static int lua_MathAtan(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.atan(value) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathAtan(x));
	
	return 1;
}

static int lua_MathAtan2(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.atan2(y, x) takes two arguments.");
	
	float y = luaL_checknumber(L, 1);
	
	float x = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathAtan2(y, x));
	
	return 1;
}

static int lua_MathCos(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.cos(radians) takes one argument.");
	
	float radians = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathCos(radians));
	
	return 1;
}

static int lua_MathSin(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.sin(radians) takes one argument.");
	
	float radians = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathSin(radians));
	
	return 1;
}

static int lua_MathAcos(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.acos(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathAcos(x));
	
	return 1;
}

static int lua_MathAsin(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.asin(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathAsin(x));
	
	return 1;
}

static int lua_MathCosh(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.cosh(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathCosh(x));
	
	return 1;
}

static int lua_MathExp(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.exp(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathExp(x));
	
	return 1;
}

static int lua_MathFmax(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmax(x, y) takes two arguments.");
	
	float x = luaL_checknumber(L, 1);
	
	float y = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathFmax(x, y));
	
	return 1;
}

static int lua_MathFmin(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmin(x, y) takes two arguments.");
	
	float x = luaL_checknumber(L, 1);
	
	float y = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathFmin(x, y));
	
	return 1;
}

static int lua_MathFmod(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.fmod(x, y) takes two arguments.");
	
	float x = luaL_checknumber(L, 1);
	
	float y = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathFmod(x, y));
	
	return 1;
}

static int lua_MathLog(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.log(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathLog(x));
	
	return 1;
}

static int lua_MathPow(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.pow(x, y) takes two arguments.");
	
	float x = luaL_checknumber(L, 1);
	
	float y = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathPow(x, y));
	
	return 1;
}

static int lua_MathSrand(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.srand(seed) takes one argument.");
	
	unsigned int seed = luaL_checkint(L, 1);
	
	MathSrand(seed);
	
	return 0;
}

static int lua_MathRandFloat(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.randfloat(min, max) takes two arguments.");
	
	float min = luaL_checknumber(L, 1);
	
	float max = luaL_checknumber(L, 2);
	
	lua_pushnumber(L, MathRandFloat(min, max));
	
	return 1;
}

static int lua_MathSinCos(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.sincos(radians) takes one argument.");
	
	float radians = luaL_checknumber(L, 1);
	
	float sin = 0.0f;
	
	float cos = 0.0f;
	
	MathSincos(radians, &sin, &cos);
	
	lua_pushnumber(L, sin);
	
	lua_pushnumber(L, cos);
	
	return 2;
}

static int lua_MathSinh(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.sinh(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathSinh(x));
	
	return 1;
}

static int lua_MathTan(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.tan(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathTan(x));
	
	return 1;
}

static int lua_MathTanh(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.tanh(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathTanh(x));
	
	return 1;
}

static int lua_MathSqrt(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.sqrt(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathSqrt(x));
	
	return 1;
}

static int lua_MathRandInt(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2)
		return luaL_error(L, "Argument error: math.randint(min, max) takes two arguments.");
	
	float min = luaL_checknumber(L, 1);
	
	float max = luaL_checknumber(L, 2);
	
	lua_pushinteger(L, MathRandInt(min, max));
	
	return 1;
}

static int lua_MathAbs(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.abs(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathAbs(x));
	
	return 1;
}

static int lua_MathCeil(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.ceil(value) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathCeil(x));
	
	return 1;
}

static int lua_MathFloor(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.floor(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathFloor(x));
	
	return 1;
}

static int lua_MathLog2(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.log2(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathLog2(x));
	
	return 1;
}

static int lua_MathLog10(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.log10(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathLog10(x));
	
	return 1;
}

static int lua_MathPow2(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.pow2(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathPow2(x));
	
	return 1;
}

static int lua_MathRound(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.round(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathRound(x));
	
	return 1;
}

static int lua_MathTrunc(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.trunc(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathTrunc(x));
	
	return 1;
}

static int lua_MathInvSqrt(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.invsqrt(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathInvSqrt(x));
	
	return 1;
}

static int lua_MathDegToRad(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.rad(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathDegToRad(x));
	
	return 1;
}

static int lua_MathRadToDeg(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: math.deg(x) takes one argument.");
	
	float x = luaL_checknumber(L, 1);
	
	lua_pushnumber(L, MathRadToDeg(x));
	
	return 1;
}

static const luaL_reg lua_Math_functions[] =
{
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