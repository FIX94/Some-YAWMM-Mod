#ifndef _WKB_H_
#define _WKB_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <malloc.h>
#include <ogcsys.h> // u8, u16, etc...

#include <wiikeyboard/keyboard.h>
#include <wiikeyboard/usbkeyboard.h>

/* Prototypes */
s32  WkbInit(void);
u32  WkbWaitKey (void);
//void Wpad_Disconnect(void);
//u32  Wpad_GetButtons(void);

#endif
