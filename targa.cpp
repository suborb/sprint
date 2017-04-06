// TARGA.CPP - GRAFIKA. PERVOE ZNAKOMSTVO
// 29.10.2001 Chabarchine Alexandre

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "targa.h"
#define TARGABUF 1024

TargaFile::TargaFile(char* name)
{
    file = fopen(name,"rb");
    if(file!=NULL)
    {
       fread(&Hdr,1,sizeof(Hdr),file);
      #if 1
       printf("Hdr.DataType = %i\n",Hdr.DataType);
       printf("Hdr.Width = %i\n",Hdr.Width);
       printf("Hdr.Height = %i\n",Hdr.Height);
       printf("Hdr.DataBits = %i\n",Hdr.DataBits);
       printf("Hdr.ImType = %i\n",Hdr.ImType);
      #endif
       if(Hdr.DataType==2&&Hdr.DataBits==24)
       {
          BufSize = Hdr.Width*Hdr.Height;
          Buffer = new TargaRGB[BufSize];
          if(Buffer!=NULL)
          {
             fseek(file,Hdr.TextSize,SEEK_CUR);
             fread(Buffer,BufSize,sizeof(TargaRGB),file);
          }
          else printf("Out of memory!\n");
       }
       else printf("Bad type of <%s>\n",name);
       fclose(file);
       file = NULL;
    }
    else printf("File <%s> error\n",name);
}

TargaFile::TargaFile(char* name, int width, int height, char* comment)
{
    file = fopen(name,"wb");
    BufSize = TARGABUF;
    Buffer = new TargaRGB[BufSize];
    pos = 0;
    
    memset(&Hdr,0,sizeof(Hdr));
    Hdr.DataType = 2;
    Hdr.Width = width;
    Hdr.Height = height;
    Hdr.DataBits = 24;
    Hdr.ImType = 32;
    
    if(comment!=NULL) Hdr.TextSize = strlen(comment)+1;
    if(file!=NULL)
    {
       fwrite(&Hdr,1,sizeof(Hdr),file);
       if(Hdr.TextSize>0) fwrite(comment,1,Hdr.TextSize,file);
    }   
}

TargaFile::~TargaFile()
{
    if(file!=NULL)
    {
       if(pos>0) Flush();
       fclose(file);
    }
    if(Buffer!=NULL) delete Buffer;
}

void TargaFile::PutPixel(TargaRGB color)
{
    Buffer[pos].Red   = color.Blue;
    Buffer[pos].Green = color.Green;
    Buffer[pos].Blue  = color.Red;
    if(++pos>=BufSize) Flush();
}

void TargaFile::Flush()
{
    if(file!=NULL) fwrite(Buffer,pos,sizeof(TargaRGB),file);
    pos = 0;
}

TargaRGB TargaFile::GetPixel(int i,int j)
{
    int k;
    TargaRGB rgb;
    k = i + j*Hdr.Width;
    if(k<BufSize)
    {
       rgb.Red   = Buffer[k].Blue;
       rgb.Green = Buffer[k].Green;
       rgb.Blue  = Buffer[k].Red;
    }
    return rgb;
}
