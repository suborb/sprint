/*  sprint.cpp - main part (Feb 18, 2002)

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef SVGALIB
#include <vga.h>
#include <vgakeyboard.h>
#define getch vga_getch
#endif
#ifndef WIN32
#define stricmp strcasecmp
#endif
#ifdef __WATCOMC__
#include <i86.h>
#include <conio.h>
#endif
#ifdef SDL
#include "SDL.h"
#include "SDL_timer.h"
#define delay(x) SDL_Delay(x)
#endif
#include "targa.h"
#include "unigraf.h"
#include "sprint.h"
#include "estex.h"
#include "z80/z80.h"
#include "z80/z_macros.h"
#include "hex.hpp"
#include "loadbmp.hpp"

#define VIDEODEBUG

#ifdef WIN32
#ifndef SDL
#define delay(x) mydelay(x)
void mydelay(int x)
{int tt=(int)(clock()+x/1000.0*CLOCKS_PER_SEC);while(clock()<tt);} 
#endif
#endif

#define COMMSTEP   20000
#define ADVTIME    5
#define PETERSURL  "http://www.shaos.ru/cgi/relink.pl?peters"

#ifdef __WATCOMC__
#define SCANCODE_F1		59
#define SCANCODE_F2             60
#define SCANCODE_F3             61
#define SCANCODE_F4             62
#define SCANCODE_F5             63
#define SCANCODE_F6             64
#define SCANCODE_F7             65
#define SCANCODE_F8             66
#define SCANCODE_F9             67
#define SCANCODE_F10            68
#define SCANCODE_LEFTSHIFT      42
#define SCANCODE_RIGHTSHIFT     54
#define SCANCODE_LEFTCONTROL    29
#define SCANCODE_RIGHTCONTROL   97
#define SCANCODE_LEFTALT        56
#define SCANCODE_RIGHTALT       100
#endif
#ifdef SDL
#define SCANCODE_F1		59
#define SCANCODE_F2             60
#define SCANCODE_F3             61
#define SCANCODE_F4             62
#define SCANCODE_F5             63
#define SCANCODE_F6             64
#define SCANCODE_F7             65
#define SCANCODE_F8             66
#define SCANCODE_F9             67
#define SCANCODE_F10            68
#define SCANCODE_LEFTSHIFT      42
#define SCANCODE_RIGHTSHIFT     54
#define SCANCODE_LEFTCONTROL    29
#define SCANCODE_RIGHTCONTROL   157
#define SCANCODE_LEFTALT        56
#define SCANCODE_RIGHTALT       184
#endif

// Use mode 320x200 if you have only standard VGA card
int VMODE = UG256_640x480;
int Windowed = 0;
int Debug = 0;
int SnapLast = 0;
double Start = 0;
double Stop = 0;
double Hz = 7e6;
DWORD tstates = 0;
DWORD tevent = 0;
BYTE *ram;
int ramw[4];
Page *ramo;
int RamPages = 0;
int Exit;
int ScrDX,ScrDY;
int flagBin = 0;
int Org = 0x8100;
int Stk = 0x80FF;
int Video = 0x03;
int VideoY = 0;
int VPage = 0;
int OffsetX = 0;
int AccelMode = 0; 
int AccelSize = 0;
int AccelFlag = 0;
BYTE AccelRAM[256];
BYTE *TextS;
BYTE *TextA;
UniGraf *ug;
TargaRGB Palette[256];
char ExePath[256] = "";
int AdvWait = ADVTIME;

FILE* UniFopen(char *name, char *fmode)
{
 char sfn[256];
 FILE *f = fopen(name,fmode);
 if(f==NULL)
 {
   sprintf(sfn,"%s%s",ExePath,name);
   f = fopen(sfn,fmode);
 }
 return f;
}

int main(int argc,char **argv)
{
 char str[256],*po;
 auto int i,j,k;
 FILE *f;
 SprintEXE exe;
 printf("\nSPRINT, Copyright (c) 2002-2003, Alexander Shabarshin (shaos@mail.ru)\n\n");
 strcpy(ExePath,argv[0]);
 po = strrchr(ExePath,'/');
 if(po==NULL) po = strrchr(ExePath,'\\');
 if(po!=NULL)
 {
    po++;
    *po = 0;
 }
 else *ExePath = 0;
 f = UniFopen("sprint.ini","rt");
 if(f!=NULL)
 {
   printf("sprint.ini founded!\n");
   while(1)
   {
     fgets(str,100,f);
     if(feof(f)) break;
     j = 0;
     for(i=0;i<(int)strlen(str);i++)
     {
       if(str[i]!='\r' && str[i]!='\n')  str[j++]=tolower(str[i]);
     }
     if(j==0) continue;
     if(*str=='/') continue;
     str[j] = 0;
     po = strchr(str,'=');
     if(po==NULL) continue;
     *po = 0;
     po++;
     printf("Key '%s' has value '%s'\n",str,po);
     if(!stricmp(str,"debug")) Debug=atoi(po);
     else if(!stricmp(str,"stop")) Stop=atof(po);   
     else if(!stricmp(str,"start")) Start=atof(po); 
     else if(!stricmp(str,"tgalast")) SnapLast=atoi(po); 
     else if(!stricmp(str,"mhz")) Hz=1e6*atof(po); 
     else if(!stricmp(str,"a")) strcpy(PathA,po);
     else if(!stricmp(str,"video")) {
    	if(!strcmp(po,"320x200")) VMODE = UG256_320x200;
    	if(!strcmp(po,"640x400")) VMODE = UG256_640x400;
    	if(!strcmp(po,"640x480")) VMODE = UG256_640x480;
    	if(!strcmp(po,"window")) Windowed = 1;
     }
     else if(!stricmp(str,"adv")) AdvWait=atoi(po); 
     else printf("Error %s!\n",str);
   }
   fclose(f);
 }
 ram = new BYTE[RAMSIZE];
 if(ram==NULL)
 {
   printf("\n\nOut of memory 1\n\n");
   return 0;
 }
 RamPages = RAMSIZE>>14;
 printf("RamSize  = %i\n",RAMSIZE);
 printf("RamPages = %i\n",RamPages);
 ramo = new Page[RamPages];
 if(ramo==NULL)
 {
   delete ram;
   printf("\n\nOut of memory 2\n\n");
   return 0;
 }
 for(i=0;i<RamPages;i++) 
 {
   ramo[i].data = &ram[i<<14];
   ramo[i].occ = 0;
 }
 TextS = new BYTE[TEXTLEN];
 if(TextS==NULL)
 {
   delete ram;
   delete ramo;
   printf("\n\nOut of memory 3\n\n");
   return 0;
 }
 TextA = new BYTE[TEXTLEN];
 if(TextA==NULL)
 {
   delete ram;
   delete ramo;
   delete TextS;
   printf("\n\nOut of memory 4\n\n");
   return 0;
 }
 for(i=0;i<TEXTLEN;i++)
 {
   TextS[i] = 0; 
   TextA[i] = 7;
 }
 
 z80_init(Debug);
 Init(); // init estex
 
 if(argc>1)
 {
   if(argc>2) Org=hex2i(argv[2]);
   strcpy(str,argv[1]);
   f = fopen(str,"rb");
   if(f==NULL)
   {
    strcpy(str,Path);
    strcat(str,argv[1]);
    f = fopen(str,"rb");
    if(f==NULL)
    {
     printf("\n\nFile open error!\n\n");
     delete ram;
     return 0;
    } 
   }
   fseek(f,0,SEEK_END);
   k = ftell(f);
   fseek(f,0,SEEK_SET);
   fread(&exe,1,sizeof(SprintEXE),f);
   if(!memcmp(exe.id,"EXE",3))
   {
#if 1
	printf("EXE.version=#%2.2X\n",exe.version);
	printf("EXE.offset=#%4.4X\n",(int)exe.offset);
	printf("EXE.loader=#%4.4X\n",exe.loader);
	printf("EXE.load=#%4.4X\n",exe.load);
	printf("EXE.start=#%4.4X\n",exe.start);
	printf("EXE.stack=#%4.4X\n",exe.stack);
#endif
	fclose(f);
	HL = 0x1000;
	k = HL;
	for(i=1;i<argc;i++)
	{
	    for(j=0;j<(int)strlen(argv[i]);j++) writebyte(k++,argv[i][j]);
	    writebyte(k++,' ');
	}
	writebyte(k-1,0);
	B = 0;
	Estex(0x40);
   }
   else
   {
	fseek(f,0,SEEK_SET);
	for(i=0;i<k;i++) writebyte(Org+i,fgetc(f));
	PC = Org;
	SP = Stk;
	fclose(f);
	Level++;
   }
   flagBin = 1;
 }
 
 ug = new UniGraf(VMODE,Windowed);
 if(ug==NULL) return 0;
 if(!ug->Screen(640,256)) 
    printf("Screen Error!\n");
 if(!ug->LoadPalette("sprint")) 
    printf("Palette Error!\n");;
 ScrDX = 640;
 ScrDY = 256;

 LoadBmp(ug,"sprinter.bmp",160,64);
 ug->DrawString(200,0,"SPRINT - Emulator of Sprinter computer (" __DATE__ ")",8,11,0);
 ug->DrawString(200,9,"Copyright (c) 2002-2003, Alexander Shabarshin",8,10,0);
 ug->DrawString(200,18,"http://shaos.ru/nedopc/sprinter/sprint.htm",8,9,0);
 ug->Update();
 CurY = 8;

 if(fdebug!=NULL) fprintf(fdebug,"\nStart #%4.4X\n",PC);

#ifdef WINDOWS
 long click = 0;
#endif
 int upi = 0;
 long t1,t2,ts;
 double d;
 t1 = clock();
 ts = tstates;

 while(!Exit)
 {
   if(z80.halted) z80.halted=0; // !!!
   if(upi++ >= COMMSTEP && clock()>t1) 
   {  
      ug->Update();
      if(ug->KeyPressed(SCANCODE_F2)) MemorySave();
      if(ug->KeyPressed(SCANCODE_F9)) ScreenShot();
      if(ug->KeyPressed(SCANCODE_F10)) break;
      upi = 0;
      d = (tstates-ts)/Hz;
      t2 = t1 + (int)(d*CLOCKS_PER_SEC);
      while(clock()<t2);
      t1 = clock();
      ts = tstates;
   }  
   tevent += 20;
   z80_do_opcodes(Debug && (int)tstates>=Start);
   if(Stop>0 && (int)tstates>Stop) 
   {
      if(Debug && fdebug!=NULL)
      {  Debug = 0;
         fclose(fdebug);
         fdebug = NULL;
      }	 
      break;
   }
   if(tstates>=0xFFFFFF00)
   {
      tevent = tstates = 0;
   }
 }
 z80_stop(Debug);

 ug->Screen(320,256);
 ug->LoadPalette("sprint");
 LoadBmp(ug,"reklama.bmp",320,256);
 t1 = clock();
 while(clock()-t1 < AdvWait*CLOCKS_PER_SEC)
 {
   ug->Update();
#ifdef WINDOWS
   if(ug->GetMouseL())
   {
//      if(ug->GetMouseX()>0   && ug->GetMouseX()<120 &&
//         ug->GetMouseY()>224 && ug->GetMouseY()<256)
      { click = 1; break; }
   }
#endif
 }

 ug->Close();
 delete ug;
 delete ram;
 delete ramo;
 delete TextS;
 delete TextA;

#ifdef WINDOWS
 if(click) system("start " PETERSURL);
#endif

 return 1;
}

int VideoWrite(int page,int offs,int byte)
{
 int i,j,k,a,s,b,c,b1,b2,VideoX; 
 if(page<0x50 || page>0x5F) return 0;
 VideoX = offs;
#ifdef VIDEODEBUG
 if(fdebug!=NULL) fprintf(fdebug,"Video%2.2X/%2.2X(%i:%i)=%2.2X ",Video,page,VideoX,VideoY,byte);
#endif
 if(Video==0x03)  
 {
    k = VideoX-768; // ???
    i = VideoY-129; // ???
    j = k>>2;
    a = i+j*80;
    if(a<0 || a>=TEXTLEN) return 0;
    if((k&3)==2) TextA[a]=byte;
    if((k&3)==1) TextS[a]=byte;
    c = TextS[a];
    if(c==0) c=' ';
    s = (TextA[a] & 0x0F);
    b = (TextA[a] & 0x70) >> 4;
    PrintChar(i,j,c,s,b,0);
 }
 else if(Video==0x81)  
 {
    VideoX -= OffsetX;
    ug->SetScreenPixel(VideoX,VideoY,byte); 
 }
 else if(Video==0x82)  
 {  
    VideoX -= OffsetX;
    b1 = byte >> 4;
    b2 = byte & 0xF; 
    VideoX <<= 1;
    ug->SetScreenPixel(VideoX,VideoY,b1); 
    ug->SetScreenPixel(VideoX+1,VideoY,b2); 
 }
 return 1;
}

int VideoRead(int page,int offs)
{
 int VideoX,k,x,y,a=0,byte=0; 
 if(page<0x50 || page>0x5F) return -1;
 VideoX = offs;
 if(Video==0x03)
 {
    k = VideoX-768; // ???
    x = VideoY-129; // ???
    y = k>>2;
    a = x+y*80;
    if(a>=0 && a<TEXTLEN)
    {
      if((k&3)==2) byte=TextA[a];
      if((k&3)==1) byte=TextS[a];
    }  
 }
 else if(Video==0x81)  
 {
    VideoX -= OffsetX;
    if(VideoX>=0 && VideoX<ScrDX && VideoY>=0 && VideoY<ScrDY)
       byte=ug->GetScreenPixel(VideoX,VideoY); 
 }
 else if(Video==0x82)  
 {  
    VideoX -= OffsetX;
    if(VideoX>=0 && VideoX<ScrDX && VideoY>=0 && VideoY<ScrDY)
       byte =  ((ug->GetScreenPixel(VideoX<<1,VideoY)&0xF)<<4) |
		(ug->GetScreenPixel((VideoX<<1)+1,VideoY)&0xF); 
 }
#ifdef VIDEODEBUG
 if(fdebug!=NULL) fprintf(fdebug,"VRead%2.2X/%2.2X(%i:%i)=%2.2X ",Video,page,VideoX,VideoY,byte);
#endif
 return byte;
}

BYTE readbyte(WORD address,BYTE flag)
{
 auto short bb;
 auto BYTE b = 0;
 auto WORD ad = address&0x3FFF;
 switch(address&0xC000)
 {
   case 0x0000: 
        bb = VideoRead(ramw[0],ad); 
	if(bb>=0) b=bb; 
	else 
	b = ramo[ramw[0]].data[ad]; 
	break;
   case 0x4000: 
        bb = VideoRead(ramw[1],ad); 
	if(bb>=0) b=bb; 
	else 
	b = ramo[ramw[1]].data[ad]; 
	break;
   case 0x8000: 
        bb = VideoRead(ramw[2],ad); 
	if(bb>=0) b=bb; 
	else 
	b = ramo[ramw[2]].data[ad]; 
	break;
   case 0xC000: 
        bb = VideoRead(ramw[3],ad); 
	if(bb>=0) b=bb; 
	else 
	b = ramo[ramw[3]].data[ad]; 
	break;
 }
 if(AccelMode && flag && !AccelFlag) Accel('R',b,address);
 return b;
}

BYTE writebyte(WORD address,BYTE data)
{
 auto BYTE b = 0;
 auto WORD ad = address&0x3FFF;
 switch(address&0xC000)
 {
   case 0x0000: 
	b = ramo[ramw[0]].data[ad]; 
	ramo[ramw[0]].data[ad] = data; 
	VideoWrite(ramw[0],ad,data);
	break;
   case 0x4000: 
	b = ramo[ramw[1]].data[ad]; 
	ramo[ramw[1]].data[ad] = data; 
	VideoWrite(ramw[1],ad,data);
	break;
   case 0x8000: 
	b = ramo[ramw[2]].data[ad]; 
	ramo[ramw[2]].data[ad] = data; 
	VideoWrite(ramw[2],ad,data);
	break;
   case 0xC000: 
	b = ramo[ramw[3]].data[ad]; 
	ramo[ramw[3]].data[ad] = data; 
	VideoWrite(ramw[3],ad,data);
	break;
 }
 if(AccelMode && !AccelFlag) Accel('W',data,address);
 return b;
}

BYTE readport(WORD port)
{
 auto BYTE b = 0xFF;
 auto BYTE po = port & 0xFF;
 auto BYTE hi = port >> 8;
 switch(po)
 {
   case PORT_PAGE_0: b=ramw[0]; break; // 0x82
   case PORT_PAGE_1: b=ramw[1]; break; // 0xA2
   case PORT_PAGE_2: b=ramw[2]; break; // 0xC2
   case PORT_PAGE_3: b=ramw[3]; break; // 0xE2
   case 0x18: b=ug->WhatKey(); break;
   case 0x19: if(ug->Update()) b=1; break;
   case 0x89: b=VideoY; break;
   case 0xFD:
	if(hi==0x1F) 
	{  // system port (#1FFD)
     
	}
        break;
 }
 return b;
}

void writeport(WORD port,BYTE b)
{
 auto BYTE po = port & 0xFF;
 auto BYTE hi = port >> 8;
 switch(po)
 {
   case PORT_PAGE_0: ramw[0]=b; break; // 0x82
   case PORT_PAGE_1: ramw[1]=b; break; // 0xA2
   case PORT_PAGE_2: ramw[2]=b; break; // 0xC2
   case PORT_PAGE_3: ramw[3]=b; break; // 0xE2
   case 0x89: VideoY=b; break;
   case 0xFD:
	if(hi==0x1F) 
	{  // system port (#1FFD)
     
	}
	break;
 }
}

void PrintChar(short x0,short y0,short ch,short cs,short cb,short f)
{
 if(f)
 {
    int a = x0+y0*80;
    if(a>=0 && a<TEXTLEN)
    {
       if(ch) TextS[a] = ch;
       if(cs>=0)
       {
         if(cb<0) TextA[a] = cs;
         else TextA[a] = cs + (cb<<4);
       }
       ch = TextS[a];
       cs = TextA[a]&15;
       cb = TextA[a]>>4;
    }
 }
 if(hFont==8)  ug -> DrawChar8x8  (x0<<3,y0<<3,ch,cs,cb);
 if(hFont==16) ug -> DrawChar8x16 (x0<<3,y0<<4,ch,cs,cb);
} 

void SetPalette(int pal,short base,short len,short num)
{
 int i,n,r,g,b;
 n = num;
 if(len==0) len=256;
 for(i=0;i<len;i++)
 {
    b = readbyte(pal+(i<<2)+0);
    g = readbyte(pal+(i<<2)+1);
    r = readbyte(pal+(i<<2)+2);
    Palette[base+i].Red   = r;
    Palette[base+i].Green = g;
    Palette[base+i].Blue  = b;
    ug->SetPalette(base+i,r,g,b);
 }
}

void Scroll(int b,int e,int d,int l,int h,int a)
{
        int i,j,k;
	if(b==1) // up
	{
	  for(j=d;j<d+h-1;j++){
	  for(i=e;i<e+l;i++){
             k = i+j*80;
             TextS[k] = TextS[k+80];
             TextA[k] = TextA[k+80];
             PrintChar(i,j,TextS[k],TextA[k]);
	  }}
	  if(a==0)
	  {
	   for(i=e;i<e+l;i++){
	     k = i+j*80;
             TextS[k] = 0x20;
             PrintChar(i,j,TextS[k],TextA[k]);
	   }
	  }
        }
        if(b==2) // down
        {
	  for(j=d+h-1;j>0;j--){
	  for(i=e;i<e+l;i++){
             k = i+j*80;
             TextS[k] = TextS[k-80];
             TextA[k] = TextA[k-80];
             PrintChar(i,j,TextS[k],TextA[k]);
	  }}
	  if(a==0)
	  {
	   for(i=E;i<E+L;i++){
             TextS[i] = 0x20;
             PrintChar(i,0,TextS[i],TextA[i]);
	   }
	  }
        }
}

int Accel(int a,int b,int adr)
{
 auto int i;
 AccelFlag = 1;
 switch(AccelMode) {
 
 case 0: // LD B,B - switch off
  AccelFlag = 0;
  return 0;
  
 case 1: // LD D,D - set size of accelerator buffer
  if(a=='R')
  { AccelSize = b;
    if(AccelSize==0) AccelSize=256;
  }      
  break;
  
 case 2: // LD C,C - fill in memory
  if(a=='W') 
  {
    for(i=0;i<AccelSize;i++) writebyte(adr+i,b);
  }
  break;      
  
 case 3: // LD E,E - fill in video
  if(a=='W')
  {
    for(i=0;i<AccelSize;i++)
    {   
        writebyte(adr,b);
        VideoY = 0xFF&(VideoY+1);
    }
  }
  break;
  
case 4: // reserved
  break;      
  
case 5: // LD L,L - copy in memory
  switch(a)
  {
   case 'R': for(i=0;i<AccelSize;i++) AccelRAM[i] = readbyte(adr+i);  break;
   case 'W': for(i=0;i<AccelSize;i++) writebyte(adr+i,AccelRAM[i]);   break;
   case '^': for(i=0;i<AccelSize;i++) AccelRAM[i] ^= readbyte(adr+i); break;
   case '|': for(i=0;i<AccelSize;i++) AccelRAM[i] |= readbyte(adr+i); break;
   case '&': for(i=0;i<AccelSize;i++) AccelRAM[i] &= readbyte(adr+i); break;
  }
  break;      
  
case 6: // LD A,A - copy in video
  switch(a)
  {
   case 'R':  
    for(i=0;i<AccelSize;i++)
    {   
        AccelRAM[i] = readbyte(adr);
        VideoY = 0xFF&(VideoY+1);
    }
    break;
   case 'W':  
    for(i=0;i<AccelSize;i++)
    {   
        writebyte(adr,AccelRAM[i]);
        VideoY = 0xFF&(VideoY+1);
    }
    break;
   case '^':
    for(i=0;i<AccelSize;i++)
    {   
        AccelRAM[i] ^= readbyte(adr);
        VideoY = 0xFF&(VideoY+1);
    }
    break;
   case '|':
    for(i=0;i<AccelSize;i++)
    {   
        AccelRAM[i] |= readbyte(adr);
        VideoY = 0xFF&(VideoY+1);
    }
    break;
   case '&':
    for(i=0;i<AccelSize;i++)
    {   
        AccelRAM[i] &= readbyte(adr);
        VideoY = 0xFF&(VideoY+1);
    }
    break;
  }
  break;      
  
 }
 AccelFlag = 0;
 return 1;
}

void SetVideo(int vid)
{
 wFont = 8;
 hFont = 8;
 switch(vid)
 {
   case 0x02:
	ug->Screen(320,256);
	ScrDX = 320;
	ScrDY = 256;
	break;
   case 0x03:
	ug->Screen(640,256);
	ScrDX = 640;
	ScrDY = 256;
	break;
   case 0x81:
	ug->Screen(320,256);
	ScrDX = 320;
	ScrDY = 256;
	break;
   case 0x82:
	ug->Screen(640,256);
	ScrDX = 640;
	ScrDY = 256;
	break;
 }
 Video = vid;
}

void ScreenShot(void)
{
 auto int i,j;
 char file[32],name[64];
 sprintf(file,"Sp%6.6d.tga",SnapLast++);
 sprintf(name,"SPRINT Screenshot %s http://shaos.ru/nedopc/",file);
 TargaFile tga(file,ScrDX,ScrDY,name);
 for(j=0;j<ScrDY;j++)
  for(i=0;i<ScrDX;i++) 
   tga.PutPixel(Palette[ug->GetScreenPixel(i,j)]);
 delay(200);
}

// 0-pos 1-char 2-shift 3-rus 4-rus+shift 5-numlock
static char _keys_[256][6] = {
{0x00,0x00,0x00,0x00,0x00,0x00}, // 00h (0)
{0x01,0x1B,0x1B,0x1B,0x1B,0xFF}, // 01h (1) - Esc
{0x02, '1', '!', '1', '!',0xFF}, // 02h (2)
{0x03, '2', '@', '2', '"',0xFF}, // 03h (3)
{0x04, '3', '#', '3', '#',0xFF}, // 04h (4)
{0x05, '4', '$', '4', ';',0xFF}, // 05h (5)
{0x06, '5', '%', '5', ':',0xFF}, // 06h (6)
{0x07, '6', '^', '6', ',',0xFF}, // 07h (7)
{0x08, '7', '&', '7', '.',0xFF}, // 08h (8)
{0x09, '8', '*', '8', '*',0xFF}, // 09h (9)
{0x0A, '9', '(', '9', '(',0xFF}, // 0Ah (10)
{0x0B, '0', ')', '0', ')',0xFF}, // 0Bh (11)
{0x0C, '-', '_', '-', '_',0xFF}, // 0Ch (12)
{0x0D, '=', '+', '=', '+',0xFF}, // 0Dh (13)
{0x0E,0x08,0x08,0x08,0x08,0xFF}, // 0Eh (14) - BackSpace
{0x0F,0x09,0x09,0x09,0x09,0xFF}, // 0Fh (15) - Tab
{0x10, 'q', 'Q', 'q', 'Q',0xFF}, // 10h (16)
{0x11, 'w', 'W', 'w', 'W',0xFF}, // 11h (17)
{0x12, 'e', 'E', 'e', 'E',0xFF}, // 12h (18)
{0x13, 'r', 'R', 'r', 'R',0xFF}, // 13h (19)
{0x14, 't', 'T', 't', 'T',0xFF}, // 14h (20)
{0x15, 'y', 'Y', 'y', 'Y',0xFF}, // 15h (21)
{0x16, 'u', 'U', 'u', 'U',0xFF}, // 16h (22)
{0x17, 'i', 'I', 'i', 'I',0xFF}, // 17h (23)
{0x18, 'o', 'O', 'o', 'O',0xFF}, // 18h (24)
{0x19, 'p', 'P', 'p', 'P',0xFF}, // 19h (25)
{0x1A, '[', '{', '[', '{',0xFF}, // 1Ah (26)
{0x1B, ']', '}', ']', '}',0xFF}, // 1Bh (27) 
{0x28,0x0D,0x0D,0x0D,0x0D,0xFF}, // 28h (28) - Enter
{0x36,0x00,0x00,0x00,0x00,0x00}, // 36h (29) - Left Ctrl
{0x1D, 'a', 'A', 'a', 'A',0xFF}, // 1Dh (30)
{0x1E, 's', 'S', 's', 'S',0xFF}, // 1Eh (31)
{0x1F, 'd', 'D', 'd', 'D',0xFF}, // 1Fh (32)
{0x20, 'f', 'F', 'f', 'F',0xFF}, // 20h (33)
{0x21, 'g', 'G', 'g', 'G',0xFF}, // 21h (34)
{0x22, 'h', 'H', 'h', 'H',0xFF}, // 22h (35)
{0x23, 'j', 'J', 'j', 'J',0xFF}, // 23h (36)
{0x24, 'k', 'K', 'k', 'K',0xFF}, // 24h (37)
{0x25, 'l', 'L', 'l', 'L',0xFF}, // 25h (38)
{0x26, ';', ':', ';', ':',0xFF}, // 26h (39)
{0x27,0x27, '"',0x27, '"',0xFF}, // 27h (40) - '
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (41) - SCANCODE_GRAVE
{0x29,0x00,0x00,0x00,0x00,0x00}, // 29h (42) - Left Shift
{0x35,0x2F, '|',0x2F, '|',0xFF}, // 35h (43) - Backslash
{0x2A, 'z', 'Z', 'z', 'Z',0xFF}, // 2Ah (44)
{0x2B, 'x', 'X', 'x', 'X',0xFF}, // 2Bh (45)
{0x2C, 'c', 'C', 'c', 'C',0xFF}, // 2Ch (46)
{0x2D, 'v', 'V', 'v', 'V',0xFF}, // 2Dh (47)
{0x2E, 'b', 'B', 'b', 'B',0xFF}, // 2Eh (48)
{0x2F, 'n', 'N', 'n', 'N',0xFF}, // 2Fh (49)
{0x30, 'm', 'M', 'm', 'M',0xFF}, // 30h (50)
{0x31, ',', '<', ',', '<',0xFF}, // 31h (51)
{0x32, '.', '>', '.', '>',0xFF}, // 32h (52)
{0x33, '/', '?', '/', '/',0xFF}, // 33h (53)
{0x34,0x00,0x00,0x00,0x00,0x00}, // 34h (54) - Right Shift
{0x4B, '*', '*', '*', '*', '*'}, // 4Bh (55) - Gray Mul
{0x37,0x00,0x00,0x00,0x00,0x00}, // 37h (56) - Left Alt
{0x38, ' ', ' ', ' ', ' ',0xFF}, // 38h (57) - Space
{0x1C,0x00,0x00,0x00,0x00,0x00}, // 1Ch (58) - CapsLock
{0x3B,0x00,0x00,0x00,0x00,0x00}, // 3Bh (59) - F1
{0x3C,0x00,0x00,0x00,0x00,0x00}, // 3Ch (60) - F2
{0x3D,0x00,0x00,0x00,0x00,0x00}, // 3Dh (61) - F3
{0x3E,0x00,0x00,0x00,0x00,0x00}, // 3Eh (62) - F4
{0x3F,0x00,0x00,0x00,0x00,0x00}, // 3Fh (63) - F5
{0x40,0x00,0x00,0x00,0x00,0x00}, // 40h (64) - F6
{0x41,0x00,0x00,0x00,0x00,0x00}, // 41h (65) - F7
{0x42,0x00,0x00,0x00,0x00,0x00}, // 42h (66) - F8
{0x43,0x00,0x00,0x00,0x00,0x00}, // 43h (67) - F9
{0x44,0x00,0x00,0x00,0x00,0x00}, // 44h (68) - F10
{0x49,0x00,0x00,0x00,0x00,0x00}, // 49h (69) - Num Lock
{0x48,0x00,0x00,0x00,0x00,0x00}, // 48h (70) - Scroll Lock
{0x57,0x00,0x00,0x00,0x00, '7'}, // 57h (71) - Home 7
{0x58,0x00,0x00,0x00,0x00, '8'}, // 58h (72) - Up 8
{0x59,0x00,0x00,0x00,0x00, '9'}, // 59h (73) - PgUp 9
{0x4C, '-', '-', '-', '-', '-'}, // 4Ch (74) - Gray Minus
{0x54,0x00,0x00,0x00,0x00, '4'}, // 54h (75) - Left 4
{0x55,0x00,0x00,0x00,0x00, '5'}, // 55h (76) - 5
{0x56,0x00,0x00,0x00,0x00, '6'}, // 56h (77) - Right 6
{0x4D, '+', '+', '+', '+', '+'}, // 4Dh (78) - Gray Plus
{0x51,0x00,0x00,0x00,0x00, '1'}, // 51h (79) - End 1
{0x52,0x00,0x00,0x00,0x00, '2'}, // 52h (80) - Down 2
{0x53,0x00,0x00,0x00,0x00, '3'}, // 53h (81) - PgDn 3
{0x50,0x00,0x00,0x00,0x00, '0'}, // 50h (82) - Ins 0
{0x4F,0x00,0x00,0x00,0x00, '.'}, // 4Fh (83) - Del .
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (84) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (85) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (86) - SCANCODE_LESS
{0x45,0x00,0x00,0x00,0x00,0x00}, // 45h (87) - F11
{0x46,0x00,0x00,0x00,0x00,0x00}, // 46h (88) - F12
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (89) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (90) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (91) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (92) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (93) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (94) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (95) - ?
{0x4E,0x0D,0x0D,0x0D,0x0D,0x0D}, // 4Eh (96) - Enter
{0x3A,0x00,0x00,0x00,0x00,0x00}, // 3Ah (97) - Right Ctrl
{0x4A, '/', '/', '/', '/', '/'}, // 4Ah (98) - Gray Div
{0x47,0x00,0x00,0x00,0x00,0x00}, // 47h (99) - Print Screen
{0x39,0x00,0x00,0x00,0x00,0x00}, // 39h (100) - Right Alt
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (101) - SCANCODE_BREAK
{0x57,0x00,0x00,0x00,0x00,0x00}, // ??h (102) - SCANCODE_HOME
{0x58,0x00,0x00,0x00,0x00,0x00}, // ??h (103) - SCANCODE_CURSORBLOCKUP
{0x59,0x00,0x00,0x00,0x00,0x00}, // ??h (104) - SCANCODE_PAGEUP
{0x54,0x00,0x00,0x00,0x00,0x00}, // ??h (105) - SCANCODE_CURSORBLOCKLEFT
{0x56,0x00,0x00,0x00,0x00,0x00}, // ??h (106) - SCANCODE_CURSORBLOCKRIGHT
{0x51,0x00,0x00,0x00,0x00,0x00}, // ??h (107) - SCANCODE_END
{0x52,0x00,0x00,0x00,0x00,0x00}, // ??h (108) - SCANCODE_CURSORBLOCKDOWN
{0x53,0x00,0x00,0x00,0x00,0x00}, // ??h (109) - SCANCODE_PAGEDOWN
{0x50,0x00,0x00,0x00,0x00,0x00}, // ??h (110) - SCANCODE_INSERT
{0x4F,0x00,0x00,0x00,0x00,0x00}, // ??h (111) - SCANCODE_REMOVE
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (112) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (113) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (114) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (115) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (116) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (117) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (118) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (119) - SCANCODE_BREAK_ALTERNATIVE
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (120) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (121) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (122) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (123) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (124) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (125) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (126) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (127) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (128) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (129) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (130) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (131) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (132) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (133) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (134) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (135) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (136) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (137) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (138) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (139) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (140) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (141) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (142) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (143) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (144) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (145) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (146) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (147) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (148) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (149) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (150) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (151) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (152) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (153) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (154) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (155) - ?
{0x4E,0x0D,0x0D,0x0D,0x0D,0x0D}, // 4Eh (156) - Gray Enter
{0x3A,0x00,0x00,0x00,0x00,0x00}, // 3Ah (157) - Right Ctrl
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (158) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (159) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (160) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (161) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (162) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (163) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (164) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (165) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (166) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (167) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (168) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (169) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (170) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (171) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (172) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (173) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (174) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (175) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (176) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (177) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (178) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (179) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (180) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (181) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (182) - ?
{0x47,0x00,0x00,0x00,0x00,0x00}, // 47h (183) - Print Screen
{0x39,0x00,0x00,0x00,0x00,0x00}, // 39h (184) - Right Alt
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (185) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (186) - ?
{0x4A, '/', '/', '/', '/', '/'}, // 4Ah (187) - Gray Div
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (188) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (189) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (190) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (191) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (192) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (193) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (194) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (195) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (196) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (197) - Pause
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (198) - ?
{0x57,0x00,0x00,0x00,0x00,0x00}, // ??h (199) - Home
{0x58,0x00,0x00,0x00,0x00,0x00}, // ??h (200) - Up
{0x59,0x00,0x00,0x00,0x00,0x00}, // ??h (201) - Page Up
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (202) - ?
{0x54,0x00,0x00,0x00,0x00,0x00}, // ??h (203) - Left
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (204) - ?
{0x56,0x00,0x00,0x00,0x00,0x00}, // ??h (205) - Right
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (206) - ?
{0x51,0x00,0x00,0x00,0x00,0x00}, // ??h (207) - End
{0x52,0x00,0x00,0x00,0x00,0x00}, // ??h (208) - Down
{0x53,0x00,0x00,0x00,0x00,0x00}, // ??h (209) - Page Down
{0x50,0x00,0x00,0x00,0x00,0x00}, // ??h (210) - Insert
{0x4F,0x00,0x00,0x00,0x00,0x00}, // ??h (211) - Delete
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (212) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (213) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (214) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (215) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (216) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (217) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (218) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (219) - ?
{0x00,0x00,0x00,0x00,0x00,0x00}, // ??h (220) - ?
};

int KeyRusLat = 0;
int KeyNumLock = 0;
int KeyScrollLock = 0;
int KeyInsert = 0;
int KeyCapsLock = 0;
int Update = 1;

// flag=0  - normal
// flag=1  - keep
// flag=-1 - clear buffer
int GetKey(int flag)
{
 int i,k=1;
 switch(flag)
 {
  case 0:    
    k = ug->Update();
    Update = 1; 
    break;
  case 1:
    if(Update) 
    {  k = ug->Update();
       if(!k) 
          Update = 1;
       else
          Update = 0;
    }
    else  Update = 1;
    break;
  case -1: 
    ug->KeyClear();
    return 0;
 }
 B = C = D = E = 0;
 if(!k) return 0;
 if(KeyRusLat)     C |= 0x80;
 if(KeyNumLock)    C |= 0x08;
 if(KeyScrollLock) C |= 0x04;
 if(KeyInsert)     C |= 0x02;
 if(KeyCapsLock)   C |= 0x01;
 int LeftShift  = (ug->KeyPressed(SCANCODE_LEFTSHIFT))?0x80:0;
 int RightShift = (ug->KeyPressed(SCANCODE_RIGHTSHIFT))?0x40:0;
 int LeftCtrl   = (ug->KeyPressed(SCANCODE_LEFTCONTROL))?0x08:0;
 int RightCtrl  = (ug->KeyPressed(SCANCODE_RIGHTCONTROL))?0x02:0;
 int LeftAlt    = (ug->KeyPressed(SCANCODE_LEFTALT))?0x04:0;
 int RightAlt   = (ug->KeyPressed(SCANCODE_RIGHTALT))?0x01:0;
 int Ctrl       = (RightCtrl&&LeftCtrl)?0x20:0;
 int Alt        = (RightAlt&&LeftAlt)?0x10:0;
 int Shift      = (RightShift&&LeftShift);
 B = LeftShift|RightShift|Ctrl|Alt|LeftCtrl|LeftAlt|RightCtrl|RightAlt;
 for(i=0;i<LASTKEY;i++)
 {
    if(i==SCANCODE_LEFTSHIFT) continue;
    if(i==SCANCODE_RIGHTSHIFT) continue;
    if(i==SCANCODE_LEFTCONTROL) continue;
    if(i==SCANCODE_RIGHTCONTROL) continue;
    if(i==SCANCODE_LEFTALT) continue;
    if(i==SCANCODE_RIGHTALT) continue;
    if(ug->KeyPressed(i)) break;
 }
 if(i==LASTKEY) i=0;
 D = _keys_[i][0];
 if(B) D |= 0x80;
 if(Shift)
    E = _keys_[i][2];
 else
    E = _keys_[i][1];
 if(D||E)
 {
  if(fdebug!=NULL) fprintf(fdebug,"\n>>> KBD #%2.2X #%2.2X (%c)\n",D,E,(E>32)?E:' ');
//  if(E&&!flag) 
    delay(120);
 } 
 return E;
}

MouseState* GetMouseState(void)
{
 static MouseState m;
 m.x = ug->GetMouseX();
 m.y = ug->GetMouseY();
 m.br = ug->GetMouseR();
 m.bm = ug->GetMouseM();
 m.bl = ug->GetMouseL();
 return &m;
}

void SetPixel(short x,short y,short c)
{
 ug->SetScreenPixel(x,y,c); 
}

void MemorySave(void)
{
 FILE *f = fopen("memory.dat","wb");
 if(f==NULL) return;
 for(int i=0;i<65536;i++) fputc(readbyte(i),f);
 fclose(f);
}
