#include <psputility.h>
#include <pspdisplay.h>
#include <psputility_osk.h>
#include <stdio.h>
#include <string.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspsdk.h>
#include "osk.h"
#include "../graphics/graphics.h"


PspGeContext __attribute__((aligned(16))) geContext3;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get text from OSK:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int get_text_osk(char *input, unsigned short *intext, unsigned short *desc){
    int done=0;
    unsigned short outtext[128] = { 0 }; // text after input
    clearScreen(0);
    flipScreen();
    clearScreen(0);
    flipScreen();
    sceGeSaveContext(&geContext3);

    SceUtilityOskData data;
    memset(&data, 0, sizeof(data));
    data.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT;            // key glyphs: 0-1=hiragana, 2+=western/whatever the other field says
    data.lines = 1;                // just one line
    data.unk_24 = 1;            // set to 1
    data.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL;
    data.desc = desc;
    data.intext = intext;
    data.outtextlength = 512;    // sizeof(outtext) / sizeof(unsigned short)
    data.outtextlimit = 512;        // just allow 50 chars
    data.outtext = (unsigned short*)outtext;

    SceUtilityOskParams osk;
    memset(&osk, 0, sizeof(osk));
    osk.base.size = sizeof(osk);
    // dialog language: 0=Japanese, 1=English, 2=French, 3=Spanish, 4=German,
    // 5=Italian, 6=Dutch, 7=Portuguese, 8=Russian, 9=Korean, 10-11=Chinese, 12+=default
    osk.base.language = 12;
    osk.base.buttonSwap = 1;        // X button: 1
    osk.base.graphicsThread = 17;    // gfx thread pri
    //osk.base.unknown = 19;            // unknown thread pri (?)
    osk.base.accessThread = 19;
    osk.base.fontThread = 18;
    osk.base.soundThread = 16;
    osk.datacount = 1;
    osk.data = &data;

    int rc = sceUtilityOskInitStart(&osk);

    while(!done) {
        int i,j=0;

        clearScreen(0);

        switch(sceUtilityOskGetStatus()){
            case PSP_UTILITY_DIALOG_INIT :
            break;
            case PSP_UTILITY_DIALOG_VISIBLE :
            sceUtilityOskUpdate(1); // 2 is taken from ps2dev.org recommendation
            break;
            case PSP_UTILITY_DIALOG_QUIT :
            sceUtilityOskShutdownStart();
            break;
            case PSP_UTILITY_DIALOG_FINISHED :
            sceUtilityOskShutdownStart();
            done = 1;
            break;
            case PSP_UTILITY_DIALOG_NONE :
            default :
            break;
        }

        for(i = 0; data.outtext[i]; i++)
            if (data.outtext[i]!='\0' && data.outtext[i]!='\n' && data.outtext[i]!='\r'){
                input[j] = data.outtext[i];
                j++;
            }
        input[j] = 0;

        // wait TWO vblanks because one makes the input "twitchy"
        sceDisplayWaitVblankStart();
        flipScreen();
    }
sceUtilityOskShutdownStart();
sceUtilityOskShutdownStart();
sceDisplayWaitVblankStart();
    clearScreen(0);
    flipScreen();
    clearScreen(0);
    flipScreen();
    sceGuFinish();
    sceGuSync(0, 0);
    sceGeRestoreContext(&geContext3);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// requestString:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *requestString (char *descStr, char *initialStr){
    clearScreen(0);
    flipScreen();
    clearScreen(0);
    flipScreen();
    int ok, i;
    static char str[64];
    unsigned short intext[128]  = { 0 }; // text already in the edit box on start
    unsigned short desc[128]  = { 0 };

    if (initialStr[0] != 0)
        for (i=0; i<=strlen(initialStr); i++)
            intext[i] = (unsigned short)initialStr[i];

    if (descStr[0] != 0)
        for (i=0; i<=strlen(descStr); i++)
            desc[i] = (unsigned short)descStr[i];

    ok = get_text_osk(str, intext, desc);

    pspDebugScreenInit();
    pspDebugScreenSetBackColor(0xFF000000);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenClear();

    if (ok)
        return str;

    return 0;
}
