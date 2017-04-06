// TARGA.H - GRAFIKA. PERVOE ZNAKOMSTVO
// 29.10.2001 Chabarchine Alexandre

#ifndef __TARGA__
#define __TARGA__

#include <stdio.h>

#pragma pack(1)

struct TargaHeader
{
    char TextSize;
    char MapType;
    char DataType;
    short MapOrg;
    short MapLength;
    char CMapBits;
    short XOffset;
    short YOffset;
    short Width;
    short Height;
    char DataBits;
    char ImType;
};

struct TargaRGB
{
    unsigned char Red;
    unsigned char Green;
    unsigned char Blue;
    TargaRGB(short r=0,short g=0,short b=0){Red=r;Green=g;Blue=b;}
};

#pragma pack()

class TargaFile
{
 public:
    TargaFile(char* name); // Load
    TargaFile(char* name, int width, int height, char* comment=NULL); // Save
    ~TargaFile();
    void PutPixel(TargaRGB color);
    TargaRGB GetPixel(int i,int j);
    TargaHeader Hdr;
 private:
    TargaRGB *Buffer;
    int BufSize;
    int pos;
    FILE *file;
    void Flush();        
};

#endif
