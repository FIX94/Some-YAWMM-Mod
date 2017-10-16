#include <stdio.h>
#include <ogcsys.h>

#include "nand.h"
#include "sys.h"
#include "wpad.h"
#include "video.h"


void Restart(void)
{
	Con_Clear ();
	printf("\n    Restarting Wii...");
	fflush(stdout);

	/* Disable NAND emulator */
	Nand_Disable();

	/* Load system menu */
	Sys_LoadMenu();
}

void Restart_Wait(void)
{
	printf("\n");

	printf("    Press any button to restart...");
	fflush(stdout);

	/* Wait for button */
	Wpad_WaitButtons();

	/* Restart */
	Restart();
}
 
