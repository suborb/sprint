/*  unigraf.cpp - Universal Graphics interface  (March 8, 2002)

    Copyright (c) 2002-2003, Alexander Shabarshin (shaos@mail.ru)

    This file is part of SPRINT (Emulator of Sprinter computer).

    SPRINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SPRINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SPRINT; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*

 You can use:

 -DSVGALIB for svgalib
 -DSDL for SDL using (Win9X and Linux)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __WATCOMC__
#include <graph.h>
#include "kbd_rsdn.hpp"
#endif
#ifdef SVGALIB
#include <vga.h>
#include <vgakeyboard.h>
#endif
#ifdef SDL
#include "SDL.h"
#include "SDL_timer.h"
#endif
#include <stdio.h>
#include "unigraf.h"
#ifdef FONT8X8
#include "font8x8a.h"
#endif
#ifdef FONT8X16
#include "font8x16.h"
#endif
#ifdef UNIFOPEN
extern FILE* UniFopen(char *name, char *fmode);
#define Fopen UniFopen
#else
#define Fopen fopen
#endif

const unsigned char MAC_NATIVE_TO_HID[256] = {
  4, 22,  7,  9, 11, 10, 29, 27,  6, 25,255,  5, 20, 26,  8, 21,
 28, 23, 30, 31, 32, 33, 35, 34, 46, 38, 36, 45, 37, 39, 48, 18,
 24, 47, 12, 19, 40, 15, 13, 52, 14, 51, 49, 54, 56, 17, 16, 55,
 43, 44, 53, 42,255, 41,231,227,225, 57,226,224,229,230,228,255,
108, 99,255, 85,255, 87,255, 83,255,255,255, 84, 88,255, 86,109,
110,103, 98, 89, 90, 91, 92, 93, 94, 95,111, 96, 97,255,255,255,
 62, 63, 64, 60, 65, 66,255, 68,255,104,107,105,255, 67,255, 69,
255,106,117, 74, 75, 76, 61, 77, 59, 78, 58, 80, 79, 81, 82,255
};

const unsigned char EVDEV_HID_TO_NATIVE[256] = {
255,255,255,255, 38, 56, 54, 40, 26, 41, 42, 43, 31, 44, 45, 46,
 58, 57, 32, 33, 24, 27, 39, 28, 30, 55, 25, 53, 29, 52, 10, 11,
 12, 13, 14, 15, 16, 17, 18, 19, 36,  9, 22, 23, 65, 20, 21, 34,
 35, 51,255, 47, 48, 49, 59, 60, 61, 66, 67, 68, 69, 70, 71, 72,
 73, 74, 75, 76, 95, 96,107, 78,127,118,110,112,119,115,117,114,
113,116,111, 77,106, 63, 82, 86,104, 87, 88, 89, 83, 84, 85, 79,
 80, 81, 90, 91, 94,255,255,125,191,192,193,194,195,196,197,198,
199,200,201,202,255,255,135,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
 37, 50, 64,133,105, 62,108,134,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

struct UniGrafPrivate
{
 int i;
#ifdef SDL
 int SDLmodif,PALmodif,dx,dy;
 SDL_Surface *screen;
 SDL_Color col[256];
 UniGrafPrivate(){screen=NULL;};
 void ComplainAndExit(void)
 {
  fprintf(stderr, "SDL Problem: %s\n", SDL_GetError());
  SDL_Quit();
  exit(1);
 }
 void SDLpixel(Sint32 X, Sint32 Y, Uint8 c)
 {
  if(X<0 || X>=dx || Y<0 || Y>=dy) return;
  Uint8 *bits;
  if ( SDL_MUSTLOCK(screen) ) {
       if ( SDL_LockSurface(screen) < 0 )
            return;
  }
  bits = ((Uint8 *)screen->pixels)+Y*screen->pitch+X;
  *((Uint8 *)(bits)) = c;
  if ( SDL_MUSTLOCK(screen) ) {
       SDL_UnlockSurface(screen);
  }
 }
#endif
};

#define PRIV ((UniGrafPrivate*)PrivateData)

UniGraf::UniGraf(int m,int w)
{
 int mode = 0;
 graf = 0;
 PrivateData = new UniGrafPrivate;
 pal = new long[256];
 graf = m;
 switch(graf)
 {
   case UG256_320x200:
#ifdef __WATCOMC__
	mode = 19;
#endif
#ifdef SVGALIB
	mode = 5;
#endif
#ifdef SDL
	dx = 320;
	dy = 200;
	mode = 1;
#endif
	break;
   case UG256_640x400:
#ifdef __WATCOMC__
	mode = 0x100;
#endif
#ifdef SVGALIB
	mode = 9;
#endif
#ifdef SDL
	dx = 640;
	dy = 400;
	mode = 2;
#endif
	break;
   case UG256_640x480:
#ifdef __WATCOMC__
	mode = 0x101;
#endif
#ifdef SVGALIB
	mode = 10;
#endif
#ifdef SDL
	dx = 640;
	dy = 480;
	mode = 3;
#endif
	break;
 }
// if(mode<0) return;
 mox = moy = mol = mom = mor = 0;
 for(int i=0;i<256;i++) keys[i]=0;
#ifdef __WATCOMC__
 videoconfig vc;
 _setvideomode(mode);
 _setbkcolor(0);
 _getvideoconfig(&vc);
 dx = vc.numxpixels;
 dy = vc.numypixels;
 keyboardCapture();
#endif
#ifdef SVGALIB
 vga_init();
 if (!(vga_ext_set(VGA_EXT_AVAILABLE, VGA_AVAIL_SET) & (1 << VGA_EXT_PAGE_OFFSET))) {
	puts("You need at least svgalib 1.2.10 to run this program!\n");
	exit(1);
 }
 if (!vga_hasmode(mode)) return;
 vga_setmode(mode);
 vga_modeinfo *ginfo = vga_getmodeinfo(mode);
 dx = ginfo->width;
 dy = ginfo->height;
 if (keyboard_init()) {
	printf("Could not initialize keyboard.\n");
	exit(1);
 }
#endif
#ifdef SDL
 PRIV->SDLmodif = 0;
 PRIV->PALmodif = 0;
 if(PrivateData==NULL) exit(1);
 if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
      PRIV->ComplainAndExit();
 }
 PRIV->dx = dx;
 PRIV->dy = dy;
 unsigned long sdl_mode = SDL_SWSURFACE;
 if(!w) sdl_mode |= SDL_FULLSCREEN;
#ifndef WIN32
 if(!w) sdl_mode |= SDL_HWPALETTE;
#endif
 PRIV->screen = SDL_SetVideoMode(dx, dy, 8, sdl_mode);
 if ( PRIV->screen == NULL ) {
      PRIV->ComplainAndExit();
 }
 printf("SDL Ok (%i)!\n",PRIV->screen->format->BytesPerPixel);
#endif
 image = NULL;
 apy = NULL;
 if(!Screen()) graf = 0;
 keybuf[0] = keybuf[1] = keybuf[2] = 0;
}

int UniGraf::Screen(int dxs,int dys)
{
#ifdef SDL
 PRIV->SDLmodif = 0;
#endif
 dxscr = dxs;
 dyscr = dys;
 if(dxscr==0) dxscr=dx;
 if(dyscr==0) dyscr=dy;
 mx = dx/(double)dxscr;
 my = dy/(double)dyscr;
 px = (short)mx;
 py = (short)my;
 if(mx>(double)px) px++;
 if(my>(double)py) py++;
 if(apy!=NULL) delete apy;
 apy = new short[dyscr];
 if(apy==NULL) return 0;
 for(int i=0;i<dyscr;i++) apy[i]=(short)(my*i);
 if(image!=NULL) delete image;
 image = new unsigned char[dxscr*dyscr];
 if(image==NULL) return 0;
 memset(image,0,dxscr*dyscr);
 return 1;
}

UniGraf::~UniGraf()
{
 Close();
 if(PrivateData!=NULL) delete PrivateData;
}

void UniGraf::Close(void)
{
#ifdef SDL
 SDL_Quit();
#endif
#ifdef __WATCOMC__
 _setvideomode(-1);
 keyboardRelease();
#endif
#ifdef SVGALIB
 vga_setmode(TEXT);
 keyboard_close();
#endif
 if(image!=NULL)
 {
    delete image;
    image = NULL;
 }    
 if(pal!=NULL)
 {
    delete pal;
    pal = NULL;
 }
 graf = 0;
}

int UniGraf::SetPalette(int i,int r,int g,int b)
{
 if(i<0 || i>255) return 0;
 pal[i] = r|(g<<8)|(b<<16);
#ifdef __WATCOMC__
 pal[i] = (pal[i]&0xFCFCFC)>>2;
 _remappalette(i,pal[i]);
#endif
#ifdef SVGALIB
 vga_setpalette(i,r>>2,g>>2,b>>2);
#endif
#ifdef SDL
 for(int j=0;j<256;j++) 
 {
   PRIV->col[j].r =  (pal[j]&0xFF);
   PRIV->col[j].g = ((pal[j]&0xFF00)>>8);
   PRIV->col[j].b = ((pal[j]&0xFF0000)>>16);
 }
 PRIV->SDLmodif = 1;
 PRIV->PALmodif = 1;
#endif
 return 1;
}

long UniGraf::GetPalette(int i)
{
 if(i<0 || i>255) return 0;
 return pal[i];
}

int UniGraf::LoadPalette(char* s)
{  
 int b,g,r,i;
 char ss[100];
 strcpy(ss,s);
 strcat(ss,".pal");
 FILE *f=Fopen(ss,"rb");
 if(f==NULL) return 0;
 for(i=0;i<256;i++)
 {
  r = fgetc(f)<<2;
  g = fgetc(f)<<2;
  b = fgetc(f)<<2;
  if(!SetPalette(i,r,g,b)){fclose(f);return 0;}
 }
 fclose(f);
 return 1;
}

int UniGraf::SetScreenPixel(int x,int y,int c)
{
 if(x<0 || x>=dxscr || y<0 || y>=dyscr) return 0;
#ifdef SDL
 Sint32 X, Y, X1, Y1;
#endif
 image[x+y*dxscr] = c;
 int x0 = (int)(x*mx);
 int y0 = (int)(y*my);
 if(y==dxscr-1) py=dxscr-apy[dxscr-1];
 else py=apy[y+1]-apy[y];
 if(!py) py=1;
 for(int j=0;j<py;j++){
 for(int i=0;i<px;i++){
#ifdef __WATCOMC__
  _setcolor(c);
  _setpixel(x0+i,y0+j);
#endif
#ifdef SVGALIB
  vga_setcolor(c);
  vga_drawpixel(x0+i,y0+j);
#endif
#ifdef SDL
  X = x0 + i;
  Y = y0 + j;
  if(i==0||j==0){X1=X;Y1=Y;} 
  PRIV->SDLpixel(X,Y,c);
#endif
 }}
#ifdef SDL
 PRIV->SDLmodif = 1;
#endif
 return 1;
}

int UniGraf::GetScreenPixel(int x,int y)
{
 if(x<0 || x>=dxscr || y<0 || y>=dyscr) return 0;
 return image[x+y*dxscr];
}

int UniGraf::DrawChar8x8(int x, int y, int c, int s, int b)
{
 auto int k = 0;
 for(int i = 0; i < 8; i++)
 {
#ifdef FONT8X8 
    k = font8x8[c][i];
#endif
    for (int j = 0; j < 8; j++)
    {
        if((k<<j) & 0x80)
          SetScreenPixel(x+j,y+i,s);
        else if(b>=0)
          SetScreenPixel(x+j,y+i,b);
    }
 }
 return 1;
}

int UniGraf::DrawChar8x16(int x, int y, int c, int s, int b)
{
 auto int k = 0;
 for(int i = 0; i < 16; i++)
 {  
#ifdef FONT8X16
    k = font8x16[c][i];
#endif
    for(int j = 0; j < 8; j++)
    {  
        if((k<<j) & 0x80)
          SetScreenPixel(x+j,y+i,s);
	else if(b>=0)
          SetScreenPixel(x+j,y+i,b);
    }
 }
 return 1;
}

int UniGraf::DrawString(int x,int y,char *c,int h,int s,int b)
{
 unsigned char *po = (unsigned char*) c;
 while(*po)
 {
	switch(h)
	{
	    case 8:  
		DrawChar8x8(x,y,*po,s,b);  
		break;
	    case 16: 
		DrawChar8x16(x,y,*po,s,b); 
		break;
	    default:
		return 0;
	}	
	po++;
	x += 8;
 }
 return 1;
}

int UniGraf::Update(void)
{
 int ev = 0;
#ifdef __WATCOMC__
 for(int i=1;i<128;i++) if(keys[i]=keyPressed[i]) ev=1;
#endif
#ifdef SVGALIB
 ev |= keyboard_update();
#endif
#ifdef SDL
 if(PRIV->PALmodif)
 {
   SDL_SetColors(PRIV->screen,PRIV->col,0,256);
 }
 if(PRIV->SDLmodif)
 {
   SDL_UpdateRect(PRIV->screen,0,0,dx,dy);
   PRIV->SDLmodif = 0;
 }
 SDL_Event event;
 while(SDL_PollEvent(&event))  
 {
  switch(event.type)
  {
   case SDL_KEYDOWN:
#ifndef WIN32
#ifdef __APPLE__
        event.key.keysym.scancode = EVDEV_HID_TO_NATIVE[MAC_NATIVE_TO_HID[event.key.keysym.scancode]] - 8;
#endif
	if(event.key.keysym.scancode>0)
#endif 
        keys[event.key.keysym.scancode]=1;
        ev = 1;
        break;
   case SDL_KEYUP:
#ifndef WIN32
#ifdef __APPLE__
        event.key.keysym.scancode = EVDEV_HID_TO_NATIVE[MAC_NATIVE_TO_HID[event.key.keysym.scancode]] - 8;
#endif
	if(event.key.keysym.scancode>0)
#endif 
        keys[event.key.keysym.scancode]=0;
        break;
   case SDL_MOUSEMOTION:
	mox = (int)(event.motion.x/mx);
        moy = (int)(event.motion.y/my);
	break;
   case	SDL_MOUSEBUTTONDOWN: 
        switch(event.button.button)
        {
          case 1: mol = 1; break;
          case 2: mom = 1; break;
          case 3: mor = 1; break;
        }
	mox = (int)(event.button.x/mx);
        moy = (int)(event.button.y/my);
        break;
   case	SDL_MOUSEBUTTONUP: 
        switch(event.button.button)
        {
          case 1: mol = 0; break;
          case 2: mom = 0; break;
          case 3: mor = 0; break;
        }
        break;
  }
  if(ev) break;
 }
 if(!ev) for(int j=1;j<LASTKEY;j++) if(keys[j]) {ev=1;break;}
#endif
 return ev;
}

int UniGraf::KeyPressed(int scan)
{
 int i = 0;
#ifdef __WATCOMC__
 i = keys[scan];
#endif
#ifdef SVGALIB
 i = keyboard_keypressed(scan);
#endif
#ifdef SDL
 i = keys[scan];
#endif
// if(keys[scan]) printf("key %i\n",scan);
 return i;
}

int UniGraf::WhatKey(void)
{
 auto int i,j,k;
 int kk,ok;
 kk = 0;
 for(i=1;i<LASTKEY;i++) 
 {
    k = KeyPressed(i);
    if(k) 
    {	ok = 0;
	for(j=0;j<3;j++) if(keybuf[j]==k) ok=1;
	if(!kk) kk = k;
	if(!ok) break;
    }
 }
 if(i<LASTKEY) kk=i;
 keybuf[2] = keybuf[1];
 keybuf[1] = keybuf[0];
 keybuf[0] = kk;
 return keybuf[0];
}

int UniGraf::KeyClear(void)
{
 keybuf[2] = keybuf[1] = keybuf[0] = 0;
 return 1;
}

