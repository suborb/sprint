/*  sprint.h - SPRINT defines (Feb 18, 2002)

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

#ifndef _SPRINT_H
#define _SPRINT_H

#include <inttypes.h>

typedef int8_t SBYTE;
typedef uint8_t BYTE;
typedef int16_t SWORD;
typedef uint16_t WORD;
typedef int64_t SDWORD;
typedef uint64_t DWORD;

#define PORT_PAGE_0 0x82
#define PORT_PAGE_1 0xA2
#define PORT_PAGE_2 0xC2
#define PORT_PAGE_3 0xE2

extern DWORD tstates;
extern DWORD tevent;

BYTE readbyte(WORD address,BYTE flag=1);
BYTE writebyte(WORD address,BYTE data);
BYTE readport(WORD port);
void writeport(WORD port,BYTE b);

#define RAMSIZE 4194304 // 4M

typedef struct _Page
{ BYTE occ;
  BYTE num;
  BYTE *data;
} Page;

#define TEXTLEN 2560

extern BYTE *ram;
extern int ramw[4];
extern Page *ramo;
extern int RamPages;
extern int ScrDX,ScrDY;
extern int Exit;
extern int Video;
extern int VPage;
extern int OffsetX;
extern int AccelMode; 
extern BYTE *TextS;
extern BYTE *TextA;

int Accel(int a,int b,int adr);
void PrintChar(short x0,short y0,short ch,short cs=-1,short cb=-1,short f=1);
void SetPalette(int,short,short,short);
void SetPixel(short,short,short);
void Scroll(int b,int e,int d,int l,int h,int a=0);
void SetVideo(int);
void ScreenShot(void);
int GetKey(int flag=0);
void MemorySave(void);

struct MouseState
{
 short x,y;
 unsigned char bl,bm,br;
};

MouseState* GetMouseState(void);

FILE* UniFopen(char *name, char *fmode);

#endif	

