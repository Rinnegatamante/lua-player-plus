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

#include "include/luaplayer.h"
#include "libs/adhoc/adhoc.h"

//Initialize Adhoc
static int lua_AdhocInit(lua_State *L)
{
	int argc = lua_gettop(L); 
	if (argc != 0)
	{
		return luaL_error(L, "Adhoc.init() takes no arguments"); 
	}
	
	return 0;
}

//Shutdown Adhoc
static int lua_AdhocShutDown(lua_State *L)
{
	int argc = lua_gettop(L); 
	if (argc != 0)
	{
		return luaL_error(L, "Adhoc.shutdown() takes no arguments"); 
	}
	
	return 0;
}

//Connect Adhoc
static int lua_AdhocConnect(lua_State *L)
{
	int argc = lua_gettop(L); 
	if (argc != 0)
	{
		return luaL_error(L, "Adhoc.connect() takes no arguments"); 
	}
	
	return 0;
}

//Send a message
static int lua_AdhocSendMessage(lua_State *L)
{
	int argc = lua_gettop(L); 
	if (argc != 1)
	{
		return luaL_error(L, "Adhoc.sendMessage(message) takes 1 argument (the message)"); 
	}
	
	return 1;
}

static const luaL_reg Adhoc_functions[] = {
	{"init", 		lua_AdhocInit},
	{"shutdown", 	lua_AdhocShutDown},
	{"connect", 	lua_AdhocConnect},
	{"sendMessage",	lua_AdhocSendMessage},
	{0, 0}
};

void luaAdhoc_init(lua_State *L)
{
	luaL_openlib(L, "Adhoc", Adhoc_functions, 0);
}
