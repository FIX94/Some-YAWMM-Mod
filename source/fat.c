#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <fat.h>
#include <sys/dir.h>
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>

//#include <smb.h>

#include "fat.h"
#include "usbstorage.h"

typedef struct
{
	/* Device prefix */
	char* prefix;

	/* Device name */
	char* name;

	/* Device available */
	bool isMounted;

	/* Device interface */
	const DISC_INTERFACE* interface;
} FatDevice;

static FatDevice DeviceList[] =
{
	{ "sd",		"Wii SD Slot",					false,	&__io_wiisd },
	{ "usb",	"USB Mass Storage Device",		false,	&__io_usbstorage },
	{ "usb2",	"USB 2.0 Mass Storage Device",	false,	&__io_wiiums },
	{ "gcsda",	"SD Gecko (Slot A)",			false,	&__io_gcsda },
	{ "gcsdb",	"SD Gecko (Slot B)",			false,	&__io_gcsdb },
};

static u32 gNumDevices = 0;
FatDevice* gDevices[(sizeof(DeviceList) / sizeof(FatDevice))];

void FatMount()
{
	FatUnmount();
	
	s32 i;
	for (i = 0; i < (sizeof(DeviceList) / sizeof(FatDevice)); i++)
	{
		gDevices[gNumDevices] = &DeviceList[i];
		
		s32 ret = gDevices[gNumDevices]->interface->startup();

		if (!ret)
			continue;

		ret = fatMountSimple(gDevices[gNumDevices]->prefix, gDevices[gNumDevices]->interface);

		if (!ret)
			continue;

		gDevices[gNumDevices]->isMounted = true;
		gNumDevices++;
	}
}

void FatUnmount()
{
	s32 i;
	for (i = 0; i < FatGetDeviceCount(); i++)
	{
		fatUnmount(gDevices[i]->prefix);
		gDevices[i]->interface->shutdown();
		gDevices[i]->isMounted = false;
	}

	gNumDevices = 0;
}

char* FatGetDeviceName(u8 index)
{
	if (index >= FatGetDeviceCount())
		return NULL;

	if (gDevices[index]->isMounted)
		return gDevices[index]->name;

	return NULL;
}


char* FatGetDevicePrefix(u8 index)
{
	if (index >= FatGetDeviceCount())
		return NULL;

	if (gDevices[index]->isMounted)
		return gDevices[index]->prefix;

	return NULL;
}

s32 FatGetDeviceCount()
{
	return gNumDevices;
}
