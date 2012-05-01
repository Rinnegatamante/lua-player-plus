/** LPP Dir lib by Nanni */

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

#include <pspkernel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "Dir.h"

#ifdef DEBUG
extern int dwrite_output(const char *format, ...);
#endif

int LPP_MkDir(const char *dir) {
    char *buffer;
	char *p;
	u32 len = strlen(dir);

	if(len <= 0) return -1;

	buffer = (char*)malloc(len + 1);
	strcpy(buffer, dir);

	if(buffer[len - 1] == '/') {
	    buffer[len - 1] = '\0';
	}

	if(sceIoMkdir(buffer, 0777) == 0)
	{
	    free(buffer);
		return 0;
	}

	p = buffer + 1;

	while(1) {
	    char hold;
		while(*p && *p != '\\' && *p != '/') p++;
		hold = *p;
		*p = 0;

		if((sceIoMkdir(buffer, 0777) == -1) && (errno == ENOENT))
		{
		    free(buffer);
			return -1;
		}

		if(hold == 0) break;
		*p++ = hold;
	}

	free(buffer);
	return 0;
}

u8 LPP_FileExists(const char *Filename)
{
    SceUID fd = sceIoOpen(Filename, PSP_O_RDONLY, 0777);
    if(fd <= 0) return 0;
    sceIoClose(fd);
    return 1;
}

void LPP_RemoveDir(const char *dir)
{
    char *fullOldPath;
    SceIoDirent oneDir;
    SceUID oDir = sceIoDopen(dir);
    if(oDir < 0) return;

    while(1) {
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if(sceIoDread(oDir, &oneDir) <= 0) break;

        if(!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, "..")) continue;

        if(dir[strlen(dir) - 1] != '/') {
            fullOldPath = (char *)calloc(strlen(dir) + strlen(oneDir.d_name) + 2, 1);
            sprintf(fullOldPath, "%s/%s", dir, oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(dir) + strlen(oneDir.d_name) + 1, 1);
            sprintf(fullOldPath, "%s/%s", dir, oneDir.d_name);
        }

        if(FIO_S_ISDIR(oneDir.d_stat.st_mode)) {
            LPP_RemoveDir(fullOldPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)) {
            sceIoRemove(fullOldPath);
        }

        if(fullOldPath) free(fullOldPath);
    }

    sceIoDclose(oDir);
    sceIoRmdir(dir);
}

int LPP_Chdir(const char *path)
{
    int res = chdir(path);
    if(res == 0) return 1;

    return res;
}

int LPP_GetCwd(char *path, size_t size)
{
    if(getcwd(path, size) == 0)
        return 0;
    return 1;
}

LPP_Dir *LPP_DirOpen(const char *path)
{
    LPP_Dir *dir = (LPP_Dir *)malloc(sizeof(LPP_Dir));
    if(dir == 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Cannot alloc 'dir' to memory.\n", __FUNCTION__, __LINE__);
        #endif
        return 0;
    }

    SceIoDirent oDir;
    memset(&oDir, 0, sizeof(SceIoDirent));

    dir->fd = sceIoDopen(path);

    if(dir->fd < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Cannot open the dir '%s'.\n", __FUNCTION__, __LINE__, path);
        #endif
        free(dir);
        return 0;
    }

    return dir;
}

int LPP_DirRead(LPP_Dir *dir)
{
    int count = 0;

    SceIoDirent dirent;
    memset(&dirent, 0, sizeof(SceIoDirent));

    dir->entries = (LPP_DirEntry *)malloc(sizeof(LPP_DirEntry));
    if(dir->entries == NULL) return -1;

    while((sceIoDread(dir->fd, &dirent) > 0))
    {
        if((strcmp(dirent.d_name, ".") != 0) && (strcmp(dirent.d_name, "..") != 0))
        {
            dir->entries = (LPP_DirEntry *)realloc(dir->entries, sizeof(LPP_DirEntry) * (count + 1));

            if(dir->entries == 0)
            {
                #ifdef DEBUG
                dwrite_output("Function %s Line %d : Cannot alloc 'entries' to memory.\n", __FUNCTION__, __LINE__);
                #endif
                return -1;
            }

            strcpy(dir->entries[count].name, dirent.d_name);
            dir->entries[count].size = dirent.d_stat.st_size;
            dir->entries[count].type = (FIO_SO_IFDIR & dirent.d_stat.st_attr) == FIO_SO_IFDIR;
            count++;
        }
    }

    if(count == 0) return 0;

    dir->count = count;

    return(count);
}

void LPP_DirClose(LPP_Dir *dir)
{
    if(dir != 0)
    {
        sceIoDclose(dir->fd);
        if(dir->entries != 0) free(dir->entries);

        free(dir);
    }
}

u8 LPP_DirExists(const char *path)
{
    SceIoStat stats;

    int res = sceIoGetstat(path, &stats);
    if(res < 0) return 0;

    if(stats.st_mode & FIO_S_IFDIR) return 1;

    return 0;
}

int LPP_DirRename(const char *oldname, const char *newname)
{
    int res = sceIoRename(oldname, newname);
    if(res == 0) return 1;

    return res;
}

int LPP_FileRename(const char *oldname, const char *newname)
{
    int res = sceIoRename(oldname, newname);
    if(res == 0) return 1;
    return res;
}

void LPP_MoveFile(const char *oldpath, const char *newpath)
{
    SceUID oldFile, newFile;
    size_t readSize = 0;
    u8 filebuf[0x8000];

    oldFile = sceIoOpen(oldpath, PSP_O_RDONLY, 0777);
    if(oldFile < 0) return;
    newFile = sceIoOpen(newpath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if(newFile < 0) {
        sceIoClose(oldFile);
        return;
    }

    while((readSize == sceIoRead(oldFile, filebuf, 0x08000)) > 0)
    {
        sceIoWrite(newFile, filebuf, readSize);
    }

    sceIoClose(newFile);
    sceIoClose(oldFile);
    sceIoRemove(oldpath);
}

void LPP_CopyFile(const char *oldpath, const char *newpath)
{
    SceUID oldFile, newFile;
    size_t readSize = 0;
    u8 filebuf[0x8000];

    oldFile = sceIoOpen(oldpath, PSP_O_RDONLY, 0777);
    if(oldFile < 0) return;
    newFile = sceIoOpen(newpath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if(newFile < 0) {
        sceIoClose(oldFile);
        return;
    }

    while((readSize == sceIoRead(oldFile, filebuf, 0x08000)) > 0)
    {
        sceIoWrite(newFile, filebuf, readSize);
    }

    sceIoClose(newFile);
    sceIoClose(oldFile);
}

void LPP_MoveDir(const char *oldPath, const char *newPath)
{
    if(!LPP_DirExists(newPath)) {
        LPP_MkDir(newPath);
    }

    char *fullOldPath;
    char *fullNewPath;
    SceIoDirent oneDir;

    SceUID oDir = sceIoDopen(oldPath);
    if(oDir < 0) return;

    while(1) {
        memset(&oneDir, 0, sizeof(SceIoDirent));

        if(sceIoDread(oDir, &oneDir) <= 0) break;

        if(!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, "..")) continue;

        if(oldPath[strlen(oldPath) - 1] != '/') {
            fullOldPath = (char *)calloc(strlen(oldPath) + strlen(oneDir.d_name) + 2, 1);
            fullNewPath = (char *)calloc(strlen(newPath) + strlen(oneDir.d_name) + 2, 1);
            sprintf(fullOldPath, "%s/%s", oldPath, oneDir.d_name);
            sprintf(fullNewPath, "%s/%s", newPath, oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath) + strlen(oneDir.d_name) + 1, 1);
            fullNewPath = (char *)calloc(strlen(newPath) + strlen(oneDir.d_name) + 1, 1);
            sprintf(fullOldPath, "%s/%s", oldPath, oneDir.d_name);
            sprintf(fullNewPath, "%s/%s", newPath, oneDir.d_name);
        }

        if(FIO_S_ISDIR(oneDir.d_stat.st_mode)) {
            LPP_MoveDir(fullOldPath, fullNewPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)) {
            LPP_MoveFile(fullOldPath, fullNewPath);
        }

        free(fullOldPath);
        free(fullNewPath);
    }

    sceIoDclose(oDir);
    sceIoRmdir(oldPath);
}

void LPP_CopyDir(const char *oldPath, const char *newPath)
{
    if(!LPP_DirExists(newPath)) {
        LPP_MkDir(newPath);
    }

    char *fullOldPath;
    char *fullNewPath;
    SceIoDirent oneDir;

    SceUID oDir = sceIoDopen(oldPath);
    if(oDir < 0) return;

    while(1) {
        memset(&oneDir, 0, sizeof(SceIoDirent));

        if(sceIoDread(oDir, &oneDir) <= 0) break;

        if(!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, "..")) continue;

        if(oldPath[strlen(oldPath) - 1] != '/') {
            fullOldPath = (char *)calloc(strlen(oldPath) + strlen(oneDir.d_name) + 2, 1);
            fullNewPath = (char *)calloc(strlen(newPath) + strlen(oneDir.d_name) + 2, 1);
            sprintf(fullOldPath, "%s/%s", oldPath, oneDir.d_name);
            sprintf(fullNewPath, "%s/%s", newPath, oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath) + strlen(oneDir.d_name) + 1, 1);
            fullNewPath = (char *)calloc(strlen(newPath) + strlen(oneDir.d_name) + 1, 1);
            sprintf(fullOldPath, "%s/%s", oldPath, oneDir.d_name);
            sprintf(fullNewPath, "%s/%s", newPath, oneDir.d_name);
        }

        if(FIO_S_ISDIR(oneDir.d_stat.st_mode)) {
            LPP_CopyDir(fullOldPath, fullNewPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)) {
            LPP_CopyFile(fullOldPath, fullNewPath);
        }

        free(fullOldPath);
        free(fullNewPath);
    }

    sceIoDclose(oDir);
}

void LPP_RemoveFile(const char *path)
{
    if(LPP_FileExists(path))
        sceIoRemove(path);
}
