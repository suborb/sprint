// KBD_RSDN.HPP for WATCOM by Shaos

#include <i86.h>
#include <dos.h>
#include <conio.h>

#define TRUE 1
#define FALSE 0

char keyPressed[128];  
// Array specifying for each key
// whether it's up or down

// keyBoardOldHandler keeps track of old interrupt function
void (__interrupt __far *keyboardOldHandler)();

// keyboardISR will be our new keyboard interrupt service
// routine
void (__interrupt __far keyboardISR)();

char haveKey=FALSE;   // a flag to tell us if a new
                      // keyboard event happened

void keyboardCapture()
{
    keyboardOldHandler=_dos_getvect(9);
    _dos_setvect(9,keyboardISR);
    for(int i=0;i<128;i++) keyPressed[i]=0;
}

void keyboardRelease()
{
    _dos_setvect(9,keyboardOldHandler);
}

void __interrupt __far keyboardISR()
{
    int i,j;

    i=inp(0x60);          // read in scan code
    j=inp(0x61);          // read in command status

    outp(0x61,j|0x80);    // reset the keyboard controller
    outp(0x61,j);
    // if bit 7 (high bit) is set, the key was released
    if(i&0x80) keyPressed[i&0x7f]=0;
    // if bit 7 (high bit) is 0, the key just went down
    else keyPressed[i&0x7f]=1;
    outp(0x20,0x20);
    haveKey=TRUE;
}

