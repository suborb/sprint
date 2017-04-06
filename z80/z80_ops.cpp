/* z80_ops.c: Process the next opcode
   Copyright (c) 1999-2000 Philip Kendall
   Modified by Alexander Shabarshin (shaos@mail.ru) in 2002

   $Id: z80_ops.c,v 1.13 2001/11/20 01:20:15 pak21 Exp $

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

   Author contact information:

   E-mail: pak@ast.cam.ac.uk
   Postal address: 15 Crescent Road, Wokingham, Berks, RG40 2DB, England

*/

#include <stdio.h>
#include "../sprint.h"
#include "../estex.h"
#include "z_macros.h"
#include "z80.h"

#define DON
#define DPR0(s)      if(flag_db) fprintf(fdebug,"%s",s);
#define DPR1(s,x)    if(flag_db) fprintf(fdebug,"%s\t#%2.2X",s,(x));
#define DPR2(s,x,y)  if(flag_db) fprintf(fdebug,"%s\t#%4.4X",s,(x)*256+(y));
#define DPR3(s,z)    if(flag_db) fprintf(fdebug,"%s\t#%4.4X",s,z);
#define DPR4(s,x,y)  if(flag_db) fprintf(fdebug,s,(x)*256+(y));
#define DPR5(s,z)    if(flag_db) fprintf(fdebug,s,z);

#ifndef HAVE_ENOUGH_MEMORY
static void z80_cbxx(BYTE opcode2);
static void z80_ddxx(BYTE opcode2);
static void z80_edxx(BYTE opcode2);
static void z80_fdxx(BYTE opcode2);
static void z80_ddfdcbxx(BYTE opcode3, WORD tempaddr);
#endif				/* #ifndef HAVE_ENOUGH_MEMORY */

int flag_db = 0;
WORD old_pc = 0;

/* Execute Z80 opcodes until the next event */
void z80_do_opcodes(int debug)
{
  register WORD wordtemp;
  register BYTE bytetemp;
  
  while(tevent > tstates) {

    BYTE opcode;

    /* If the z80 is HALTed, execute a NOP-equivalent and loop again */
    if(z80.halted) {
      tstates+=4;
      continue;
    }

    opcode=readbyte(PC++,0); R++;

#ifdef DON
    if(debug && fdebug!=NULL && old_pc!=PC-1)
    {  flag_db = 1;
       old_pc = PC-1;	
       fprintf(fdebug,"%i)\t[%4.4X]=%2.2X %2.2X-%2.2X\t",
                       (int)tstates,PC-1,opcode,readbyte(PC,0),readbyte(PC+1,0));
    }
    else 
       flag_db = 0;
#endif

    switch(opcode) {
    case 0x00:		/* NOP */
      tstates+=4;
#ifdef DON
DPR0("NOP")
#endif
      break;
    case 0x01:		/* LD BC,nnnn */
      tstates+=10;
      C=readbyte(PC++);
      B=readbyte(PC++);
#ifdef DON
DPR2("LD BC,",B,C)
#endif
      break;
    case 0x02:		/* LD (BC),A */
      tstates+=7;
      writebyte(BC,A);
#ifdef DON
DPR0("LD (BC),A")
#endif
      break;
    case 0x03:		/* INC BC */
      tstates+=6;
      BC++;
#ifdef DON
DPR0("INC BC")
#endif
      break;
    case 0x04:		/* INC B */
      tstates+=4;
      INC(B);
#ifdef DON
DPR0("INC B")
#endif
      break;
    case 0x05:		/* DEC B */
      tstates+=4;
      DEC(B);
#ifdef DON
DPR0("DEC B")
#endif
      break;
    case 0x06:		/* LD B,nn */
      tstates+=7;
      B=readbyte(PC++);
#ifdef DON
DPR1("LD B,",B)
#endif
      break;
    case 0x07:		/* RLCA */
      tstates+=4;
      A = ( A << 1 ) | ( A >> 7 );
      F = ( F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
	( A & ( FLAG_C | FLAG_3 | FLAG_5 ) );
#ifdef DON
DPR0("RLCA")
#endif
      break;
    case 0x08:		/* EX AF,AF' */
      tstates+=4;
      wordtemp=AF; AF=AF_; AF_=wordtemp;
#ifdef DON
DPR0("EX AF,AF'")
#endif
      break;
    case 0x09:		/* ADD HL,BC */
      tstates+=11;
      ADD16(HL,BC);
#ifdef DON
DPR0("ADD HL,BC")
#endif
      break;
    case 0x0a:		/* LD A,(BC) */
      tstates+=7;
      A=readbyte(BC);
#ifdef DON
DPR0("LD A,(BC)")
#endif
      break;
    case 0x0b:		/* DEC BC */
      tstates+=6;
      BC--;
#ifdef DON
DPR0("DEC BC")
#endif
      break;
    case 0x0c:		/* INC C */
      tstates+=4;
      INC(C);
#ifdef DON
DPR0("INC C")
#endif
      break;
    case 0x0d:		/* DEC C */
      tstates+=4;
      DEC(C);
#ifdef DON
DPR0("DEC C")
#endif
      break;
    case 0x0e:		/* LD C,nn */
      tstates+=7;
      C=readbyte(PC++);
#ifdef DON
DPR1("LD C,",C)
#endif
      break;
    case 0x0f:		/* RRCA */
      tstates+=4;
      F = ( F & ( FLAG_P | FLAG_Z | FLAG_S ) ) | ( A & FLAG_C );
      A = ( A >> 1) | ( A << 7 );
      F |= ( A & ( FLAG_3 | FLAG_5 ) );
#ifdef DON
DPR0("RRCA")
#endif
      break;
    case 0x10:		/* DJNZ offset */
      tstates+=8;
      B--;
      if(B) { tstates+=5; JR(); }
      PC++;
#ifdef DON
DPR0("DJNZ offset")
#endif
      break;
    case 0x11:		/* LD DE,nnnn */
      tstates+=10;
      E=readbyte(PC++);
      D=readbyte(PC++);
#ifdef DON
DPR2("LD DE,",D,E)
#endif
      break;
    case 0x12:		/* LD (DE),A */
      tstates+=7;
      writebyte(DE,A);
#ifdef DON
DPR0("LD (DE),A")
#endif
      break;
    case 0x13:		/* INC DE */
      tstates+=6;
      DE++;
#ifdef DON
DPR0("INC DE")
#endif
      break;
    case 0x14:		/* INC D */
      tstates+=4;
      INC(D);
#ifdef DON
DPR0("INC D")
#endif
      break;
    case 0x15:		/* DEC D */
      tstates+=4;
      DEC(D);
#ifdef DON
DPR0("DEC D")
#endif
      break;
    case 0x16:		/* LD D,nn */
      tstates+=7;
      D=readbyte(PC++);
#ifdef DON
DPR1("LD D,",D)
#endif
      break;
    case 0x17:		/* RLA */
      tstates+=4;
	bytetemp = A;
	A = ( A << 1 ) | ( F & FLAG_C );
	F = ( F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
	  ( A & ( FLAG_3 | FLAG_5 ) ) | ( bytetemp >> 7 );
#ifdef DON
DPR0("RLA")
#endif
      break;
    case 0x18:		/* JR offset */
      tstates+=12;
      JR();
      PC++;
#ifdef DON
DPR0("JR offset")
#endif
      break;
    case 0x19:		/* ADD HL,DE */
      tstates+=11;
      ADD16(HL,DE);
#ifdef DON
DPR0("ADD HL,DE")
#endif
      break;
    case 0x1a:		/* LD A,(DE) */
      tstates+=7;
      A=readbyte(DE);
#ifdef DON
DPR0("LD A,(DE)")
#endif
      break;
    case 0x1b:		/* DEC DE */
      tstates+=6;
      DE--;
#ifdef DON
DPR0("DEC DE")
#endif
      break;
    case 0x1c:		/* INC E */
      tstates+=4;
      INC(E);
#ifdef DON
DPR0("INC E")
#endif
      break;
    case 0x1d:		/* DEC E */
      tstates+=4;
      DEC(E);
#ifdef DON
DPR0("DEC E")
#endif
      break;
    case 0x1e:		/* LD E,nn */
      tstates+=7;
      E=readbyte(PC++);
#ifdef DON
DPR1("LD E,",E)
#endif
      break;
    case 0x1f:		/* RRA */
      tstates+=4;
	bytetemp = A;
	A = ( A >> 1 ) | ( F << 7 );
	F = ( F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
	  ( A & ( FLAG_3 | FLAG_5 ) ) | ( bytetemp & FLAG_C ) ;
#ifdef DON
DPR0("RRA")
#endif
      break;
    case 0x20:		/* JR NZ,offset */
      tstates+=7;
      if( ! ( F & FLAG_Z ) ) { tstates+=5; JR(); }
      PC++;
#ifdef DON
DPR0("JR NZ,offset")
#endif
      break;
    case 0x21:		/* LD HL,nnnn */
      tstates+=10;
      L=readbyte(PC++);
      H=readbyte(PC++);
#ifdef DON
DPR2("LD HL,",H,L)
#endif
      break;
    case 0x22:		/* LD (nnnn),HL */
      tstates+=16;
#ifdef DON
DPR4("LD (#%4.4X),HL",readbyte(PC+1,0),readbyte(PC,0))
#endif
      LD16_NNRR(L,H);
      break;
    case 0x23:		/* INC HL */
      tstates+=6;
      HL++;
#ifdef DON
DPR0("INC HL")
#endif
      break;
    case 0x24:		/* INC H */
      tstates+=4;
      INC(H);
#ifdef DON
DPR0("INC H")
#endif
      break;
    case 0x25:		/* DEC H */
      tstates+=4;
      DEC(H);
#ifdef DON
DPR0("DEC H")
#endif
      break;
    case 0x26:		/* LD H,nn */
      tstates+=7;
      H=readbyte(PC++);
#ifdef DON
DPR1("LD H,",H)
#endif
      break;
    case 0x27:		/* DAA */
      tstates+=4;
      {
	BYTE add = 0,carry= ( F & FLAG_C );
	if( ( F & FLAG_H ) || ( (A & 0x0f)>9 ) ) add=6;
	if( carry || (A > 0x9f ) ) add|=0x60;
	if( A > 0x99 ) carry=1;
	if ( F & FLAG_N ) {
	  SUB(add);
	} else {
	  if( (A>0x90) && ( (A & 0x0f)>9) ) add|=0x60;
	  ADD(add);
	}
	F = ( F & ~( FLAG_C | FLAG_P) ) | carry | parity_table[A];
      }
#ifdef DON
DPR0("DAA")
#endif
      break;
    case 0x28:		/* JR Z,offset */
      tstates+=7;
      if( F & FLAG_Z ) { tstates+=5; JR(); }
      PC++;
#ifdef DON
DPR0("JR Z,offset")
#endif
      break;
    case 0x29:		/* ADD HL,HL */
      tstates+=11;
      ADD16(HL,HL);
#ifdef DON
DPR0("ADD HL,HL")
#endif
      break;
    case 0x2a:		/* LD HL,(nnnn) */
      tstates+=16;
#ifdef DON
DPR4("LD HL,(#%4.4X)",readbyte(PC+1,0),readbyte(PC,0))
#endif
      LD16_RRNN(L,H);
      break;
    case 0x2b:		/* DEC HL */
      tstates+=6;
      HL--;
#ifdef DON
DPR0("DEC HL")
#endif
      break;
    case 0x2c:		/* INC L */
      tstates+=4;
      INC(L);
#ifdef DON
DPR0("INC L")
#endif
      break;
    case 0x2d:		/* DEC L */
      tstates+=4;
      DEC(L);
#ifdef DON
DPR0("DEC L")
#endif
      break;
    case 0x2e:		/* LD L,nn */
      tstates+=7;
      L=readbyte(PC++);
#ifdef DON
DPR1("LD L,",L)
#endif
      break;
    case 0x2f:		/* CPL */
      tstates+=4;
      A ^= 0xff;
      F = ( F & ( FLAG_C | FLAG_P | FLAG_Z | FLAG_S ) ) |
	( A & ( FLAG_3 | FLAG_5 ) ) | ( FLAG_N | FLAG_H );
#ifdef DON
DPR0("CPL")
#endif
      break;
    case 0x30:		/* JR NC,offset */
      tstates+=7;
      if( ! ( F & FLAG_C ) ) { tstates+=5; JR(); }
      PC++;
#ifdef DON
DPR0("JR NC,offset")
#endif
      break;
    case 0x31:		/* LD SP,nnnn */
      tstates+=10;
      SPL=readbyte(PC++);
      SPH=readbyte(PC++);
#ifdef DON
DPR2("LD SP,",SPH,SPL)
#endif
      break;
    case 0x32:		/* LD (nnnn),A */
      tstates+=13;
      {
	wordtemp=readbyte(PC++);
	wordtemp|=readbyte(PC++) << 8;
	writebyte(wordtemp,A);
      }
#ifdef DON
DPR5("LD (#%4.4X),A",wordtemp)
#endif
      break;
    case 0x33:		/* INC SP */
      tstates+=6;
      SP++;
#ifdef DON
DPR0("INC SP")
#endif
      break;
    case 0x34:		/* INC (HL) */
      tstates+=11;
	bytetemp=readbyte(HL);
	INC(bytetemp);
	writebyte(HL,bytetemp);
#ifdef DON
DPR0("INC (HL)")
#endif
      break;
    case 0x35:		/* DEC (HL) */
      tstates+=11;
	bytetemp=readbyte(HL);
	DEC(bytetemp);
	writebyte(HL,bytetemp);
#ifdef DON
DPR0("DEC (HL)")
#endif
      break;
    case 0x36:		/* LD (HL),nn */
      tstates+=10;
      writebyte(HL,readbyte(PC++));
#ifdef DON
DPR1("LD (HL),",readbyte(PC-1,0))
#endif
      break;
    case 0x37:		/* SCF */
      tstates+=4;
      F = F | FLAG_C;
#ifdef DON
DPR0("SCF")
#endif
      break;
    case 0x38:		/* JR C,offset */
      tstates+=7;
      if( F & FLAG_C ) { tstates+=5; JR(); }
      PC++;
#ifdef DON
DPR0("JR C,offset")
#endif
      break;
    case 0x39:		/* ADD HL,SP */
      tstates+=11;
      ADD16(HL,SP);
#ifdef DON
DPR0("ADD HL,SP")
#endif
      break;
    case 0x3a:		/* LD A,(nnnn) */
      tstates+=13;
      {
	wordtemp=readbyte(PC++);
	wordtemp|= ( readbyte(PC++) << 8 );
	A=readbyte(wordtemp);
      }
#ifdef DON
DPR5("LD A,(#%4.4X)",wordtemp)
#endif
      break;
    case 0x3b:		/* DEC SP */
      tstates+=6;
      SP--;
#ifdef DON
DPR0("DEC SP")
#endif
      break;
    case 0x3c:		/* INC A */
      tstates+=4;
      INC(A);
#ifdef DON
DPR0("INC A")
#endif
      break;
    case 0x3d:		/* DEC A */
      tstates+=4;
      DEC(A);
#ifdef DON
DPR0("DEC A")
#endif
      break;
    case 0x3e:		/* LD A,nn */
      tstates+=7;
      A=readbyte(PC++);
#ifdef DON
DPR1("LD A,",A)
#endif
      break;
    case 0x3f:		/* CCF */
      tstates+=4;
      F = ( F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
	( ( F & FLAG_C ) ? FLAG_H : FLAG_C ) | ( A & ( FLAG_3 | FLAG_5 ) );
#ifdef DON
DPR0("CCF")
#endif
      break;
    case 0x40:		/* LD B,B */
      tstates+=4;
      AccelMode = 0;
#ifdef DON
DPR0("LD B,B")
#endif
      break;
    case 0x41:		/* LD B,C */
      tstates+=4;
      B=C;
#ifdef DON
DPR0("LD B,C")
#endif
      break;
    case 0x42:		/* LD B,D */
      tstates+=4;
      B=D;
#ifdef DON
DPR0("LD B,D")
#endif
      break;
    case 0x43:		/* LD B,E */
      tstates+=4;
      B=E;
#ifdef DON
DPR0("LD B,E")
#endif
      break;
    case 0x44:		/* LD B,H */
      tstates+=4;
      B=H;
#ifdef DON
DPR0("LD B,H")
#endif
      break;
    case 0x45:		/* LD B,L */
      tstates+=4;
      B=L;
#ifdef DON
DPR0("LD B,L")
#endif
      break;
    case 0x46:		/* LD B,(HL) */
      tstates+=7;
      B=readbyte(HL);
#ifdef DON
DPR0("LD B,(HL)")
#endif
      break;
    case 0x47:		/* LD B,A */
      tstates+=4;
      B=A;
#ifdef DON
DPR0("LD B,A")
#endif
      break;
    case 0x48:		/* LD C,B */
      tstates+=4;
      C=B;
#ifdef DON
DPR0("LD C,B")
#endif
      break;
    case 0x49:		/* LD C,C */
      tstates+=4;
      AccelMode = 2;
#ifdef DON
DPR0("LD C,C")
#endif
      break;
    case 0x4a:		/* LD C,D */
      tstates+=4;
      C=D;
#ifdef DON
DPR0("LD C,D")
#endif
      break;
    case 0x4b:		/* LD C,E */
      tstates+=4;
      C=E;
#ifdef DON
DPR0("LD C,E")
#endif
      break;
    case 0x4c:		/* LD C,H */
      tstates+=4;
      C=H;
#ifdef DON
DPR0("LD C,H")
#endif
      break;
    case 0x4d:		/* LD C,L */
      tstates+=4;
      C=L;
#ifdef DON
DPR0("LD C,L")
#endif
      break;
    case 0x4e:		/* LD C,(HL) */
      tstates+=7;
      C=readbyte(HL);
#ifdef DON
DPR0("LD C,(HL)")
#endif
      break;
    case 0x4f:		/* LD C,A */
      tstates+=4;
      C=A;
#ifdef DON
DPR0("LD C,A")
#endif
      break;
    case 0x50:		/* LD D,B */
      tstates+=4;
      D=B;
#ifdef DON
DPR0("LD D,B")
#endif
      break;
    case 0x51:		/* LD D,C */
      tstates+=4;
      D=C;
#ifdef DON
DPR0("LD D,C")
#endif
      break;
    case 0x52:		/* LD D,D */
      tstates+=4;
      AccelMode = 1;
#ifdef DON
DPR0("LD D,D")
#endif
      break;
    case 0x53:		/* LD D,E */
      tstates+=4;
      D=E;
#ifdef DON
DPR0("LD D,E")
#endif
      break;
    case 0x54:		/* LD D,H */
      tstates+=4;
      D=H;
#ifdef DON
DPR0("LD D,H")
#endif
      break;
    case 0x55:		/* LD D,L */
      tstates+=4;
      D=L;
#ifdef DON
DPR0("LD D,L")
#endif
      break;
    case 0x56:		/* LD D,(HL) */
      tstates+=7;
      D=readbyte(HL);
#ifdef DON
DPR0("LD D,(HL)")
#endif
      break;
    case 0x57:		/* LD D,A */
      tstates+=4;
      D=A;
#ifdef DON
DPR0("LD D,A")
#endif
      break;
    case 0x58:		/* LD E,B */
      tstates+=4;
      E=B;
#ifdef DON
DPR0("LD E,B")
#endif
      break;
    case 0x59:		/* LD E,C */
      tstates+=4;
      E=C;
#ifdef DON
DPR0("LD E,C")
#endif
      break;
    case 0x5a:		/* LD E,D */
      tstates+=4;
      E=D;
#ifdef DON
DPR0("LD E,D")
#endif
      break;
    case 0x5b:		/* LD E,E */
      tstates+=4;
      AccelMode = 3;
#ifdef DON
DPR0("LD E,E")
#endif
      break;
    case 0x5c:		/* LD E,H */
      tstates+=4;
      E=H;
#ifdef DON
DPR0("LD E,H")
#endif
      break;
    case 0x5d:		/* LD E,L */
      tstates+=4;
      E=L;
#ifdef DON
DPR0("LD E,L")
#endif
      break;
    case 0x5e:		/* LD E,(HL) */
      tstates+=7;
      E=readbyte(HL);
#ifdef DON
DPR0("LD E,(HL)")
#endif
      break;
    case 0x5f:		/* LD E,A */
      tstates+=4;
      E=A;
#ifdef DON
DPR0("LD E,A")
#endif
      break;
    case 0x60:		/* LD H,B */
      tstates+=4;
      H=B;
#ifdef DON
DPR0("LD H,B")
#endif
      break;
    case 0x61:		/* LD H,C */
      tstates+=4;
      H=C;
#ifdef DON
DPR0("LD H,C")
#endif
      break;
    case 0x62:		/* LD H,D */
      tstates+=4;
      H=D;
#ifdef DON
DPR0("LD H,D")
#endif
      break;
    case 0x63:		/* LD H,E */
      tstates+=4;
      H=E;
#ifdef DON
DPR0("LD H,E")
#endif
      break;
    case 0x64:		/* LD H,H */
      tstates+=4;
      AccelMode = 4;
#ifdef DON
DPR0("LD H,H")
#endif
      break;
    case 0x65:		/* LD H,L */
      tstates+=4;
      H=L;
#ifdef DON
DPR0("LD H,L")
#endif
      break;
    case 0x66:		/* LD H,(HL) */
      tstates+=7;
      H=readbyte(HL);
#ifdef DON
DPR0("LD H,(HL)")
#endif
      break;
    case 0x67:		/* LD H,A */
      tstates+=4;
      H=A;
#ifdef DON
DPR0("LD H,A")
#endif
      break;
    case 0x68:		/* LD L,B */
      tstates+=4;
      L=B;
#ifdef DON
DPR0("LD L,B")
#endif
      break;
    case 0x69:		/* LD L,C */
      tstates+=4;
      L=C;
#ifdef DON
DPR0("LD L,C")
#endif
      break;
    case 0x6a:		/* LD L,D */
      tstates+=4;
      L=D;
#ifdef DON
DPR0("LD L,D")
#endif
      break;
    case 0x6b:		/* LD L,E */
      tstates+=4;
      L=E;
#ifdef DON
DPR0("LD L,E")
#endif
      break;
    case 0x6c:		/* LD L,H */
      tstates+=4;
      L=H;
#ifdef DON
DPR0("LD L,H")
#endif
      break;
    case 0x6d:		/* LD L,L */
      tstates+=4;
      AccelMode = 5;
#ifdef DON
DPR0("LD L,L")
#endif
      break;
    case 0x6e:		/* LD L,(HL) */
      tstates+=7;
      L=readbyte(HL);
#ifdef DON
DPR0("LD L,(HL)")
#endif
      break;
    case 0x6f:		/* LD L,A */
      tstates+=4;
      L=A;
#ifdef DON
DPR0("LD L,A")
#endif
      break;
    case 0x70:		/* LD (HL),B */
      tstates+=7;
      writebyte(HL,B);
#ifdef DON
DPR0("LD (HL),B")
#endif
      break;
    case 0x71:		/* LD (HL),C */
      tstates+=7;
      writebyte(HL,C);
#ifdef DON
DPR0("LD (HL),C")
#endif
      break;
    case 0x72:		/* LD (HL),D */
      tstates+=7;
      writebyte(HL,D);
#ifdef DON
DPR0("LD (HL),D")
#endif
      break;
    case 0x73:		/* LD (HL),E */
      tstates+=7;
      writebyte(HL,E);
#ifdef DON
DPR0("LD (HL),E")
#endif
      break;
    case 0x74:		/* LD (HL),H */
      tstates+=7;
      writebyte(HL,H);
#ifdef DON
DPR0("LD (HL),H")
#endif
      break;
    case 0x75:		/* LD (HL),L */
      tstates+=7;
      writebyte(HL,L);
#ifdef DON
DPR0("LD (HL),L")
#endif
      break;
    case 0x76:		/* HALT */
      tstates+=4;
      z80.halted=1;
#ifdef DON
DPR0("HALT")
#endif
      break;
    case 0x77:		/* LD (HL),A */
      tstates+=7;
      writebyte(HL,A);
#ifdef DON
DPR0("LD (HL),A")
#endif
      break;
    case 0x78:		/* LD A,B */
      tstates+=4;
      A=B;
#ifdef DON
DPR0("LD A,B")
#endif
      break;
    case 0x79:		/* LD A,C */
      tstates+=4;
      A=C;
#ifdef DON
DPR0("LD A,C")
#endif
      break;
    case 0x7a:		/* LD A,D */
      tstates+=4;
      A=D;
#ifdef DON
DPR0("LD A,D")
#endif
      break;
    case 0x7b:		/* LD A,E */
      tstates+=4;
      A=E;
#ifdef DON
DPR0("LD A,E")
#endif
      break;
    case 0x7c:		/* LD A,H */
      tstates+=4;
      A=H;
#ifdef DON
DPR0("LD A,H")
#endif
      break;
    case 0x7d:		/* LD A,L */
      tstates+=4;
      A=L;
#ifdef DON
DPR0("LD A,L")
#endif
      break;
    case 0x7e:		/* LD A,(HL) */
      tstates+=7;
      A=readbyte(HL);
#ifdef DON
DPR0("LD A,(HL)")
#endif
      break;
    case 0x7f:		/* LD A,A */
      tstates+=4;
      AccelMode = 6;
#ifdef DON
DPR0("LD A,A")
#endif
      break;
    case 0x80:		/* ADD A,B */
      tstates+=4;
      ADD(B);
#ifdef DON
DPR0("ADD A,B")
#endif
      break;
    case 0x81:		/* ADD A,C */
      tstates+=4;
      ADD(C);
#ifdef DON
DPR0("ADD A,C")
#endif
      break;
    case 0x82:		/* ADD A,D */
      tstates+=4;
      ADD(D);
#ifdef DON
DPR0("ADD A,D")
#endif
      break;
    case 0x83:		/* ADD A,E */
      tstates+=4;
      ADD(E);
#ifdef DON
DPR0("ADD A,E")
#endif
      break;
    case 0x84:		/* ADD A,H */
      tstates+=4;
      ADD(H);
#ifdef DON
DPR0("ADD A,H")
#endif
      break;
    case 0x85:		/* ADD A,L */
      tstates+=4;
      ADD(L);
#ifdef DON
DPR0("ADD A,L")
#endif
      break;
    case 0x86:		/* ADD A,(HL) */
      tstates+=7;
	bytetemp=readbyte(HL);
	ADD(bytetemp);
#ifdef DON
DPR0("ADD A,(HL)")
#endif
      break;
    case 0x87:		/* ADD A,A */
      tstates+=4;
      ADD(A);
#ifdef DON
DPR0("ADD A,A")
#endif
      break;
    case 0x88:		/* ADC A,B */
      tstates+=4;
      ADC(B);
#ifdef DON
DPR0("ADC A,B")
#endif
      break;
    case 0x89:		/* ADC A,C */
      tstates+=4;
      ADC(C);
#ifdef DON
DPR0("ADC A,C")
#endif
      break;
    case 0x8a:		/* ADC A,D */
      tstates+=4;
      ADC(D);
#ifdef DON
DPR0("ADC A,D")
#endif
      break;
    case 0x8b:		/* ADC A,E */
      tstates+=4;
      ADC(E);
#ifdef DON
DPR0("ADC A,E")
#endif
      break;
    case 0x8c:		/* ADC A,H */
      tstates+=4;
      ADC(H);
#ifdef DON
DPR0("ADC A,H")
#endif
      break;
    case 0x8d:		/* ADC A,L */
      tstates+=4;
      ADC(L);
#ifdef DON
DPR0("ADC A,L")
#endif
      break;
    case 0x8e:		/* ADC A,(HL) */
      tstates+=7;
	bytetemp=readbyte(HL);
	ADC(bytetemp);
#ifdef DON
DPR0("ADC A,(HL)")
#endif
      break;
    case 0x8f:		/* ADC A,A */
      tstates+=4;
      ADC(A);
#ifdef DON
DPR0("ADC A,A")
#endif
      break;
    case 0x90:		/* SUB A,B */
      tstates+=4;
      SUB(B);
#ifdef DON
DPR0("SUB A,B")
#endif
      break;
    case 0x91:		/* SUB A,C */
      tstates+=4;
      SUB(C);
#ifdef DON
DPR0("SUB A,C")
#endif
      break;
    case 0x92:		/* SUB A,D */
      tstates+=4;
      SUB(D);
#ifdef DON
DPR0("SUB A,D")
#endif
      break;
    case 0x93:		/* SUB A,E */
      tstates+=4;
      SUB(E);
#ifdef DON
DPR0("SUB A,E")
#endif
      break;
    case 0x94:		/* SUB A,H */
      tstates+=4;
      SUB(H);
#ifdef DON
DPR0("SUB A,H")
#endif
      break;
    case 0x95:		/* SUB A,L */
      tstates+=4;
      SUB(L);
#ifdef DON
DPR0("SUB A,L")
#endif
      break;
    case 0x96:		/* SUB A,(HL) */
      tstates+=7;
	bytetemp=readbyte(HL);
	SUB(bytetemp);
#ifdef DON
DPR0("SUB A,(HL)")
#endif
      break;
    case 0x97:		/* SUB A,A */
      tstates+=4;
      SUB(A);
#ifdef DON
DPR0("SUB A,A")
#endif
      break;
    case 0x98:		/* SBC A,B */
      tstates+=4;
      SBC(B);
#ifdef DON
DPR0("SBC A,B")
#endif
      break;
    case 0x99:		/* SBC A,C */
      tstates+=4;
      SBC(C);
#ifdef DON
DPR0("SBC A,C")
#endif
      break;
    case 0x9a:		/* SBC A,D */
      tstates+=4;
      SBC(D);
#ifdef DON
DPR0("SBC A,D")
#endif
      break;
    case 0x9b:		/* SBC A,E */
      tstates+=4;
      SBC(E);
#ifdef DON
DPR0("SBC A,E")
#endif
      break;
    case 0x9c:		/* SBC A,H */
      tstates+=4;
      SBC(H);
#ifdef DON
DPR0("SBC A,H")
#endif
      break;
    case 0x9d:		/* SBC A,L */
      tstates+=4;
      SBC(L);
#ifdef DON
DPR0("SBC A,L")
#endif
      break;
    case 0x9e:		/* SBC A,(HL) */
      tstates+=7;
	bytetemp=readbyte(HL);
	SBC(bytetemp);
#ifdef DON
DPR0("SBC A,(HL)")
#endif
      break;
    case 0x9f:		/* SBC A,A */
      tstates+=4;
      SBC(A);
#ifdef DON
DPR0("SBC A,A")
#endif
      break;
    case 0xa0:		/* AND A,B */
      tstates+=4;
      AND(B);
#ifdef DON
DPR0("AND A,B")
#endif
      break;
    case 0xa1:		/* AND A,C */
      tstates+=4;
      AND(C);
#ifdef DON
DPR0("AND A,C")
#endif
      break;
    case 0xa2:		/* AND A,D */
      tstates+=4;
      AND(D);
#ifdef DON
DPR0("AND A,D")
#endif
      break;
    case 0xa3:		/* AND A,E */
      tstates+=4;
      AND(E);
#ifdef DON
DPR0("AND A,E")
#endif
      break;
    case 0xa4:		/* AND A,H */
      tstates+=4;
      AND(H);
#ifdef DON
DPR0("AND A,H")
#endif
      break;
    case 0xa5:		/* AND A,L */
      tstates+=4;
      AND(L);
#ifdef DON
DPR0("AND A,L")
#endif
      break;
    case 0xa6:		/* AND A,(HL) */
      tstates+=7;
    if(!Accel('&',A,HL)) {
      bytetemp=readbyte(HL);
      AND(bytetemp); 
    }
#ifdef DON
DPR0("AND A,(HL)")
#endif
      break;
    case 0xa7:		/* AND A,A */
      tstates+=4;
      AND(A);
#ifdef DON
DPR0("AND A,A")
#endif
      break;
    case 0xa8:		/* XOR A,B */
      tstates+=4; 
      XOR(B);
#ifdef DON
DPR0("XOR A,B")
#endif
      break;
    case 0xa9:		/* XOR A,C */
      tstates+=4;
      XOR(C);
#ifdef DON
DPR0("XOR A,C")
#endif
      break;
    case 0xaa:		/* XOR A,D */
      tstates+=4;
      XOR(D);
#ifdef DON
DPR0("XOR A,D")
#endif
      break;
    case 0xab:		/* XOR A,E */
      tstates+=4;
      XOR(E);
#ifdef DON
DPR0("XOR A,E")
#endif
      break;
    case 0xac:		/* XOR A,H */
      tstates+=4;
      XOR(H);
#ifdef DON
DPR0("XOR A,H")
#endif
      break;
    case 0xad:		/* XOR A,L */
      tstates+=4;
      XOR(L);
#ifdef DON
DPR0("XOR A,L")
#endif
      break;
    case 0xae:		/* XOR A,(HL) */
      tstates+=7;
    if(!Accel('^',A,HL)) {
      bytetemp=readbyte(HL);
      XOR(bytetemp);
    }
#ifdef DON
DPR0("XOR A,(HL)")
#endif
      break;
    case 0xaf:		/* XOR A,A */
      tstates+=4;
      XOR(A);
#ifdef DON
DPR0("XOR A,A")
#endif
      break;
    case 0xb0:		/* OR A,B */
      tstates+=4;
      OR(B);
#ifdef DON
DPR0("OR A,B")
#endif
      break;
    case 0xb1:		/* OR A,C */
      tstates+=4;
      OR(C);
#ifdef DON
DPR0("OR A,C")
#endif
      break;
    case 0xb2:		/* OR A,D */
      tstates+=4;
      OR(D);
#ifdef DON
DPR0("OR A,D")
#endif
      break;
    case 0xb3:		/* OR A,E */
      tstates+=4;
      OR(E);
#ifdef DON
DPR0("OR A,E")
#endif
      break;
    case 0xb4:		/* OR A,H */
      tstates+=4;
      OR(H);
#ifdef DON
DPR0("OR A,H")
#endif
      break;
    case 0xb5:		/* OR A,L */
      tstates+=4;
      OR(L);
#ifdef DON
DPR0("OR A,L")
#endif
      break;
    case 0xb6:		/* OR A,(HL) */
      tstates+=7;
    if(!Accel('|',A,HL)) {
      bytetemp=readbyte(HL);
      OR(bytetemp);
    }
#ifdef DON
DPR0("OR A,(HL)")
#endif
      break;
    case 0xb7:		/* OR A,A */
      tstates+=4;
      OR(A);
#ifdef DON
DPR0("OR A,A")
#endif
      break;
    case 0xb8:		/* CP B */
      tstates+=4;
      CP(B);
#ifdef DON
DPR0("CP B")
#endif
      break;
    case 0xb9:		/* CP C */
      tstates+=4;
      CP(C);
#ifdef DON
DPR0("CP C")
#endif
      break;
    case 0xba:		/* CP D */
      tstates+=4;
      CP(D);
#ifdef DON
DPR0("CP D")
#endif
      break;
    case 0xbb:		/* CP E */
      tstates+=4;
      CP(E);
#ifdef DON
DPR0("CP E")
#endif
      break;
    case 0xbc:		/* CP H */
      tstates+=4;
      CP(H);
#ifdef DON
DPR0("CP H")
#endif
      break;
    case 0xbd:		/* CP L */
      tstates+=4;
      CP(L);
#ifdef DON
DPR0("CP L")
#endif
      break;
    case 0xbe:		/* CP (HL) */
      tstates+=7;
	bytetemp=readbyte(HL);
	CP(bytetemp);
#ifdef DON
DPR0("CP (HL)")
#endif
      break;
    case 0xbf:		/* CP A */
      tstates+=4;
      CP(A);
#ifdef DON
DPR0("CP A")
#endif
      break;
    case 0xc0:		/* RET NZ */
      tstates+=5;
      if( ! ( F & FLAG_Z ) ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET NZ")
#endif
      break;
    case 0xc1:		/* POP BC */
      tstates+=10;
      POP16(C,B);
#ifdef DON
DPR0("POP BC")
#endif
      break;
    case 0xc2:		/* JP NZ,nnnn */
      tstates+=10;
      if ( ! ( F & FLAG_Z ) ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP NZ,",PC)
#endif
      break;
    case 0xc3:		/* JP nnnn */
      tstates+=10;
      JP();
#ifdef DON
DPR3("JP",PC)
#endif
      break;
    case 0xc4:		/* CALL NZ,nnnn */
      tstates+=10;
      if ( ! (F & FLAG_Z ) ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL NZ,",PC)
#endif
      break;
    case 0xc5:		/* PUSH BC */
      tstates+=11;
      PUSH16(C,B);
#ifdef DON
DPR0("PUSH BC")
#endif
      break;
    case 0xc6:		/* ADD A,nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	ADD(bytetemp);
#ifdef DON
DPR1("ADD A,",readbyte(PC-1,0))
#endif
      break;
    case 0xc7:		/* RST 00 */
      tstates+=11;
      RST(0x00);
#ifdef DON
DPR0("RST 00")
#endif
      break;
    case 0xc8:		/* RET Z */
      tstates+=5;
      if( F & FLAG_Z ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET Z")
#endif
      break;
    case 0xc9:		/* RET */
      tstates+=10;
      RET();
#ifdef DON
DPR0("RET")
#endif
      break;
    case 0xca:		/* JP Z,nnnn */
      tstates+=10;
      if ( F & FLAG_Z ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP Z,",PC)
#endif
      break;
    case 0xcb:		/* CBxx opcodes */
      {
	BYTE opcode2=readbyte(PC++);
	R++;
#ifdef HAVE_ENOUGH_MEMORY
	switch(opcode2) {
#include "z_cb.hpp"
	}
#else			/* #ifdef HAVE_ENOUGH_MEMORY */
	z80_cbxx(opcode2);
#endif			/* #ifdef HAVE_ENOUGH_MEMORY */
      }
      break;
    case 0xcc:		/* CALL Z,nnnn */
      tstates+=10;
      if ( F & FLAG_Z ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL Z,",PC)
#endif
      break;
    case 0xcd:		/* CALL nnnn */
      tstates+=17;
      CALL();
#ifdef DON
DPR3("CALL",PC)
#endif
      break;
    case 0xce:		/* ADC A,nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	ADC(bytetemp);
#ifdef DON
DPR1("ADC A,",readbyte(PC-1,0))
#endif
      break;
    case 0xcf:		/* RST 8 */
      tstates+=11;
      if(!Bios(C))   /* Shaos */
          RST(0x08);
#ifdef DON
DPR0("RST 8")
#endif
      break;
    case 0xd0:		/* RET NC */
      tstates+=5;
      if( ! ( F & FLAG_C ) ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET NC")
#endif
      break;
    case 0xd1:		/* POP DE */
      tstates+=10;
      POP16(E,D);
#ifdef DON
DPR0("POP DE")
#endif
      break;
    case 0xd2:		/* JP NC,nnnn */
      tstates+=10;
      if ( ! ( F & FLAG_C ) ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP NC,",PC)
#endif
      break;
    case 0xd3:		/* OUT (nn),A */
      tstates+=11;
      writeport( readbyte(PC++) + ( A << 8 ) , A);
#ifdef DON
DPR5("OUT (#%2.2X),A",readbyte(PC-1,0))
#endif
      break;
    case 0xd4:		/* CALL NC,nnnn */
      tstates+=10;
      if ( ! (F & FLAG_C ) ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL NC,",PC)
#endif
      break;
    case 0xd5:		/* PUSH DE */
      tstates+=11;
      PUSH16(E,D);
#ifdef DON
DPR0("PUSH DE")
#endif
      break;
    case 0xd6:		/* SUB nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	SUB(bytetemp);
#ifdef DON
DPR1("SUB",readbyte(PC-1,0))
#endif
      break;
    case 0xd7:		/* RST 10 */
      tstates+=11;
      if(!Estex(C))   /* Shaos */
          RST(0x10); 
#ifdef DON
DPR0("RST 10")
#endif
      break;
    case 0xd8:		/* RET C */
      tstates+=5;
      if( F & FLAG_C ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET C")
#endif
      break;
    case 0xd9:		/* EXX */
      tstates+=4;
      {
	wordtemp=BC; BC=BC_; BC_=wordtemp;
	wordtemp=DE; DE=DE_; DE_=wordtemp;
	wordtemp=HL; HL=HL_; HL_=wordtemp;
      }
#ifdef DON
DPR0("EXX")
#endif
      break;
    case 0xda:		/* JP C,nnnn */
      tstates+=10;
      if ( F & FLAG_C ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP C,",PC)
#endif
      break;
    case 0xdb:		/* IN A,(nn) */
      tstates+=11;
      A=readport( readbyte(PC++) + ( A << 8 ) );
#ifdef DON
DPR5("IN A,(#%2.2X)",readbyte(PC-1,0))
#endif
      break;
    case 0xdc:		/* CALL C,nnnn */
      tstates+=10;
      if ( F & FLAG_C ) { tstates+=10; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL C,",PC)
#endif
      break;
    case 0xdd:		/* DDxx opcodes */
      {
	BYTE opcode2=readbyte(PC++);
	R++;
#ifdef HAVE_ENOUGH_MEMORY
	switch(opcode2) {
#define REGISTER  IX
#define REGISTERL IXL
#define REGISTERH IXH
#include "z_ddfd.hpp"
#undef REGISTERH
#undef REGISTERL
#undef REGISTER
	}
#else			/* #ifdef HAVE_ENOUGH_MEMORY */
	z80_ddxx(opcode2);
#endif			/* #ifdef HAVE_ENOUGH_MEMORY */
      }
      break;
    case 0xde:		/* SBC A,nn */
      tstates+=4;
	bytetemp=readbyte(PC++);
	SBC(bytetemp);
#ifdef DON
DPR1("SBC A,",readbyte(PC-1,0))
#endif
      break;
    case 0xdf:		/* RST 18 */
      tstates+=11;
      RST(0x18);
#ifdef DON
DPR0("RST 18")
#endif
      break;
    case 0xe0:		/* RET PO */
      tstates+=5;
      if( ! ( F & FLAG_P ) ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET PO")
#endif
      break;
    case 0xe1:		/* POP HL */
      tstates+=10;
      POP16(L,H);
#ifdef DON
DPR0("POP HL")
#endif
      break;
    case 0xe2:		/* JP PO,nnnn */
      tstates+=10;
      if ( ! ( F & FLAG_P ) ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP PO,",PC)
#endif
      break;
    case 0xe3:		/* EX (SP),HL */
      tstates+=19;
      {
	BYTE bytetempl=readbyte(SP), bytetemph=readbyte(SP+1);
	writebyte(SP,L); writebyte(SP+1,H);
	L=bytetempl; H=bytetemph;
      }
#ifdef DON
DPR0("EX (SP),HL")
#endif
      break;
    case 0xe4:		/* CALL PO,nnnn */
      tstates+=10;
      if ( ! (F & FLAG_P ) ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL PO,",PC)
#endif
      break;
    case 0xe5:		/* PUSH HL */
      tstates+=11;
      PUSH16(L,H);
#ifdef DON
DPR0("PUSH HL")
#endif
      break;
    case 0xe6:		/* AND nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	AND(bytetemp);
#ifdef DON
DPR1("AND",readbyte(PC-1,0))
#endif
      break;
    case 0xe7:		/* RST 20 */
      tstates+=11;
      RST(0x20);
#ifdef DON
DPR0("RST 20")
#endif
      break;
    case 0xe8:		/* RET PE */
      tstates+=10;
      if( F & FLAG_P ) { tstates+=7; RET(); }
#ifdef DON
DPR0("RET PE")
#endif
      break;
    case 0xe9:		/* JP HL */
      tstates+=4;
      PC=HL;		/* NB: NOT INDIRECT! */
#ifdef DON
DPR3("JP HL",PC)
#endif
      break;
    case 0xea:		/* JP PE,nnnn */
      tstates+=10;
      if ( F & FLAG_P ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP PE,",PC)
#endif
      break;
    case 0xeb:		/* EX DE,HL */
      tstates+=4;
      {
	wordtemp=DE; DE=HL; HL=wordtemp;
      }
#ifdef DON
DPR0("EX DE,HL")
#endif
      break;
    case 0xec:		/* CALL PE,nnnn */
      tstates+=10;
      if ( F & FLAG_P ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL PE,",PC)
#endif
      break;
    case 0xed:		/* EDxx opcodes */
      {
	BYTE opcode2=readbyte(PC++);
	R++;
#ifdef HAVE_ENOUGH_MEMORY
	switch(opcode2) {
#include "z_ed.hpp"
	}
#else			/* #ifdef HAVE_ENOUGH_MEMORY */
	z80_edxx(opcode2);
#endif			/* #ifdef HAVE_ENOUGH_MEMORY */
      }
      break;
    case 0xee:		/* XOR A,nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	XOR(bytetemp);
#ifdef DON
DPR1("XOR A,",readbyte(PC-1,0))
#endif
      break;
    case 0xef:		/* RST 28 */
      tstates+=11;
      RST(0x28);
#ifdef DON
DPR0("RST 28")
#endif
      break;
    case 0xf0:		/* RET P */
      tstates+=5;
      if( ! ( F & FLAG_S ) ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET P")
#endif
      break;
    case 0xf1:		/* POP AF */
      tstates+=10;
      POP16(F,A);
#ifdef DON
DPR0("POP AF")
#endif
      break;
    case 0xf2:		/* JP P,nnnn */
      tstates+=10;
      if ( ! ( F & FLAG_S ) ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP P,",PC)
#endif
      break;
    case 0xf3:		/* DI */
      tstates+=4;
      IFF1=IFF2=0;
#ifdef DON
DPR0("DI")
#endif
      break;
    case 0xf4:		/* CALL P,nnnn */
      tstates+=10;
      if ( ! (F & FLAG_S ) ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL P,",PC)
#endif
      break;
    case 0xf5:		/* PUSH AF */
      tstates+=11;
      PUSH16(F,A);
#ifdef DON
DPR0("PUSH AF")
#endif
      break;
    case 0xf6:		/* OR nn */
      tstates+=7;
	bytetemp=readbyte(PC++);
	OR(bytetemp);
#ifdef DON
DPR1("OR",readbyte(PC-1,0))
#endif
      break;
    case 0xf7:		/* RST 30 */
      tstates+=11;
      if(!Mouse(C))  /* Shaos */
          RST(0x30);
#ifdef DON
DPR0("RST 30")
#endif
      break;
    case 0xf8:		/* RET M */
      tstates+=5;
      if( F & FLAG_S ) { tstates+=6; RET(); }
#ifdef DON
DPR0("RET M")
#endif
      break;
    case 0xf9:		/* LD SP,HL */
      tstates+=6;
      SP=HL;
#ifdef DON
DPR0("LD SP,HL")
#endif
      break;
    case 0xfa:		/* JP M,nnnn */
      tstates+=10;
      if ( F & FLAG_S ) { JP(); }
      else PC+=2;
#ifdef DON
DPR3("JP M,",PC)
#endif
      break;
    case 0xfb:		/* EI */
      tstates+=4;
      IFF1=IFF2=1;
#ifdef DON
DPR0("EI")
#endif
      break;
    case 0xfc:		/* CALL M,nnnn */
      tstates+=10;
      if ( F & FLAG_S ) { tstates+=7; CALL(); }
      else PC+=2;
#ifdef DON
DPR3("CALL M,",PC)
#endif
      break;
    case 0xfd:		/* FDxx opcodes */
      {
	BYTE opcode2=readbyte(PC++);
	R++;
#ifdef HAVE_ENOUGH_MEMORY
	switch(opcode2) {
#define REGISTER  IY
#define REGISTERL IYL
#define REGISTERH IYH
#include "z_ddfd.hpp"
#undef REGISTERH
#undef REGISTERL
#undef REGISTER
	}
#else			/* #ifdef HAVE_ENOUGH_MEMORY */
	z80_fdxx(opcode2);
#endif			/* #ifdef HAVE_ENOUGH_MEMORY */
      }
      break;
    case 0xfe:		/* CP nn */
      tstates+=7;
      bytetemp=readbyte(PC++);
      CP(bytetemp);
#ifdef DON
DPR1("CP",readbyte(PC-1,0))
#endif
      break;
    case 0xff:		/* RST 38 */
      tstates+=11;
      RST(0x38);
#ifdef DON
DPR0("RST 38")
#endif
      break;

    }			/* Matches switch(opcode) { */

#ifdef DON
    if(flag_db) fprintf(fdebug,"\n");
#endif

  }		

}

#ifndef HAVE_ENOUGH_MEMORY

static void z80_cbxx(BYTE opcode2)
{
  switch(opcode2) {
#include "z_cb.hpp"
  }
}

static void z80_ddxx(BYTE opcode2)
{
  switch(opcode2) {
#define REGISTER  IX
#define REGISTERL IXL
#define REGISTERH IXH
#include "z_ddfd.hpp"
#undef REGISTERH
#undef REGISTERL
#undef REGISTER
  }
}

static void z80_edxx(BYTE opcode2)
{
  switch(opcode2) {
#include "z_ed.hpp"
  }
}

static void z80_fdxx(BYTE opcode2)
{
  switch(opcode2) {
#define REGISTER  IY
#define REGISTERL IYL
#define REGISTERH IYH
#include "z_ddfd.hpp"
#undef REGISTERH
#undef REGISTERL
#undef REGISTER
  }
}

static void z80_ddfdcbxx(BYTE opcode3, WORD tempaddr)
{
  switch(opcode3) {
#include "z_ddfdcb.hpp"
  }
}

#endif			/* #ifndef HAVE_ENOUGH_MEMORY */
