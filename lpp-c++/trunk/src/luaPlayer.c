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
#include <unistd.h>
#include <string.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include "include/luaplayer.h"

static lua_State *L;

const char *runScript(const char* script, bool isStringBuffer)
{
	L = lua_open();
	
	// Standard libraries
	luaL_openlibs(L);
	
	// Modules
	luaSound_init(L);
	luaControls_init(L);
	luaGraphics_init(L);
	lua3D_init(L);
	luaTimer_init(L);
	luaSystem_init(L);
	luaWlan_init(L);
	luaAdhoc_init(L);
	
	int s = 0;
	const char *errMsg = NULL;

	if(!isStringBuffer) 
		s = luaL_loadfile(L, script);
	else 
		s = luaL_loadbuffer(L, script, strlen(script), NULL);
		
	if (s == 0) 
	{
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s) 
	{
		errMsg = lua_tostring(L, -1);
		printf("error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // remove error message
	}
	lua_close(L);
	
	return errMsg;
}
