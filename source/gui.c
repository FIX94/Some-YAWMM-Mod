#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>

#include "video.h"
#include "fat.h"
#include "menu.h"
#include "nand.h"
#include "globals.h"

/* Constants */
#define CONSOLE_XCOORD		70
#define CONSOLE_YCOORD		118
#define CONSOLE_WIDTH		502
#define CONSOLE_HEIGHT		300

bool file_exists(const char * filename)
{
	FILE * file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}


s32 __Gui_DrawPng(void *img, u32 x, u32 y)
{
	IMGCTX   ctx = NULL;
	PNGUPROP imgProp;

	s32 ret;

	fatDevice *fdev = &fdevList[0];
	ret = Fat_Mount(fdev);
	if (file_exists("sd:/wad/background.png")) ctx = PNGU_SelectImageFromDevice ("sd:/wad/background.png");
	
	if (ret < 0) 
	{
		fdev = &fdevList[2];
		Fat_Mount(fdev);
		if (file_exists("usb2:/wad/background.png")) ctx = PNGU_SelectImageFromDevice ("usb2:/wad/background.png");
	}
	
	if(!ctx)
	{
	/* Select PNG data */
	ctx = PNGU_SelectImageFromBuffer(img);
	if (!ctx) {
		ret = -1;
		goto out;
	}
	}
	/* Get image properties */
	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK) {
		ret = -1;
		goto out;
	}

	/* Draw image */
	Video_DrawPng(ctx, imgProp, x, y);

	/* Success */
	ret = 0;

out:
	/* Free memory */
	if (ctx)
		PNGU_ReleaseImageContext(ctx);

	return ret;
}


void Gui_InitConsole(void)
{
	/* Initialize console */
	Con_Init(CONSOLE_XCOORD, CONSOLE_YCOORD, CONSOLE_WIDTH, CONSOLE_HEIGHT);
}

void Gui_DrawBackground(void)
{
	extern char bgData[];
	
	/* Draw background */
	__Gui_DrawPng(bgData, 0, 0);
} 
