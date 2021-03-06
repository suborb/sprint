/* z80.h: z80 emulation core
   Copyright (c) 1999-2000 Philip Kendall
   Modified by Alexander Shabarshin (shaos@mail.ru) in 2002

   $Id: z80.h,v 1.3 2000/12/08 15:13:58 pak Exp $

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

#ifndef FUSE_Z80_H
#define FUSE_Z80_H

/* Union allowing a register pair to be accessed as bytes or as a word */
typedef union {
#ifdef WORDS_BIGENDIAN
  struct { BYTE h,l; } b;
#else
  struct { BYTE l,h; } b;
#endif
  WORD w;
} regpair;

/* What's stored in the main processor */
typedef struct {
  regpair af,bc,de,hl;
  regpair af_,bc_,de_,hl_;
  regpair ix,iy;
  BYTE i,r,r7; /* r is the lower 7 bits of R; r7 is the high bit */
  regpair sp,pc;
  BYTE iff1,iff2,im;
  int halted;
} processor;

extern FILE *fdebug;

void z80_init(int debug = 0);
void z80_stop(int debug = 0);
void z80_reset(void);
void z80_interrupt(void);

void z80_do_opcodes(int debug=0);

extern processor z80;
extern BYTE halfcarry_add_table[];
extern BYTE halfcarry_sub_table[];
extern BYTE overflow_add_table[];
extern BYTE overflow_sub_table[];
extern BYTE sz53_table[];
extern BYTE sz53p_table[];
extern BYTE parity_table[];

#endif			/* #ifndef FUSE_Z80_H */
