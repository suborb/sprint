// WILDCMP.HPP - Shabarshin A.A.  15.12.1999

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define MAIN

int WildCmp0(char *str, char *wld);

int WildCmp(char *str, char *wld)
{
  int i,k;
  k = 0;
  for(i=0;i<(int)strlen(wld);i++)
  {
    if(wld[i]=='*') k++;
  }
  if((int)strlen(str)<i-k) return 0;
  return WildCmp0(str,wld);
}

int WildN = 0;

int WildCmp0(char *str, char *wld)
{
  char *ps,*pw;
//  printf("\n%u)\t'%s'\n\t'%s'\n",++WildN,str,wld);
  if(*str==0&&*wld==0) return 1;
  ps = &str[1];
  pw = &wld[1];
  if(*str==*wld) return WildCmp0(ps,pw);
  if(*wld=='?') return WildCmp0(ps,pw);
  if(*wld=='*')
  {
     if(*pw==0) return 1;
     while(*pw!=*ps)
     {
        laba:
        if(*ps++==0) return 0;
     }
     if(WildCmp0(ps,pw)) return 1;
     else goto laba;
  }
  return 0;
}


#ifdef MAIN

int main(int argc,char **argv)
{
  int i;
  char str[100],wld[100];
  if(argc<3) return 0;
  strcpy(str,argv[1]);
  strcpy(wld,argv[2]);
  if(WildCmp(str,wld))
     printf("\n\n<%s> and <%s> equel\n",str,wld);
  else
     printf("\n\n<%s> and <%s> NOT equel\n",str,wld);
  return 1;
}


#endif
