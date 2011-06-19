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
 
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <string.h>
#include <math.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspwlan.h>

#include "include/luaplayer.h"
#include "libs/graphics/graphics.h"

static int running = 1;

int netDialog()
{
	int done = 0;

   	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	
	struct pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);
	
	while(running)
	{
		guStart();
		clearScreen(0xff554433);
		guEnd();

		switch(sceUtilityNetconfGetStatus())
		{
			case PSP_UTILITY_DIALOG_NONE:
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityNetconfUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityNetconfShutdownStart();
				break;
				
			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;

			default:
				break;
		}

		sceDisplayWaitVblankStart();
		flipScreen();

		if(done)
			break;
	}
	
	return 1;
}

void netInit(void)
{
	sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	
	sceNetInetInit();
	
	sceNetApctlInit(0x8000, 48);
}

void netTerm(void)
{
	sceNetApctlTerm();
	
	sceNetInetTerm();
	
	sceNetTerm();
}

//Init Network Prompt
static int lua_NetPromptInit(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0) 
	{
		return luaL_error(L, "System.Quit() takes no arguments");
	}
	
	static int doOnce = 1;
    if (doOnce)
    {
    	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);

		sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	
		netInit();
		
		netDialog();
		netTerm();
		doOnce = 0;
	}	
	
	return 0;
}

//Shutdown Network Prompt
static int lua_NetPromptShutDown(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0) 
	{
		return luaL_error(L, "System.Quit() takes no arguments");
	}
	
	netTerm();
	
	return 0;
}

static const luaL_reg Wlan_functions[] = {
	{"init", 	lua_NetPromptInit},
	{"term",	lua_NetPromptShutDown},
	//{"getConnectionConfigs", Wlan_getConnectionConfigs},
	//{"useConnectionConfig", Wlan_useConnectionConfig},
	//{"getIPAddress", Wlan_getIPAddress},
	{0, 0}
};


void luaWlan_init(lua_State *L)
{
	luaL_openlib(L, "Wlan", Wlan_functions, 0);
}

