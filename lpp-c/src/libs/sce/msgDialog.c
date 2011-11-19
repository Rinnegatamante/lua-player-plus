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
 


#include "../../include/luaplayer.h"
#include "../graphics/graphics.h"
#include "msgDialog.h"


static void ConfigureDialog(pspUtilityMsgDialogParams *dialog, size_t dialog_size)
{
    memset(dialog, 0, dialog_size);

    dialog->base.size = dialog_size;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,
				&dialog->base.language); // Prompt language
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,
				&dialog->base.buttonSwap); // X/O button swap

    dialog->base.graphicsThread = 0x11;
    dialog->base.accessThread = 0x13;
    dialog->base.fontThread = 0x12;
    dialog->base.soundThread = 0x10;
}

void dialog_create(pspUtilityMsgDialogParams *dialog, const char* message, int mode, int opts)
{
	ConfigureDialog(dialog, sizeof(pspUtilityMsgDialogParams));
    dialog->mode = mode;
	dialog->options = opts;
	
    strcpy(dialog->message, message);
    sceUtilityMsgDialogInitStart(dialog);
}

int dialog_update()
{
	switch(sceUtilityMsgDialogGetStatus()) {		
		case PSP_UTILITY_DIALOG_INIT:
			break;
		
		case PSP_UTILITY_DIALOG_VISIBLE:
			sceUtilityMsgDialogUpdate(1);
			break;
		
		case PSP_UTILITY_DIALOG_QUIT:
			sceUtilityMsgDialogShutdownStart();
			break;
		
		case PSP_UTILITY_DIALOG_FINISHED:
			break;
			
		case PSP_UTILITY_DIALOG_NONE:
			return 0;
			
		default :
			break;
	}

	return 1;
}

