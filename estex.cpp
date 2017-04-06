/*  estex.cpp - Call OS Estex (Feb 18, 2002)

    Copyright (c) 2002-2003, Alexander Shabarshin (shaos@mail.ru)
    
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#ifndef WINN2
#include <dirent.h>                        
#include <errno.h> 
#define DIRENT
#endif
#ifdef __WATCOMC__
#include <direct.h>     
#define DIRENT
#endif
#ifdef WIN95
#include <direct.h>
#include <io.h>
#endif
#include "sprint.h"
#include "estex.h"
#include "z80/z80.h"
#include "z80/z_macros.h"
#include "wildcmp.hpp"

#define CF_OFF   F &= ~FLAG_C
#define CF_ON    F |=  FLAG_C
#define ZF_OFF   F &= ~FLAG_Z
#define ZF_ON    F |=  FLAG_Z
#define MAXFILES 16
#define DPRE(s)  if(fdebug!=NULL) fprintf(fdebug,"\n\tEstex:%s ",s)
#define DPRB(s)  if(fdebug!=NULL) fprintf(fdebug,"\n\tBios:%s ",s)
#define DPRM(s)  if(fdebug!=NULL) fprintf(fdebug,"\n\tMouse:%s ",s)
#define DPRE1(s,c) if(fdebug!=NULL) fprintf(fdebug,"\n\tEstex:%s %c ",s,(c)>32?c:' ')
#define DPRE2(s,x) if(fdebug!=NULL) fprintf(fdebug,"\n\tEstex:%s %i ",s,x)
#define DPRB1(s,c) if(fdebug!=NULL) fprintf(fdebug,"\n\tBios:%s %c ",s,(c)>32?c:' ')
#define DPRB2(s,x) if(fdebug!=NULL) fprintf(fdebug,"\n\tBios:%s %4.4X ",s,x)

int wFont = 8;
int hFont = 8;
int CurX = 0;
int CurY = 0;
int ColS = 7;
int ColB = 0;
int LastId = 1;
int Level = 0;
int SaveSP[16];
int SavePages[16][5];
int CurDisk = 0; // A:
int MaxDisk = 1; // the only A:
char *Path = PathA;
char PathA[64] = "";
char PathCur[64] = "";
FILE *OpenFile[MAXFILES];

int Init(void)
{
 int i;
 for(i=0;i<MAXFILES;i++) OpenFile[i]=NULL;
 Level = 0;
 Bios(0xC1);
 ramo[0].occ = 1;
 FILE *f = UniFopen("SYSTEM.DOS","rb");
 if(f==NULL)
 {
   printf("\n\nSYSTEM.DOS not found\n\n");
   return 0;
 }
 fread(ram,16384,1,f);
 fclose(f);
 return 1;
}

int Fopen(char *name,int t)
{
 int i;
 char tip[4];
 char str[32];
 char st2[100];
 if(t==0) strcpy(tip,"r+b");
 if(t==1) strcpy(tip,"rb");
 if(t==2) strcpy(tip,"wb");
 if(fdebug!=NULL) fprintf(fdebug,"Fopen(%s,%i) ",name,t);
 for(i=0;i<(int)strlen(name);i++)
 {
    str[i] = name[i];
    if(str[i]=='\\') str[i]='/';
    if(str[i]==' ') break;
 }
 str[i] = 0;
 for(i=0;i<MAXFILES;i++)
 {
    if(OpenFile[i]==NULL) break;
 } 
 if(i==MAXFILES) return -2;
 OpenFile[i] = fopen(str,tip);
 if(OpenFile[i]==NULL) 
 {
    strcpy(st2,Path);
    strcat(st2,str);
    OpenFile[i] = fopen(st2,tip);
    if(OpenFile[i]==NULL) return -1;
 }
 return i;
}

int Fclose(int f)
{
 if(OpenFile[f]==NULL) return 0;
 fclose(OpenFile[f]);
 OpenFile[f] = NULL;
 return 1;
}

int Estex(int command)
{
 auto int i,j,k,f,a;
 char str[256],*po;
 time_t tt;
 struct tm *tim;
 SprintEXE exe;
 switch(command)
 {
   case 0x01: // CHDISK
	DPRE("CHDISK");
	if(A>MaxDisk) CF_ON;
	else
	{  CF_OFF;
	   A = MaxDisk;
	}
	break;
   case 0x02: // CURDISK
	DPRE("CURDISK");
	CF_OFF;
	A = CurDisk;
	break;
   case 0x0A: // CREATE
   case 0x0B: // CREATE NEW FILE
   case 0x11: // OPEN
	if(command==0x0A) DPRE("CREATE");
	if(command==0x0B) DPRE("CREATE NEW FILE");
	if(command==0x11) DPRE("OPEN");
	CF_OFF;
	j = 0;
	while(1)
	{
	   str[j] = readbyte(HL+j);
	   if(str[j]==0 || j>=255) break;
	   j++;
	}
	str[j] = 0;
	if(command==0x0A||command==0x0B) A=2;
	f = Fopen(str,A);
	if(f<0){A=1;CF_ON;break;}
	A = f; 
	break;
   case 0x12: // CLOSE
	DPRE("CLOSE");
	CF_OFF;
	if(!Fclose(A)){A=1;CF_ON;break;}
	break;
   case 0x13: // READ
	DPRE("READ");
	CF_OFF;
	if(OpenFile[A]==NULL){A=1;CF_ON;break;}
	for(i=0;i<DE;i++) writebyte(HL+i,fgetc(OpenFile[A]));
	break;
   case 0x14: // WRITE
	DPRE("WRITE");
	CF_OFF;
	if(OpenFile[A]==NULL){A=1;CF_ON;break;}
	for(i=0;i<DE;i++) fputc(readbyte(HL+i),OpenFile[A]);
	break;
   case 0x15: // MOVE_FP
	DPRE2("MOVE_FP",(int16_t)IX);
	CF_OFF;
	if(OpenFile[A]==NULL){A=1;CF_ON;break;}
	if(B) k = (int16_t)IX; // ???
	else k = (((int16_t)(HL))<<16)|IX;
	switch(B)
	{
	   case 0: fseek(OpenFile[A],k,SEEK_SET); break;
	   case 1: fseek(OpenFile[A],k,SEEK_CUR); break;
	   case 2: fseek(OpenFile[A],k,SEEK_END); break;
	   default: A=2;CF_ON;break;
	}
	if(!(F & FLAG_C))
	{
	   k = ftell(OpenFile[A]);
	   IX = k & 0xFFFF;
	   HL = k >> 16;
	}
	break;
   case 0x19: // F_FIRST
	DPRE("F_FIRST");
	CF_ON;
	if(Find(DE,HL,A,B)) CF_OFF;
	break;
   case 0x1A: // F_NEXT
	DPRE("F_NEXT");
	CF_ON;
	if(Find(DE)) CF_OFF;
	break;
   case 0x1D: // CHDIR
	DPRE("CHDIR");
	CF_OFF;
	k = 0;
	po = str;
	for(i=0;i<100;i++)
	{
	    str[i] = readbyte(HL+i);
	    if(str[i]==':' && i>0) 
	    {
		k = toupper(str[i-1]);
		po = &str[i+1];
	    }
	    if(str[i]=='\\') str[i]='/';
	    if(!str[i]) break;
	    str[i] = tolower(str[i]);
	}
	if(str[0]=='/') strcpy(PathCur,&str[1]);
	else
	{
	    if(str[0]=='.'&&str[1]=='.'&&str[2]==0)
	    {
		po = strrchr(PathCur,'/');
		if(po!=NULL) *po=0;
	    }
	    else 
	    {
		if(*PathCur) strcat(PathCur,po);	
		else strcat(PathCur,&po[1]);	
	    }
	}
	CurDisk = k-'A';
	break;
   case 0x1E: // CURDIR
	DPRE("CURDIR");
	CF_OFF;
	strcpy(str,"\\");
	strcat(str,PathCur);
	po = str;
	k = HL;
	while(*po)
	{
	    if(*po=='/') *po='\\';
	    writebyte(k++,*po);
	    po++;
	}
	writebyte(k,0);
	break;
   case 0x21: // SYSTIME
	DPRE("SYSTIME");
	CF_OFF;
	tt = time(NULL);
	tim = localtime(&tt);
	if(tim!=NULL)
	{
	    D = tim->tm_mday;
	    E = tim->tm_mon+1;
	    IX = tim->tm_year+1900;
	    H = tim->tm_hour;
	    L = tim->tm_min;
	    B = tim->tm_sec;
	    C = tim->tm_wday;
	    if(!C) C=7;
	}
	sprintf(str,"%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d",D,E,IX,H,L,B);
	DPRE(str);
	break;
   case 0x30: // WAITKEY
	DPRE("WAITKEY");
	do {
	  A = GetKey(); 
	} while(!D);
	break;	
   case 0x31: // SCANKEY
	DPRE("SCANKEY");
	ZF_OFF;
	A = GetKey();
	if(!D) ZF_ON;
	break;
   case 0x32: // ECHOKEY
	DPRE("ECHOKEY");
	Estex(0x30);
	Estex(0x5B);
	break;	
   case 0x33: // CTRLKEY
	DPRE("CTRLKEY");
	A = GetKey(1);
	break;
   case 0x35: // K_CLEAR
	DPRE("K_CLEAR");
	GetKey(-1);
	Estex(B);
	break;
   case 0x37: // TESTKEY
	DPRE("TESTKEY");
	ZF_OFF;
	A = GetKey(1);
	if(!D) ZF_ON;
	break;
   case 0x38: // SETWIN
	DPRE2("SETWIN",A);
	CF_OFF;
	for(i=0;i<RamPages;i++) 
	{
	   if(ramo[i].occ==A)
	   {
	      if(ramo[i].num==B) break;
	   }
	}
	if(i==RamPages)
	{
	   CF_ON;
	   A = 1;
	   break;
	}
	j = H;
	j >>= 6;
        A = i;
	ramw[j] = i;
	sprintf(str,"[%i,%i,%i,%i]",ramw[0],ramw[1],ramw[2],ramw[3]);
	DPRE(str);
	break;
   case 0x39: // SETWIN1
	DPRE("SETWIN1");
	k = H;
	H = 0x40;
	Estex(0x38);
	H = k;
	break;
   case 0x3A: // SETWIN2
	DPRE("SETWIN2");
	k = H;
	H = 0x80;
	Estex(0x38);
	H = k;
	break;
   case 0x3B: // SETWIN3
	DPRE("SETWIN3");
	k = H;
	H = 0xC0;
	Estex(0x38);
	H = k;
	break;
   case 0x3C: // INFOMEM
	DPRE("INFOMEM");
	Bios(0xC0);
	break;
   case 0x3D: // GETMEM
	DPRE("GETMEM");
	Bios(0xC2);
	break;	
   case 0x3E: // FREEMEM
	DPRE("FREEMEM");
	Bios(0xC3);
	break;	
   case 0x40: // EXEC
	DPRE("EXEC");
	CF_OFF;
	j = 0;
	k = -1;
	while(1)
	{
	   str[j] = readbyte(HL+j);
	   if(k<0 && str[j]==' ') k=j;
	   if(str[j]==0 || j>=255) break;
	   j++;
	}
	if(k<0) k=j;
	str[j] = 0;
   	IX = 0x1000;
	writebyte(IX,j-k);
	for(i=k;i<=j;i++) writebyte(IX+(i-k)+1,str[i]);
	writebyte(IX-3,0);
	writebyte(IX-2,0);
	writebyte(IX-1,0);
	f = Fopen(str,1);
	if(f<0){A=1;CF_ON;break;}
	fseek(OpenFile[f],0,SEEK_END);
	k = ftell(OpenFile[f]);
	fseek(OpenFile[f],0,SEEK_SET);
	fread(&exe,1,sizeof(SprintEXE),OpenFile[f]);
	if(memcmp(exe.id,"EXE",3)){fclose(OpenFile[f]);A=2;CF_ON;break;}
	PUSH16(PCL,PCH);    // !!!
	SaveSP[Level] = SP; // !!!
	B=3;Estex(0x3D);
	SavePages[Level][0] = ramw[0];
	SavePages[Level][1] = ramw[1];
	SavePages[Level][2] = ramw[2];
	SavePages[Level][3] = ramw[3];
	SavePages[Level][4] = A;
	for(i=0;i<RamPages;i++) 
	{
	   if(ramo[i].occ==A)
	   {
	      ramw[ramo[i].num+1]=i;
	   }
	}
	fseek(OpenFile[f],exe.offset,SEEK_SET);
	if(exe.loader) 
	{
	   j = exe.loader;
	   for(i=0;i<j;i++) writebyte(exe.load+i,fgetc(OpenFile[f]));
	   writebyte(IX-3,f);
        }
        else
        {
	   j = k-exe.offset;
	   for(i=0;i<j;i++) writebyte(exe.load+i,fgetc(OpenFile[f]));
	   Fclose(f);
	}
	PC = exe.start;
	SP = exe.stack;
        Level++;
	break;	
   case 0x41: // EXIT
	DPRE("EXIT");
	if(!--Level) Exit=1;
	else
	{
	  A = SavePages[Level][4];
          Estex(0x3E);
	  ramw[0] = SavePages[Level][0];
	  ramw[1] = SavePages[Level][1];
	  ramw[2] = SavePages[Level][2];
	  ramw[3] = SavePages[Level][3];
	  SP = SaveSP[Level];
	  RET();
	}
	break;
   case 0x50: // SETVMOD
	DPRE("SETVMOD");
	CF_OFF;
	if(A==0x02 || A==0x03 || A==0x81 || A==0x82)
	{
	   SetVideo(A);
	   VPage = B;
	   OffsetX = 0;
	}  
	else CF_ON;
	break;
   case 0x51: // GETVMOD
	DPRE("GETVMOD");
	CF_OFF;
	A = Video;
	B = VPage;
	break;
   case 0x52: // LOCATE
	DPRE("LOCATE");
	CF_OFF;
	CurX = E;
	CurY = D;
	break;
   case 0x53: // CURSOR
	DPRE("CURSOR");
	CF_OFF;
	D = CurY;
	E = CurX;
	break;
   case 0x54: // SELPAGE
	DPRE("SELPAGE");
	CF_OFF;
        VPage = B;
	break;
   case 0x55: // SCROLL
	DPRE("SCROLL");
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
	if(D+H>32) break;
	if(E+L>80) break;
	Scroll(B,E,D,L,H);
	break;
   case 0x56: // CLEAR
	DPRE("CLEAR");
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
	if(D+H>32) break;
	if(E+L>80) break;
	for(j=D;j<D+H-1;j++){
	for(i=E;i<E+L;i++){
           k = i+j*80;
           TextS[k] = A;
           TextA[k] = B;
           PrintChar(i,j,TextS[k],TextA[k]);
	}}
	break;
   case 0x57: // RDCHAR
	DPRE("RDCHAR");
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
        k = E+D*80;
        A = TextS[k];
        B = TextA[k];
	break;
   case 0x58: // WRCHAR
	DPRE1("WRCHAR",A);
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
        k = E+D*80;
        TextS[k] = A;
        TextA[k] = B;
        PrintChar(E,D,TextS[k],TextA[k]);
	break;
   case 0x59: // WINCOPY
	DPRE("WINCOPY");
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
	if(D+H>32) break;
	if(E+L>80) break;
	a = B;
	a = (a<<14)+(IX&0x3FFF);
	for(j=D;j<D+H;j++){
	for(i=E;i<E+L;i++){
           k = i+j*80;
           ram[a++] = TextS[k];
           ram[a++] = TextA[k];
	}}
	break;
   case 0x5A: // WINREST
	DPRE("WINREST");
	CF_OFF;
	if(D>=32) break;
	if(E>=80) break;
	if(D+H>32) break;
	if(E+L>80) break;
	a = B;
	a = (a<<14)+(IX&0x3FFF);
	for(j=D;j<D+H;j++){
	for(i=E;i<E+L;i++){
           k = i+j*80;
           TextS[k] = ram[a++];
           TextA[k] = ram[a++];
           PrintChar(i,j,TextS[k],TextA[k]);
	}}
	break;
   case 0x5B: // PUTCHAR
	DPRE1("PUTCHAR",A);
	if(A>=32) 
	{
	 PrintChar(CurX++,CurY,A,ColS,ColB);
         if(CurX*wFont > ScrDX-wFont)
         {  CurX = 0;
    	    CurY ++; 
         }
	}
        if(A==13) CurX = 0;
        if(A==10) CurY ++; 
        if(CurY*hFont > ScrDY-hFont)
        {
           Scroll(1,0,0,80,32);
           CurY --;
        }
        break;
   case 0x5C: // PCHARS
	DPRE("PCHARS");
        i = A;
        while(1)
        {  A = readbyte(HL++);
           if(A==0) break;
           Estex(0x5B);
        }
        A = i;
	break;
   default:	
	if(fdebug!=NULL) fprintf(fdebug,"[Unknown Estex command %2.2X] ",command);
#if 1
	RST(0x10);
#else
	CF_ON;
#endif
 }
 return 1;
}


int Bios(int command)
{
 auto int i,j,k;
 char str[256];
 switch(command)
 {
   case 0x81: // LP_PRINT_ALL
	DPRB1("LP_PRINT_ALL",A);
        CF_OFF;
	while(B)
	{
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A,E&0xF,E>>4);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	}
	break;
   case	0x82: // LP_PRINT_SYM 
        DPRB1("LP_PRINT_SYM",A);
        CF_OFF;
	while(B)
	{
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	}
	break;
   case	0x83: // LP_PRINT_ATR
        DPRB2("LP_PRINT_ATR",E);
        CF_OFF;
	while(B)
	{
	    PrintChar(CurX++,CurY,0,E&0xF,E>>4);
    	    if(CurX*wFont > ScrDX-wFont)
    	    {  CurX = 0;
    	       CurY ++; 
  	    }
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	}
	break;
   case	0x84: // LP_SET_PLACE
        DPRB2("LP_SET_PLACE",DE);
        CF_OFF;
        CurX = E;
        CurY = D;
        break;
   case	0x85: // LP_PRINT_LN 
        DPRB2("LP_PRINT_LN",B);
        CF_OFF;
	while(B)
	{
	    A = readbyte(HL);
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A,E&0xF,E>>4);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x86: // LP_PRINT_LN2 
        DPRB2("LP_PRINT_LN2",B);
        CF_OFF;
	while(B)
	{
	    A = readbyte(HL);
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x87: // LP_PRINT_LN3 
        DPRB2("LP_PRINT_LN3",B);
        CF_OFF;
        i = 1;
	while(B)
	{
	    if(i) A = readbyte(HL);
	    else  A = 0x20;
	    if(A==D) i = 0;
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A,E&0xF,E>>4);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x88: // LP_PRINT_LN4
        DPRB2("LP_PRINT_LN4",B);
        CF_OFF;
        i = 1;
	while(B)
	{
	    if(i) A = readbyte(HL);
	    else  A = 0x20;
	    if(A==D) i = 0;
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x89: // LP_CLS_WIN
        DPRB2("LP_CLS_WIN",B);
        CF_OFF;
        for(i=E;i<E+L;i++){
        for(j=D;j<D+H;j++){
           PrintChar(i,j,0x20,B&0xF,B>>4);
        }}
        break;
   case 0x8A: // LP_SCROLL_UD
        DPRB2("LP_SCROLL_UD",B);
        CF_OFF;
        Scroll(B,0,D,80,E);
        break;
   case	0x8B: // LP_PRINT_LN5
        DPRB2("LP_PRINT_LN5",B);
        CF_OFF;
	while(B)
	{
	    A = readbyte(HL);
	    if(A==D) break;
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A,E&0xF,E>>4);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x8C: // LP_PRINT_LN6
        DPRB2("LP_PRINT_LN6",B);
        CF_OFF;
	while(B)
	{
	    A = readbyte(HL);
	    if(A==D) break;
	    if(A>=32) 
	    {
	     PrintChar(CurX++,CurY,A);
    	     if(CurX*wFont > ScrDX-wFont)
    	     {  CurX = 0;
    	        CurY ++; 
    	     }
	    } 
    	    if(A==13) CurX = 0;
    	    if(A==10) CurY ++; 
    	    if(CurY*hFont > ScrDY-hFont){CurX=CurY=0;break;}
//    	    {Scroll(1,0,0,80,32);CurY--;}
	    B--;
	    HL++;
	}
	break;
   case	0x8D: // LP_CLS_WIN2
        DPRB2("LP_CLS_WIN2",B);
        CF_OFF;
        for(i=E;i<E+L;i++){
        for(j=D;j<D+H;j++){
           PrintChar(i,j,A,B&0xF,B>>4);
        }}
        break;
   case	0x8E: // LP_GET_PLACE
        DPRB("LP_GET_PLACE");
        CF_OFF;
        E = CurX;
        D = CurY;
        break;
   case	0xA1: // PIC_POINT
        DPRB("PIC_POINT");
        CF_OFF;
        SetPixel(HL,DE,B);
        break;
   case 0xA4: // PIC_SET_PAL
	DPRB("PIC_SET_PAL");
	SetPalette(HL,E,D,A);
	break;
   case 0xB0: // WIN_OPEN	
	DPRB("WIN_OPEN");
	OffsetX = 64;
	break;
   case 0xC0: // EMM_FN0
	DPRB("EMM_FN0");
        HL = RamPages;
        k = 0;
	for(i=0;i<RamPages;i++) 
	{
	   if(!ramo[i].occ) k++;
	}
	BC = k;
	break;
   case 0xC1: // EMM_FN1
	DPRB("EMM_FN1");
	ramw[0] = 0;
	ramw[1] = 1;
	ramw[2] = 2;
	ramw[3] = 3;
	for(i=0;i<RamPages;i++) 
	{
	   ramo[i].occ = 0;
	   if(i>=0x50 && i<=0x5F)
	   {
		ramo[i].occ = 1;
		ramo[i].num = i-0x50;
	   }
	}
	LastId = 1;
	break;
   case 0xC2: // EMM_FN2
	DPRB("EMM_FN2");
	LastId++;
        k = 0;

if(fdebug!=NULL) fprintf(fdebug," LastID=%i",LastId);

	for(i=0;i<RamPages;i++) 
	{
	   if(!ramo[i].occ) 
	   {

if(fdebug!=NULL) fprintf(fdebug," [%i]=%i",k,i);

	      ramo[i].occ = LastId;
	      ramo[i].num = k++;
	      if(k==B) break;
	   }
	}
	if(k<B)
	{  CF_ON;
	   A = 1;
	}
	else
	{  CF_OFF;
	   A = LastId;
	}

if(fdebug!=NULL) fprintf(fdebug," RET(%i)\n",LastId);

	break;
   case 0xC3: // EMM_FN3
	DPRB("EMM_FN3");
	k = 0;
	for(i=0;i<RamPages;i++) 
	{
	   if(ramo[i].occ==A)
	   {
	      k++;
	      ramo[i].occ = 0;
	   }
	}
	if(k)
	{  CF_OFF; 
	   if(LastId==A) LastId--;
	}
	else
	{  CF_ON;
	}
	break;
   case 0xC4: // EMM_FN4
	DPRB("EMM_FN4");
	for(i=0;i<RamPages;i++) 
	{
	   if(ramo[i].occ==A && ramo[i].num==B) break;
	}
	if(i<RamPages)
	{  CF_OFF;
	   A = i;
	}
	else
	{  CF_ON;
	   A = 0;
	}
	break;
   case 0xC5: // EMM_FN5
	DPRB("EMM_FN5");
	j = HL;
	k = 0;
	for(i=0;i<RamPages;i++) 
	{
	   if(ramo[i].occ==A) 
	   {
	      k++;
	      writebyte(j++,i);
	   }
	}
	if(k)
	{  CF_OFF;
	   B = k;
	   writebyte(j,0xFF);
	}
	else
	{  CF_ON;
	   B = 0;
	}
	break;
   case 0xC6: // EMM_FN6
	DPRB("EMM_FN6");
	CF_OFF;
	switch(A)
	{  case 0: C=PORT_PAGE_0; B=ramw[0]; break;
	   case 1: C=PORT_PAGE_1; B=ramw[1]; break;
	   case 2: C=PORT_PAGE_2; B=ramw[2]; break;
	   case 3: C=PORT_PAGE_3; B=ramw[3]; break;
	   default: CF_ON; break;
	}
	break;
   case 0xC7: // EMM_FN7
	DPRB("EMM_FN7");
	CF_OFF;
	if(A<RamPages && ramo[A].occ)
	{
	   if(A==RamPages-1)
	   {  A = 0xFF;
	      break;
	   }
	   k = ramo[A].occ;
	   j = ramo[A].num + 1;
	   for(i=A+1;i<RamPages;i++)
	   {
	      if(ramo[i].occ==k && ramo[i].num==j) break;
	   }
	   if(i==RamPages) 
	      A = 0xFF;
	   else 
	      A = i;
	}
	else CF_ON;
	break;
   case 0xEF: // FN_VERSION
	DPRB("FN_VERSION");
	sprintf(str,"v0.0 SPRINT (" __DATE__ ")");
	for(i=0;i<=(int)strlen(str);i++) writebyte(HL+i,str[i]);
	writebyte(HL+i+1,0);
	DE = 0x100;
	BC = 0xFFFD;
	A = 2;
	break;
   default:	
        if(fdebug!=NULL) fprintf(fdebug,"[Unknown Bios command %2.2X] ",command);
	CF_ON;
//	return 0;
 }
 return 1;
}


int Mouse(int command)
{
 MouseState *m;
 switch(command)
 {
   case 0x00: // INITIALIZATION
        DPRM("INITIALIZATION");
	CF_OFF;
        break;
   case 0x03: // READ MOUSE STATE
        DPRM("READ MOUSE STATE");
	CF_OFF;
	m = GetMouseState();
	A = m->bl|(m->br<<1);
	HL = m->x;
	DE = m->y;
        if(fdebug!=NULL) fprintf(fdebug,"MOUS%i[%i:%i]\n",A,m->x,m->y);
        break;
   default:	
	if(fdebug!=NULL) fprintf(fdebug,"[Unknown Mouse command %2.2X] ",command);
	CF_ON;
//	return 0;
 }
 return 1;
}

int TestAddress(int adr)
{
 switch(adr)
 {
  case 0x08: Bios(C);  break;
  case 0x10: Estex(C); break;
  case 0x30: Mouse(C); break;
  default:
   return 0;
 }
 return 1;
}

int FindFlag = 1;
int FindAtr = 0;
char FindName[32];
int FindId;

int Find(int adrbuf,int adrnam,int atr,int flag)
{
 auto int j,k;
 char str[256],*po=str;

#ifdef DIRENT

 static DIR *pdir = NULL; 
 struct dirent *pfile = NULL;  
 if(adrnam>=0)
 {  // FIND FIRST
    FindFlag = flag;
    FindAtr = atr;
    for(j=0;j<100;j++)
    {
	str[j] = readbyte(adrnam+j);
	if(str[j]==0) break;
    }
    if(fdebug!=NULL) fprintf(fdebug,"F_FIRST %s #%2.2X ",str,atr);
    if(!strcmp(str,"*.*"))
    {
	strcpy(FindName,str);
	sprintf(str,"%s%s",Path,PathCur);
	if(str[strlen(str)-1]=='/') str[strlen(str)-1]=0;
	pdir = opendir(str);
	if(!pdir) return 0;
	for(j=0;j<11;j++) writebyte(adrbuf+j,'?');
	writebyte(adrbuf+11,FindAtr);
	errno = 0;
	while(1)
	{
	    pfile = readdir(pdir);
	    if(!pfile)
	    {
		if(errno==0) closedir(pdir); 
		return 0;
	    }
	    if(pfile->d_name[0]=='.' && pfile->d_name[1]==0) continue;
//	    if(FindAtr==0x37){}
	    break;
	}
     }
 }
 else
 {  // FIND NEXT
    if(!strcmp(FindName,"*.*"))
    {
	errno = 0;
	pfile = readdir(pdir);
	if(!pfile)
	{
	    if(errno==0) closedir(pdir); 
	    return 0;
	}
    }
 }
 po = pfile->d_name;
 if(fdebug!=NULL) fprintf(fdebug,"=> %s ",po);

#endif

#ifdef WIN95

 static struct _finddata_t fdt;
 if(adrnam>=0)
 {  // FIND FIRST
    FindFlag = flag;
    FindAtr = atr;
    for(j=0;j<100;j++)
    {
	str[j] = readbyte(adrnam+j);
	if(str[j]==0) break;
    }
    if(fdebug!=NULL) fprintf(fdebug,"F_FIRST %s #%2.2X ",str,atr);
    if(!strcmp(str,"*.*"))
    {
	strcpy(FindName,str);
	sprintf(str,"%s%s",Path,PathCur);
	if(str[strlen(str)-1]=='/') str[strlen(str)-1]=0;
	if(str[strlen(str)-1]=='\\') str[strlen(str)-1]=0;
	strcat(str,"\\*.*");
        FindId = _findfirst(str,&fdt);
        if(FindId==0) return 0;
	for(j=0;j<11;j++) writebyte(adrbuf+j,'?');
	writebyte(adrbuf+11,FindAtr);
     }
 }
 else
 {  // FIND NEXT
    if(!strcmp(FindName,"*.*"))
    {
       k = _findnext(FindId,&fdt);
       if(k!=0){_findclose(FindId);return 0;}
    }
 }
 po = fdt.name;
 if(fdebug!=NULL) fprintf(fdebug,"=> %s ",po);

#endif

 sprintf(str,"%s%s%s",Path,PathCur,po);
//if(fdebug!=NULL) fprintf(fdebug," [%s] ",str);
 FILE *f = fopen(str,"r+");
 if(f==NULL)
 {
    atr = 0x10;
    k = 0;
 }
 else
 {
    atr = 0x00;
    fseek(f,0,SEEK_END);
    k = ftell(f);
    fclose(f);
 }
 writebyte(22,0);writebyte(23,0);
 writebyte(24,0);writebyte(25,0);
 writebyte(28,0);writebyte(29,(k&0xFF0000)>>16);
 writebyte(30,(k&0xFF00)>>8);writebyte(31,k&0xFF);
 writebyte(adrbuf+32,atr); 
 if(FindFlag) 
    for(j=0;j<=(int)strlen(po);j++) writebyte(adrbuf+33+j,po[j]);
 else {
    k = 1;
    for(j=0;j<8;j++) 
    {
 	if((j>=2 && *po=='.') || *po==0) k = 0;
 	if(k) writebyte(adrbuf+33+j,toupper(*po++));
 	else  writebyte(adrbuf+33+j,' ');
    }
    if(*po=='.') po++;
    k = 1;
    for(j=0;j<3;j++) 
    {
        if(*po==0) k = 0;
	if(k) writebyte(adrbuf+41+j,toupper(*po++));
	else  writebyte(adrbuf+41+j,' ');
    }
 }
 return 1;
}

