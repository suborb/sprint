/*  unigraf.h - Universal Graphics interface  (March 8, 2002)

    Copyright (c) 2002, Alexander Shabarshin (shaos@mail.ru)

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

#ifndef __UNIGRAF_H
#define __UNIGRAF_H

#define UG256_320x200 0x10
#define UG256_640x400 0x20
#define UG256_640x480 0x21

#define LASTKEY 220

class UniGraf
{
 int dx,dy,dxscr,dyscr,graf;
 double mx,my;
 short px,py,*apy;
 short mox,moy,mol,mom,mor;
 unsigned char *image;
 long *pal;
 int keybuf[3];
 char keys[256];
 void* PrivateData;
public:
 UniGraf(int m,int w);
~UniGraf();
 void Close(void);
 int Screen(int dxs=0,int dys=0);
 int GetScreenWidth(void){return dxscr;}
 int GetScreenHeight(void){return dyscr;}
 int LoadPalette(char* s);
 int SetPalette(int i,int r,int g,int b); 
 long GetPalette(int i); 
 int SetScreenPixel(int x,int y,int c);
 int GetScreenPixel(int x,int y);
 int DrawChar8x8(int x,int y,int c,int s,int b=-1);
 int DrawChar8x16(int x,int y,int c,int s,int b=-1);
 int DrawString(int x,int y,char *c,int h,int s,int b=-1);
 int Update(void);
 int KeyPressed(int scan);
 int WhatKey(void);
 int KeyClear(void);
 int GetMouseX(void){return mox;}
 int GetMouseY(void){return moy;}
 int GetMouseL(void){return mol;}
 int GetMouseM(void){return mom;}
 int GetMouseR(void){return mor;}
};

#endif
