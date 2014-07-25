#include "danzeff.h"
#include "../Graphics/Graphics.h"

#include <malloc.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <png.h>

#define false 0
#define true 1

int holding = false;     //user is holding a button
int dirty = true;        //keyboard needs redrawing
int shifted = false;     //user is holding shift
int mode = 0;             //charset selected. (0 - letters or 1 - numbers)
int initialized = false; //keyboard is initialized

//Position on the 3-3 grid the user has selected (range 0-2)
int selected_x = 1;
int selected_y = 1;

int moved_x = 0, moved_y = 0; // location that we are moved to

//Variable describing where each of the images is
#define guiStringsSize 12 /* size of guistrings array */
#include "Images.c"

#define MODE_COUNT 2
//this is the layout of the keyboard
char modeChar[MODE_COUNT*2][3][3][5] =
{
   {   //standard letters
      { ",abc",  ".def","!ghi" },
      { "-jkl","\010m n", "?opq" },
      { "(rst",  ":uvw",")xyz" }
   },

   {   //capital letters
      { "^ABC",  "@DEF","*GHI" },
      { "_JKL","\010M N", "\"OPQ" },
      { "=RST",  ";UVW","/XYZ" }
   },

   {   //numbers
      { "\0\0\0001","\0\0\0002","\0\0\0003" },
      { "\0\0\0004",  "\010\0 5","\0\0\0006" },
      { "\0\0\0007","\0\0\0008", "\0\00009" }
   },

   {   //special characters
      { "'(.)",  "\"<'>","-[_]" },
      { "!{?}","\010\0 \0", "+\\=/" },
      { ":@;#",  "~$`%","*^|&" }
   }
};

int danzeff_isinitialized()
{
   return initialized;
}

int danzeff_dirty()
{
   return dirty;
}



unsigned int danzeff_readInput(SceCtrlData pspctrl)
{
   //Work out where the analog stick is selecting
   int x = 1;
   int y = 1;
   if (pspctrl.Lx < 85)     x -= 1;
   else if (pspctrl.Lx > 170) x += 1;

   if (pspctrl.Ly < 85)     y -= 1;
   else if (pspctrl.Ly > 170) y += 1;

   if (selected_x != x || selected_y != y) //If they've moved, update dirty
   {
      dirty = true;
      selected_x = x;
      selected_y = y;
   }
   //if they are changing shift then that makes it dirty too
   if ((!shifted && (pspctrl.Buttons & PSP_CTRL_RTRIGGER)) || (shifted && !(pspctrl.Buttons & PSP_CTRL_RTRIGGER)))
      dirty = true;

   unsigned int pressed = 0; //character they have entered, 0 as that means 'nothing'
   shifted = (pspctrl.Buttons & PSP_CTRL_RTRIGGER)?true:false;

   if (!holding)
   {
      if (pspctrl.Buttons& (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE)) //pressing a char select button
      {
         int innerChoice = 0;
         if      (pspctrl.Buttons & PSP_CTRL_TRIANGLE)
            innerChoice = 0;
         else if (pspctrl.Buttons & PSP_CTRL_SQUARE)
            innerChoice = 1;
         else if (pspctrl.Buttons & PSP_CTRL_CROSS)
            innerChoice = 2;
         else //if (pspctrl.Buttons & PSP_CTRL_CIRCLE)
            innerChoice = 3;

         //Now grab the value out of the array
         pressed = modeChar[ mode*2 + shifted][y][x][innerChoice];
      }
      else if (pspctrl.Buttons& PSP_CTRL_LTRIGGER) //toggle mode
      {
         dirty = true;
         mode++;
         mode %= MODE_COUNT;
      }
      else if (pspctrl.Buttons& PSP_CTRL_DOWN)
      {
         pressed = '\n';
      }
      else if (pspctrl.Buttons& PSP_CTRL_UP)
      {
         pressed = 8; //backspace
      }
      else if (pspctrl.Buttons& PSP_CTRL_LEFT)
      {
         pressed = DANZEFF_LEFT; //LEFT
      }
      else if (pspctrl.Buttons& PSP_CTRL_RIGHT)
      {
         pressed = DANZEFF_RIGHT; //RIGHT
      }
      else if (pspctrl.Buttons& PSP_CTRL_SELECT)
      {
         pressed = DANZEFF_SELECT; //SELECT
      }
      else if (pspctrl.Buttons& PSP_CTRL_START)
      {
         pressed = DANZEFF_START; //START
      }
   }

   holding = pspctrl.Buttons & ~PSP_CTRL_RTRIGGER; //RTRIGGER doesn't set holding

   return pressed;
}

LPP_Surface* keyTextures[guiStringsSize];

void surface_draw_offset(LPP_Surface* surface, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight)
{
    LPPG_BlitSurfaceScreen(screenX, screenY, surface, 255, 0.0, offsetX, offsetY, intWidth, intHeight);
}

/* load all the guibits that make up the OSK */
void danzeff_load()
{
   if (initialized) return;

   keyTextures[0] = LPPG_LoadImageFMem(keys_start, keys_size, "");
   keyTextures[1] = LPPG_LoadImageFMem(keys_t_start, keys_t_size, "");
   keyTextures[2] = LPPG_LoadImageFMem(keys_s_start, keys_s_size, "");
   keyTextures[3] = LPPG_LoadImageFMem(keys_c_start, keys_c_size, "");
   keyTextures[4] = LPPG_LoadImageFMem(keys_c_t_start, keys_c_t_size, "");
   keyTextures[5] = LPPG_LoadImageFMem(keys_s_c_start, keys_s_c_size, "");
   keyTextures[6] = LPPG_LoadImageFMem(nums_start, nums_size, "");
   keyTextures[7] = LPPG_LoadImageFMem(nums_t_start, nums_t_size, "");
   keyTextures[8] = LPPG_LoadImageFMem(nums_s_start, nums_s_size, "");
   keyTextures[9] = LPPG_LoadImageFMem(nums_c_start, nums_c_size, "");
   keyTextures[10] = LPPG_LoadImageFMem(nums_c_t_start, nums_c_t_size, "");
   keyTextures[11] = LPPG_LoadImageFMem(nums_s_c_start, nums_s_c_size, "");

   initialized = true;
}

/* remove all the guibits from memory */
void danzeff_free()
{
   if (!initialized) return;

   int a;
   for (a = 0; a < guiStringsSize; a++)
   {
      LPPG_FreeSurface(keyTextures[a]);
      keyTextures[a] = NULL;
   }
   initialized = false;
}

/* blit the images to screen */
void danzeff_render()
{
   dirty = false;

   if (selected_x == 1 && selected_y == 1)
        {
      surface_draw_offset(keyTextures[6*mode + shifted*3], moved_x, moved_y, 0, 0, keyTextures[6*mode + shifted*3]->width, keyTextures[6*mode + shifted*3]->height);
        }
   else
        {
      surface_draw_offset(keyTextures[6*mode + shifted*3 + 1], moved_x, moved_y, 0, 0, keyTextures[6*mode + shifted*3 + 1]->width, keyTextures[6*mode + shifted*3 + 1]->height);
        }

        surface_draw_offset(keyTextures[6*mode + shifted*3 + 2], selected_x*43 + moved_x, selected_y*43 + moved_y, selected_x*64,selected_y*64, 64, 64);
}

void danzeff_moveTo(const int newX, const int newY)
{
   moved_x = newX;
   moved_y = newY;
}
