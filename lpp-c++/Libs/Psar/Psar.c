/** LPP Psar lib by Nanni */

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

#include <zzip/lib.h>
#include <zzip/plugin.h>

#include <pspkernel.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "Psar.h"

static unsigned int pOffset = 0;
static zzip_plugin_io_handlers psar_handlers = {};

static unsigned char initialized = 0;

#ifdef DEBUG
extern int dwrite_output(const char *format, ...);
#endif

static int psar_open(zzip_char_t *name, int flags, ...)
{
    int fd = sceIoOpen(name, PSP_O_RDONLY, 0777);

    if(fd < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %i : Cannot open the file '%s'.\n", __FUNCTION__, __LINE__, name);
        #endif
        return -1;
    }

    sceIoLseek(fd, 0x24, PSP_SEEK_SET);
    sceIoRead(fd, &pOffset, sizeof(u32));
    sceIoLseek(fd, pOffset, PSP_SEEK_SET);

    return (fd);
}

static int psar_close(int fd)
{
    return sceIoClose(fd);
}

static zzip_ssize_t psar_read(int fd, void *buf, zzip_size_t len)
{
    return sceIoRead(fd, buf, len);
}

static zzip_off_t psar_seek(int fd, zzip_off_t offset, int whence)
{
    switch(whence)
    {
    case PSP_SEEK_SET :
        return sceIoLseek(fd, (pOffset + offset), PSP_SEEK_SET) - pOffset;
        break;
    case PSP_SEEK_CUR:
        return sceIoLseek(fd, offset, PSP_SEEK_CUR) - pOffset;
        break;
    case PSP_SEEK_END:
        if(offset < 0) return sceIoLseek(fd, offset, PSP_SEEK_END) - pOffset;
        break;
        default: break;
    }

    return 0;
}

static zzip_off_t psar_fsize(int fd)
{
    return sceIoLseek32(fd, 0, PSP_SEEK_END) - pOffset;
}

void LPP_PsarDecoder_Init(void)
{
    if(initialized)
    {
        return;
    }

    zzip_init_io(&psar_handlers, 0);
    psar_handlers.fd.open = &psar_open;
    psar_handlers.fd.close = &psar_close;
    psar_handlers.fd.read = &psar_read;
    psar_handlers.fd.seeks = &psar_seek;
    psar_handlers.fd.filesize = &psar_fsize;

    initialized = 1;
}

PSAR_ENTRY *LPP_PsarDecoder_getEntry(const char *filename)
{
    if(!initialized) return NULL;
    zzip_strings_t ext[] = {"", 0};

    ZZIP_FILE *fd = zzip_open_ext_io(filename, O_RDONLY | (0x0), ZZIP_ONLYZIP, ext, &psar_handlers);
    if(fd == NULL)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filename);
        #endif
        return NULL;
    }

    PSAR_ENTRY *entry = (PSAR_ENTRY*)malloc(sizeof(PSAR_ENTRY));
    if(!entry)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Cannot allocate 'entry' to memory.\n", __FUNCTION__, __LINE__);
        #endif
        zzip_close(fd);
        return NULL;
    }

    memset(entry, 0, sizeof(PSAR_ENTRY));

    zzip_seek(fd, 0, SEEK_END);
    entry->len = zzip_tell(fd);
    zzip_rewind(fd);

    if(entry->len <= 0)
    {
        free(entry);
        zzip_fclose(fd);
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : file len is lower than zero.\n", __FUNCTION__, __LINE__);
        #endif
        return NULL;
    }

    entry->data = (u8*)malloc(entry->len);
    zzip_fread(entry->data, 1, entry->len, fd);
    zzip_fclose(fd);

    return(entry);
}

void LPP_PsarDecoder_freeEntry(PSAR_ENTRY *entry)
{
    if(entry->data) free(entry->data);
    if(entry) free(entry);
}
