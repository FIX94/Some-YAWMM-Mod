#include "wkb.h"

/*

s32 USBKeyboard_Open(const eventcallback cb);
void USBKeyboard_Close(void);

bool USBKeyboard_IsConnected(void);
s32 USBKeyboard_Scan(void);

s32 USBKeyboard_SetLed(const USBKeyboard_led led, bool on);
s32 USBKeyboard_ToggleLed(const USBKeyboard_led led);
*/

s32  WkbInit(void)
{
    s32 retval = 0;

    retval = USBKeyboard_Initialize();

    return (retval);

} // WkbInit

s32 WkbDeInit(void)
{
    s32 retval = 0;

    retval = USBKeyboard_Deinitialize();

    return (retval);

} // WkbDeInit

u32  WkbWaitKey (void)
{
    u32 retval = 0;

    // Stub
    return (retval);

} // WkbWaitKey

//void Wpad_Disconnect(void);
//u32  Wpad_GetButtons(void);


