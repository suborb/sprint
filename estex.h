/*  estex.h - Defines for OS Estex (Feb 20, 2002)

    Copyright (c) 2002, Alexander Shabarshin (shaos@mail.ru)
    
    Interface to Estex OS
    Copyright (c) 2001, Peters Plus (http://www.petersplus.com)

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

#ifndef _ESTEX_H
#define _ESTEX_H

#pragma pack(1)

typedef struct _SprintEXE_
{ char id[3];
  uint8_t version;
  uint32_t offset;
  uint16_t loader;
  uint16_t loader2;
  uint16_t reserv1;
  uint16_t reserv2;
  uint16_t load;
  uint16_t start;
  uint16_t stack;
  uint8_t res[490];
} SprintEXE;

#pragma pack()

extern int wFont;
extern int hFont;
extern int CurX;
extern int CurY;
extern int ColS;
extern int ColB;
extern int Level;

extern char *Path;
extern char PathA[64];

int Init(void);
int Bios(int);
int Estex(int);
int Mouse(int);
int TestAddress(int adr);
int Find(int adrbuf,int adrnam=-1,int atr=0,int flag=1);

#endif
