// HEX.HPP - Alexander Shabarshin  17.11.2000

#include <string.h>

#ifndef _HEX_HPP
#define _HEX_HPP

int hexx(char c)
{  int i = 0;
   switch(c)
   {  case '0': i=0;break;
      case '1': i=1;break;
      case '2': i=2;break;
      case '3': i=3;break;
      case '4': i=4;break;
      case '5': i=5;break;
      case '6': i=6;break;
      case '7': i=7;break;
      case '8': i=8;break;
      case '9': i=9;break;
      case 'a': i=10;break;
      case 'b': i=11;break;
      case 'c': i=12;break;
      case 'd': i=13;break;
      case 'e': i=14;break;
      case 'f': i=15;break;
      case 'A': i=10;break;
      case 'B': i=11;break;
      case 'C': i=12;break;
      case 'D': i=13;break;
      case 'E': i=14;break;
      case 'F': i=15;break;
   }
   return i;
}

int hex2i(char *s)
{  int len = strlen(s);
   int n = 0;
   if(len<1||len>4) return -1;
   if(len==1) n=hexx(s[0]);
   if(len==2) n=(hexx(s[0])<<4)|hexx(s[1]);
   if(len==3) n=(hexx(s[0])<<8)|(hexx(s[1])<<4)|hexx(s[2]);
   if(len==4) n=(hexx(s[0])<<12)|(hexx(s[1])<<8)|(hexx(s[2])<<4)|hexx(s[3]);
   return n;
}

#endif
