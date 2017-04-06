/*  loadbmp.hpp - Load 256-color BMP (Feb 20, 2002)

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

int LoadBmp(UniGraf* ug,char *str,int dx,int dy)
{
 int i,j,k,c;
 FILE *f = UniFopen(str,"rb");
 if(f==NULL) return 0;
 fseek(f,0x436,SEEK_SET);
 k=dx;
 if(dx&3) k=(dx&0xFFFC)+4;
 for(j=dy-1;j>=0;j--){
 for(i=0;i<k;i++){
   c = fgetc(f);
   if(i<dx) ug->SetScreenPixel(i,j,c);
 }}
 return 1;
}

