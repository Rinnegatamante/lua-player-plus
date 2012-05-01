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

#include "../LPP.h"
#include "Archive.h"
#include "../Libs/Fex/fex.h"
#include "../Libs/Dir/Dir.h"
#include "../Libs/strreplace.h"

UserdataStubs(Archive, fex_t*);

static int luaArchive_open(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Archive.open(filename) takes 1 argument.");
    }
    fex_t *ar = null;

    if(fex_open(&ar, luaL_checkstring(L, 1)) != null)
    {
        return luaL_error(L, "Cannot open the archive '%s'.", luaL_checkstring(L, 1));
    }

    *pushArchive(L) = ar;
    return(1);
}

static int luaArchive_extractFile(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Archive:extractFile(file, dest) takes 2 arguments and must be called with a colon.");
    }

    fex_t *fex = *toArchive(L, 1);
    L_CONST char *fin = luaL_checkstring(L, 2);
    L_CONST char *fot = luaL_checkstring(L, 3);
    L_CONST void *data = null;

	fex_rewind(fex);
    while(strcmp(fex_name(fex), fin) && !fex_done(fex)) fex_next(fex);

    if(strcmp(fex_name(fex), fin))
    {
        return luaL_error(L, "Cannot extract the file '%s'.", fin);
    }

    fex_data(fex, &data);
    size_t size = fex_size(fex);

    FILE *fd = fopen(fot, "wb");
    if(!fd)
    {
        return 0;
    }
    fwrite(data, size, 1, fd);
    fclose(fd);

    return(0);
}

static int luaArchive_extractAll(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Archive:extractAll(dir) takes 1 argument and must be called with a colon.");
    }

    fex_t *fex = *toArchive(L, 1);
    L_CONST char *dirto = luaL_checkstring(L, 2);

	char oldpath[255];
	getcwd(oldpath, 255);

	if(strcmp(dirto, "./") || strcmp(dirto, "../")) {
	    LPP_MkDir(dirto);
		chdir(dirto);
	}

	char filename_inzip[256];
    char *filename_withoutpath;

    L_CONST void *data;
    size_t size = 0;

	/* Go to the first file */
	fex_rewind(fex);

    while(!fex_done(fex))
    {
        strcpy(filename_inzip, fex_name(fex));

		filename_withoutpath = strrchr(filename_inzip, '/') + 1;

		if(strstr(filename_inzip, "/")) {
		    char *dir = replace(filename_inzip, filename_withoutpath, "");
			LPP_MkDir(dir);
			free(dir);
		}

        fex_data(fex, &data);
        size = fex_size(fex);

        FILE *fout = fopen(filename_inzip, "wb" );
        if(!fout) continue;
        fwrite(data, size, 1, fout);
        fclose(fout);

        fex_next(fex);
    }

	chdir(oldpath);

    return (0);
}

static int luaArchive__gc(lua_State *L)
{
    fex_close(*toArchive(L, 1));
    return 0;
}

static int luaArchive__tostring(lua_State *L)
{
    fex_t *fex = *toArchive(L,1);
    lua_pushstring(L, fex_name(fex));
    return 1;
}

static L_CONST luaL_reg luaArchive_methods[] = {
    { "open", luaArchive_open },
    { "extractFile", luaArchive_extractFile },
    { "extractAll", luaArchive_extractAll },
    { 0, 0 }
};

static L_CONST luaL_reg luaArchive_meta[] = {
    { "__gc", luaArchive__gc },
    { "__tostring", luaArchive__tostring },
    { 0, 0 }
};

UserdataRegister(Archive, luaArchive_methods, luaArchive_meta);

void luaArchive_Init(lua_State *L)
{
    Archive_register(L);
}
