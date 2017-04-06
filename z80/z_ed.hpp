/* z80_ed.c: z80 EDxx opcodes
   Copyright (c) 1999 Philip Kendall

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

   E-mail: pak21@cam.ac.uk
   Postal address: 15 Crescent Road, Wokingham, Berks, RG40 2DB, England

*/

case 0x40:	/* IN B,(C) */
tstates+=12;
IN(B,BC);
#ifdef DON
DPR0("IN B,(C)")
#endif
break;

case 0x41:	/* OUT (C),B */
tstates+=12;
writeport(BC,B);
#ifdef DON
DPR0("OUT (C),B")
#endif
break;

case 0x42:	/* SBC HL,BC */
tstates+=15;
SBC16(BC);
#ifdef DON
DPR0("SBC HL,BC")
#endif
break;

case 0x43:	/* LD (nnnn),BC */
tstates+=20;
#ifdef DON
DPR4("LD (#%4.4X),BC",readbyte(PC+1),readbyte(PC))
#endif
LD16_NNRR(C,B);
break;

case 0x44:	
case 0x4c:
case 0x54:
case 0x5c:
case 0x64:
case 0x6c:
case 0x74:
case 0x7c:	/* NEG */
tstates+=8;
{
BYTE bytetemp=A;
A=0;
SUB(bytetemp);
}
#ifdef DON
DPR0("NEG")
#endif
break;

case 0x45:
case 0x4d:
case 0x55:
case 0x5d:
case 0x65:
case 0x6d:
case 0x75:
case 0x7d:      /* RETN */
tstates+=14;
IFF1=IFF2;
RET();
#ifdef DON
DPR0("RETN")
#endif
break;

case 0x46:
case 0x4e:
case 0x66:
case 0x6e:	/* IM 0 */
tstates+=8;
IM=0;
#ifdef DON
DPR0("IM 0")
#endif
break;

case 0x47:	/* LD I,A */
tstates+=9;
I=A;
#ifdef DON
DPR0("LD I,A")
#endif
break;

case 0x48:	/* IN C,(C) */
tstates+=12;
IN(C,BC);
#ifdef DON
DPR0("IN C,(C)")
#endif
break;

case 0x49:	/* OUT (C),C */
tstates+=12;
writeport(BC,C);
#ifdef DON
DPR0("OUT (C),C")
#endif
break;

case 0x4a:	/* ADC HL,BC */
tstates+=15;
ADC16(BC);
#ifdef DON
DPR0("ADC HL,BC")
#endif
break;

case 0x4b:	/* LD BC,(nnnn) */
tstates+=20;
#ifdef DON
DPR4("LD BC,(#%4.4X)",readbyte(PC+1),readbyte(PC))
#endif
LD16_RRNN(C,B);
break;

case 0x4f:	/* LD R,A */
tstates+=9;
R=R7=A;
#ifdef DON
DPR0("LD R,A")
#endif
break;

case 0x50:	/* IN D,(C) */
tstates+=12;
IN(D,BC);
#ifdef DON
DPR0("IN D,(C)")
#endif
break;

case 0x51:	/* OUT (C),D */
tstates+=12;
writeport(BC,D);
#ifdef DON
DPR0("OUT (C),D")
#endif
break;

case 0x52:	/* SBC HL,DE */
tstates+=15;
SBC16(DE);
#ifdef DON
DPR0("SBC HL,DE")
#endif
break;

case 0x53:	/* LD (nnnn),DE */
tstates+=20;
#ifdef DON
DPR4("LD (#%4.4X),DE",readbyte(PC+1),readbyte(PC))
#endif
LD16_NNRR(E,D);
break;

case 0x56:
case 0x76:	/* IM 1 */
tstates+=8;
IM=1;
#ifdef DON
DPR0("IM 1")
#endif
break;

case 0x57:	/* LD A,I */
tstates+=9;
A=I;
F = ( F & FLAG_C ) | sz53_table[A] | ( IFF2 ? FLAG_V : 0 );
#ifdef DON
DPR0("LD A,I")
#endif
break;

case 0x58:	/* IN E,(C) */
tstates+=12;
IN(E,BC);
#ifdef DON
DPR0("IN E,(C)")
#endif
break;

case 0x59:	/* OUT (C),E */
tstates+=12;
writeport(BC,E);
#ifdef DON
DPR0("OUT (C),E")
#endif
break;

case 0x5a:	/* ADC HL,DE */
tstates+=15;
ADC16(DE);
#ifdef DON
DPR0("ADC HL,DE")
#endif
break;

case 0x5b:	/* LD DE,(nnnn) */
tstates+=20;
#ifdef DON
DPR4("LD DE,(#%4.4X)",readbyte(PC+1),readbyte(PC))
#endif
LD16_RRNN(E,D);
break;

case 0x5e:
case 0x7e:	/* IM 2 */
tstates+=8;
IM=2;
#ifdef DON
DPR0("IM 2")
#endif
break;

case 0x5f:	/* LD A,R */
tstates+=9;
A=(R&0x7f) | (R7&0x80);
F = ( F & FLAG_C ) | sz53_table[A] | ( IFF2 ? FLAG_V : 0 );
#ifdef DON
DPR0("LD A,R")
#endif
break;

case 0x60:	/* IN H,(C) */
tstates+=12;
IN(H,BC);
#ifdef DON
DPR0("IN H,(C)")
#endif
break;

case 0x61:	/* OUT (C),H */
tstates+=12;
writeport(BC,H);
#ifdef DON
DPR0("OUT (C),H")
#endif
break;

case 0x62:	/* SBC HL,HL */
tstates+=15;
SBC16(HL);
#ifdef DON
DPR0("SBC HL,HL")
#endif
break;

case 0x63:	/* LD (nnnn),HL */
tstates+=20;
#ifdef DON
DPR4("LD (#%4.4X),HL",readbyte(PC+1),readbyte(PC))
#endif
LD16_NNRR(L,H);
break;

case 0x67:	/* RRD */
tstates+=18;
{
BYTE bytetemp=readbyte(HL);
writebyte(HL,  ( A << 4 ) | ( bytetemp >> 4 ) );
A = ( A & 0xf0 ) | ( bytetemp & 0x0f );
F = ( F & FLAG_C ) | sz53p_table[A];
}
#ifdef DON
DPR0("RRD")
#endif
break;

case 0x68:	/* IN L,(C) */
tstates+=12;
IN(L,BC);
#ifdef DON
DPR0("IN L,(C)")
#endif
break;

case 0x69:	/* OUT (C),L */
tstates+=12;
writeport(BC,L);
#ifdef DON
DPR0("OUT (C),L")
#endif
break;

case 0x6a:	/* ADC HL,HL */
tstates+=15;
ADC16(HL);
#ifdef DON
DPR0("ADC HL,HL")
#endif
break;

case 0x6b:	/* LD HL,(nnnn) */
tstates+=20;
#ifdef DON
DPR4("LD HL,(#%4.4X)",readbyte(PC+1),readbyte(PC))
#endif
LD16_RRNN(L,H);
break;

case 0x6f:	/* RLD */
tstates+=18;
{
BYTE bytetemp=readbyte(HL);
writebyte(HL, (bytetemp << 4 ) | ( A & 0x0f ) );
A = ( A & 0xf0 ) | ( bytetemp >> 4 );
F = ( F & FLAG_C ) | sz53p_table[A];
}
#ifdef DON
DPR0("RLD")
#endif
break;

case 0x70:	/* IN F,(C) */
tstates+=12;
{
BYTE bytetemp;
IN(bytetemp,BC);
}
#ifdef DON
DPR0("IN F,(C)")
#endif
break;

case 0x71:	/* OUT (C),0 */
tstates+=12;
writeport(BC,0);
#ifdef DON
DPR0("OUT (C),0")
#endif
break;

case 0x72:	/* SBC HL,SP */
tstates+=15;
SBC16(SP);
#ifdef DON
DPR0("SBC HL,SP")
#endif
break;

case 0x73:	/* LD (nnnn),SP */
tstates+=20;
#ifdef DON
DPR4("LD (#%4.4X),SP",readbyte(PC+1),readbyte(PC))
#endif
LD16_NNRR(SPL,SPH);
break;

case 0x78:	/* IN A,(C) */
tstates+=12;
IN(A,BC);
#ifdef DON
DPR0("IN A,(C)")
#endif
break;

case 0x79:	/* OUT (C),A */
tstates+=12;
writeport(BC,A);
#ifdef DON
DPR0("OUT (C),A")
#endif
break;

case 0x7a:	/* ADC HL,SP */
tstates+=15;
ADC16(SP);
#ifdef DON
DPR0("ADC HL,SP")
#endif
break;

case 0x7b:	/* LD SP,(nnnn) */
tstates+=20;
#ifdef DON
DPR4("LD SP,(#%4.4X)",readbyte(PC+1),readbyte(PC))
#endif
LD16_RRNN(SPL,SPH);
break;

case 0xa0:	/* LDI */
tstates+=16;
{
BYTE bytetemp=readbyte(HL);
BC--;
writebyte(DE,bytetemp);
DE++; HL++;
bytetemp += A;
F = ( F & ( FLAG_C | FLAG_Z | FLAG_S ) ) | ( BC ? FLAG_V : 0 ) |
    ( bytetemp & FLAG_3 ) | ( (bytetemp & 0x02) ? FLAG_5 : 0 );
}
#ifdef DON
DPR0("LDI")
#endif
break;

case 0xa1:	/* CPI */
tstates+=16;
{
  BYTE value=readbyte(HL),bytetemp=A-value,
    lookup = ( (A & 0x08) >> 3 ) | ( ( (value) & 0x08 ) >> 2 ) |
    ( (bytetemp & 0x08) >> 1 );
  HL++; BC--;
  F = ( F & FLAG_C ) | ( BC ? ( FLAG_V | FLAG_N ) : FLAG_N ) |
    halfcarry_sub_table[lookup] | ( bytetemp ? 0 : FLAG_Z ) |
    ( bytetemp & FLAG_S );
  if(F & FLAG_H) bytetemp--;
  F |= ( bytetemp & FLAG_3 ) | ( (bytetemp&0x02) ? FLAG_5 : 0 );
}
#ifdef DON
DPR0("CPI")
#endif
break;

case 0xa2:	/* INI */
tstates+=16;
{
  WORD initemp=readport(BC);
  writebyte(HL,initemp);
  B--; HL++;
  F = (initemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
}
#ifdef DON
DPR0("INI")
#endif
break;

case 0xa3:	/* OUTI */
tstates+=16;
{
  WORD outitemp=readbyte(HL);
  B--;	HL++;	/* This does happen first, despite what the specs say */
  writeport(BC,outitemp);
  F = (outitemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
}
#ifdef DON
DPR0("OUTI")
#endif
break;

case 0xa8:	/* LDD */
tstates+=16;
{
  BYTE bytetemp=readbyte(HL);
  BC--;
  writebyte(DE,bytetemp);
  DE--; HL--;
  bytetemp += A;
  F = ( F & ( FLAG_C | FLAG_Z | FLAG_S ) ) | ( BC ? FLAG_V : 0 ) |
    ( bytetemp & FLAG_3 ) | ( (bytetemp & 0x02) ? FLAG_5 : 0 );
}
#ifdef DON
DPR0("LDD")
#endif
break;

case 0xa9:	/* CPD */
tstates+=16;
{
  BYTE value=readbyte(HL),bytetemp=A-value,
    lookup = ( (A & 0x08) >> 3 ) | ( ( (value) & 0x08 ) >> 2 ) |
    ( (bytetemp & 0x08) >> 1 );
  HL--; BC--;
  F = ( F & FLAG_C ) | ( BC ? ( FLAG_V | FLAG_N ) : FLAG_N ) |
    halfcarry_sub_table[lookup] | ( bytetemp ? 0 : FLAG_Z ) |
    ( bytetemp & FLAG_S );
  if(F & FLAG_H) bytetemp--;
  F |= ( bytetemp & FLAG_3 ) | ( (bytetemp&0x02) ? FLAG_5 : 0 );
}
#ifdef DON
DPR0("CPD")
#endif
break;

case 0xaa:	/* IND */
tstates+=16;
{
  WORD initemp=readport(BC);
  writebyte(HL,initemp);
  B--; HL--;
  F = (initemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
}
#ifdef DON
DPR0("IND")
#endif
break;

case 0xab:	/* OUTD */
tstates+=16;
{
  WORD outitemp=readbyte(HL);
  B--; HL--;	/* This does happen first, despite what the specs say */
  writeport(BC,outitemp);
  F = (outitemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
}
#ifdef DON
DPR0("OUTD")
#endif
break;

case 0xb0:	/* LDIR */
tstates+=16;
{
BYTE bytetemp=readbyte(HL);
writebyte(DE,bytetemp);
HL++; DE++; BC--;
bytetemp += A;
F = ( F & ( FLAG_C | FLAG_Z | FLAG_S ) ) | ( BC ? FLAG_V : 0 ) |
    ( bytetemp & FLAG_3 ) | ( (bytetemp & 0x02) ? FLAG_5 : 0 );
if(BC) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("LDIR")
#endif
break;

case 0xb1:	/* CPIR */
tstates+=16;
{
  BYTE value=readbyte(HL),bytetemp=A-value,
    lookup = ( (A & 0x08) >> 3 ) | ( ( (value) & 0x08 ) >> 2 ) |
    ( (bytetemp & 0x08) >> 1 );
  HL++; BC--;
  F = ( F & FLAG_C ) | ( BC ? ( FLAG_V | FLAG_N ) : FLAG_N ) |
    halfcarry_sub_table[lookup] | ( bytetemp ? 0 : FLAG_Z ) |
    ( bytetemp & FLAG_S );
  if(F & FLAG_H) bytetemp--;
  F |= ( bytetemp & FLAG_3 ) | ( (bytetemp&0x02) ? FLAG_5 : 0 );
  if( ( F & ( FLAG_V | FLAG_Z ) ) == FLAG_V ) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("CPIR")
#endif
break;
  
case 0xb2:	/* INIR */
tstates+=16;
{
  WORD initemp=readport(BC);
  writebyte(HL,initemp);
  B--; HL++;
  F = (initemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
  if(B) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("INIR")
#endif
break;

case 0xb3:	/* OTIR */
tstates+=16;
{
  WORD outitemp=readbyte(HL);
  B--;	HL++;	/* This does happen first, despite what the specs say */
  writeport(BC,outitemp);
  F = (outitemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
  if(B) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("OTIR")
#endif
break;

case 0xb8:	/* LDDR */
tstates+=17;
{
BYTE bytetemp=readbyte(HL);
writebyte(DE,bytetemp);
HL--; DE--; BC--;
bytetemp += A;
F = ( F & ( FLAG_C | FLAG_Z | FLAG_S ) ) | ( BC ? FLAG_V : 0 ) |
    ( bytetemp & FLAG_3 ) | ( (bytetemp & 0x02) ? FLAG_5 : 0 );
if(BC) { tstates+=4; PC-=2; }
}
#ifdef DON
DPR0("LDDR")
#endif
break;

case 0xb9:	/* CPDR */
tstates+=16;
{
  BYTE value=readbyte(HL),bytetemp=A-value,
    lookup = ( (A & 0x08) >> 3 ) | ( ( (value) & 0x08 ) >> 2 ) |
    ( (bytetemp & 0x08) >> 1 );
  HL--; BC--;
  F = ( F & FLAG_C ) | ( BC ? ( FLAG_V | FLAG_N ) : FLAG_N ) |
    halfcarry_sub_table[lookup] | ( bytetemp ? 0 : FLAG_Z ) |
    ( bytetemp & FLAG_S );
  if(F & FLAG_H) bytetemp--;
  F |= ( bytetemp & FLAG_3 ) | ( (bytetemp&0x02) ? FLAG_5 : 0 );
  if( ( F & ( FLAG_V | FLAG_Z ) ) == FLAG_V ) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("CPDR")
#endif
break;

case 0xba:	/* INDR */
tstates+=16;
{
  WORD initemp=readport(BC);
  writebyte(HL,initemp);
  B--; HL--;
  F = (initemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
  if(B) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("INDR")
#endif
break;

case 0xbb:	/* OTDR */
tstates+=16;
{
  WORD outitemp=readbyte(HL);
  B--; HL--;	/* This does happen first, despite what the specs say */
  writeport(BC,outitemp);
  F = (outitemp & 0x80 ? FLAG_N : 0 ) | sz53_table[B];
  /* C,H and P/V flags not implemented */
  if(B) { tstates+=5; PC-=2; }
}
#ifdef DON
DPR0("OTDR")
#endif
break;

default:	/* All other opcodes are NOPD */
tstates+=8;
#ifdef DON
DPR0("NOPD?")
#endif
break;
