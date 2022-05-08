#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <wiilight.h>
#include <wiidrc/wiidrc.h>
#include <unistd.h>

#include "sys.h"
#include "fat.h"
#include "nand.h"
#include "restart.h"
#include "title.h"
#include "usbstorage.h"
#include "utils.h"
#include "video.h"
#include "wad.h"
#include "wpad.h"
#include <ogc/pad.h>
#include "globals.h"
#include "iospatch.h"

/* FAT device list  */
//static fatDevice fdevList[] = {
fatDevice fdevList[] = {
	{ "sd",		"Wii SD Slot",					&__io_wiisd },
	{ "usb",	"USB Mass Storage Device",		&__io_usbstorage },
	{ "usb2",	"USB 2.0 Mass Storage Device",	&__io_wiiums },
	{ "gcsda",	"SD Gecko (Slot A)",			&__io_gcsda },
	{ "gcsdb",	"SD Gecko (Slot B)",			&__io_gcsdb },
	//{ "smb",	"SMB share",					NULL },
};

/* NAND device list */
//static nandDevice ndevList[] = {
nandDevice ndevList[] = {
	{ "Disable",				0,	0x00,	0x00 },
	{ "SD/SDHC Card",			1,	0xF0,	0xF1 },
	{ "USB 2.0 Mass Storage Device",	2,	0xF2,	0xF3 },
};

/* FAT device */
static fatDevice  *fdev = NULL;
static nandDevice *ndev = NULL;

// wiiNinja: Define a buffer holding the previous path names as user
// traverses the directory tree. Max of 10 levels is define at this point
static u8 gDirLevel = 0;
static char gDirList [MAX_DIR_LEVELS][MAX_FILE_PATH_LEN];
static s32  gSeleted[MAX_DIR_LEVELS];
static s32  gStart[MAX_DIR_LEVELS];

/* Macros */
#define NB_FAT_DEVICES		(sizeof(fdevList) / sizeof(fatDevice))
#define NB_NAND_DEVICES		(sizeof(ndevList) / sizeof(nandDevice))

// Local prototypes: wiiNinja
void WaitPrompt (char *prompt);
int PushCurrentDir(char *dirStr, int Selected, int Start);
char *PopCurrentDir(int *Selected, int *Start);
bool IsListFull (void);
char *PeekCurrentDir (void);
u32 WaitButtons(void);
u32 Pad_GetButtons(void);
void WiiLightControl (int state);

int __Menu_IsGreater(const void *p1, const void *p2)
{
	u32 n1 = *(u32 *)p1;
	u32 n2 = *(u32 *)p2;

	/* Equal */
	if (n1 == n2)
		return 0;

	return (n1 > n2) ? 1 : -1;
}


int __Menu_EntryCmp(const void *p1, const void *p2)
{
	fatFile *f1 = (fatFile *)p1;
	fatFile *f2 = (fatFile *)p2;

	/* Compare entries */ // wiiNinja: Include directory
    if ((f1->isdir) && !(f2->isdir))
        return (-1);
    else if (!(f1->isdir) && (f2->isdir))
        return (1);
    else
        return strcasecmp(f1->filename, f2->filename);
}

static bool __FolderExists(const char *path)
{
	DIR *dir;
	dir = opendir(path);
	if(dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}

static size_t __GetFileSizeBytes(const char *path)
{
	FILE *f;
	size_t size = 0;

	f = fopen(path, "rb");
	if(!f) return 0;

	//Get file size
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fclose(f);

	return size;
}

char gFileName[MAX_FILE_PATH_LEN];
s32 __Menu_RetrieveList(char *inPath, fatFile **outbuf, u32 *outlen)
{
	fatFile     *buffer = NULL;
	DIR            *dir = NULL;
	struct dirent  *ent = NULL;

	//char dirpath[256], filename[768];
	u32  cnt;

	/* Generate dirpath */
	//sprintf(dirpath, "%s:" WAD_DIRECTORY, fdev->mount);

	/* Open directory */
	dir = opendir(inPath);
	if (!dir)
		return -1;

	/* Count entries */
	for (cnt = 0; ((ent = readdir(dir)) != NULL);) {
		cnt++;
	}

	if (cnt > 0) {
		/* Allocate memory */
		buffer = malloc(sizeof(fatFile) * cnt);
		if (!buffer) {
			closedir(dir);
			return -2;
		}

		/* Reset directory */
		rewinddir(dir);

		/* Get entries */
		for (cnt = 0; ((ent = readdir(dir)) != NULL);)
		{
			bool addFlag = false;
			bool isdir = false;
			size_t fsize = 0;

			snprintf(gFileName, MAX_FILE_PATH_LEN, "%s/%s", inPath, ent->d_name);
			if (__FolderExists(gFileName))  // wiiNinja
            {
				isdir = true;
                // Add only the item ".." which is the previous directory
                // AND if we're not at the root directory
                if ((strcmp (ent->d_name, "..") == 0) && (gDirLevel > 1))
                    addFlag = true;
                else if (strcmp (ent->d_name, ".") != 0)
                    addFlag = true;
            }
            else
			{
				if(strlen(ent->d_name)>4)
				{
					if(!strcasecmp(ent->d_name+strlen(ent->d_name)-4, ".wad"))
					{
						fsize = __GetFileSizeBytes(gFileName);
						addFlag = true;
					}
				}
			}

            if (addFlag == true)
            {
				fatFile *file = &buffer[cnt++];

				/* File name */
				strcpy(file->filename, ent->d_name);

				/* File stats */
				file->isdir = isdir;
				file->fsize = fsize;

			}
		}

		/* Sort list */
		qsort(buffer, cnt, sizeof(fatFile), __Menu_EntryCmp);
	}

	/* Close directory */
	closedir(dir);

	/* Set values */
	*outbuf = buffer;
	*outlen = cnt;

	return 0;
}


void Menu_SelectIOS(void)
{
	u8 *iosVersion = NULL;
	u32 iosCnt;
	u8 tmpVersion;

	u32 cnt;
	s32 ret, selected = 0;
	bool found = false;

	/* Get IOS versions */
	ret = Title_GetIOSVersions(&iosVersion, &iosCnt);
	if (ret < 0)
		return;

	/* Sort list */
	qsort(iosVersion, iosCnt, sizeof(u8), __Menu_IsGreater);

	if (gConfig.cIOSVersion < 0)
		tmpVersion = CIOS_VERSION;
	else
	{
		tmpVersion = (u8)gConfig.cIOSVersion;
		// For debugging only
		//printf ("User pre-selected cIOS: %i\n", tmpVersion);
		//WaitButtons();
	}

	/* Set default version */
	for (cnt = 0; cnt < iosCnt; cnt++) {
		u8 version = iosVersion[cnt];

		/* Custom IOS available */
		//if (version == CIOS_VERSION)
		if (version == tmpVersion)
		{
			selected = cnt;
			found = true;
			break;
		}

		/* Current IOS */
		if (version == IOS_GetVersion())
			selected = cnt;
	}

	/* Ask user for IOS version */
	if ((gConfig.cIOSVersion < 0) || (found == false))
	{
		for (;;)
		{
			/* Clear console */
			Con_Clear();

			printf("\t>> Select IOS version to use: < IOS%d >\n\n", iosVersion[selected]);

			printf("\t   Press LEFT/RIGHT to change IOS version.\n\n");

			printf("\t   Press A button to continue.\n");
			printf("\t   Press HOME button to restart.\n\n");

			u32 buttons = WaitButtons();

			/* LEFT/RIGHT buttons */
			if (buttons & WPAD_BUTTON_LEFT) {
				if ((--selected) <= -1)
					selected = (iosCnt - 1);
			}
			if (buttons & WPAD_BUTTON_RIGHT) {
				if ((++selected) >= iosCnt)
					selected = 0;
			}

			/* HOME button */
			if (buttons & WPAD_BUTTON_HOME)
				Restart();

			/* A button */
			if (buttons & WPAD_BUTTON_A)
				break;
		}
	}


	u8 version = iosVersion[selected];

	if (IOS_GetVersion() != version) {
		/* Shutdown subsystems */
		Wpad_Disconnect();
		//mload_close();

		/* Load IOS */

		if(!loadIOS(version)) Wpad_Init(), Menu_SelectIOS();

		/* Initialize subsystems */
		Wpad_Init();
	}
}

void Menu_FatDevice(void)
{
	int ret, selected = 0;

	/* Unmount FAT device */
	//if (fdev)
		//Fat_Unmount(fdev);
	//if (((fdevList[selected].mount[0] == 's') && (ndev->name[0] == 'S')))
		//selected++;
	static const u16 konamiCode[] = {
		WPAD_BUTTON_UP, WPAD_BUTTON_UP, WPAD_BUTTON_DOWN, WPAD_BUTTON_DOWN, WPAD_BUTTON_LEFT,
		WPAD_BUTTON_RIGHT, WPAD_BUTTON_LEFT, WPAD_BUTTON_RIGHT, WPAD_BUTTON_B, WPAD_BUTTON_A
	};

	int codePosition = 0;

	/* Select source device */
	if (gConfig.fatDeviceIndex < 0)
	{
		for (;;) {
			/* Clear console */
			Con_Clear();

			/* Selected device */
			fdev = &fdevList[selected];

			printf("\t>> Select source device: < %s >\n\n", fdev->name);

			printf("\t   Press LEFT/RIGHT to change the selected device.\n\n");

			printf("\t   Press A button to continue.\n");
			printf("\t   Press HOME button to restart.\n\n");

			u32 buttons = WaitButtons();

			if (buttons & (WPAD_BUTTON_UP | WPAD_BUTTON_DOWN | WPAD_BUTTON_RIGHT | WPAD_BUTTON_LEFT | WPAD_BUTTON_A | WPAD_BUTTON_B)) {
				if (buttons & konamiCode[codePosition])
					++codePosition;
				else
					codePosition = 0;
			}

			/* LEFT/RIGHT buttons */
			if (buttons & WPAD_BUTTON_LEFT) {
				if ((--selected) <= -1)
					selected = (NB_FAT_DEVICES - 1);
				if ((fdevList[selected].mount[0] == 's') && (ndev->name[0] == 'S'))
					selected--;
				if ((fdevList[selected].mount[0] == 'u' && fdevList[selected].mount[3] == '2') && (ndev->name[0] == 'U'))
					selected--;
				if ((selected) <= -1)
					selected = (NB_FAT_DEVICES - 1);
			}
			if (buttons & WPAD_BUTTON_RIGHT) {
				if ((++selected) >= NB_FAT_DEVICES)
					selected = 0;
				if ((fdevList[selected].mount[0] == 's') && (ndev->name[0] == 'S'))
					selected++;
				if ((fdevList[selected].mount[0] == 'u' && fdevList[selected].mount[3] == '2') && (ndev->name[0] == 'U'))
					selected++;
			}

			/* HOME button */
			if (buttons & WPAD_BUTTON_HOME)
				Restart();

			/* A button */
			if (buttons & WPAD_BUTTON_A) {
				if (codePosition == sizeof(konamiCode) / sizeof(konamiCode[0])) {
					extern bool skipRegionSafetyCheck;
					skipRegionSafetyCheck = true;
					printf("[+] Disabled SM region checks\n");
					sleep(2);
				}
				break;
			}
		}
	}
	else
	{
		sleep(5);
		fdev = &fdevList[gConfig.fatDeviceIndex];
	}

	printf("[+] Mounting %s, please wait...", fdev->name );
	fflush(stdout);

	/* Mount FAT device */

	ret = Fat_Mount(fdev);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	} else
		printf(" OK!\n");

	return;

err:

    if(gConfig.fatDeviceIndex >= 0) gConfig.fatDeviceIndex = -1;
	WiiLightControl (WII_LIGHT_OFF);
	printf("\n");
	printf("    Press any button to continue...\n");

	WaitButtons();

	/* Prompt menu again */
	Menu_FatDevice();
}

void Menu_NandDevice(void)
{
	int ret, selected = 0;

	/* Disable NAND emulator */
	if (ndev) {
		Nand_Unmount(ndev);
		Nand_Disable();
	}

	/* Select source device */
	if (gConfig.nandDeviceIndex < 0)
	{
		for (;;) {
			/* Clear console */
			Con_Clear();

			/* Selected device */
			ndev = &ndevList[selected];

			printf("\t>> Select NAND emulator device: < %s >\n\n", ndev->name);

			printf("\t   Press LEFT/RIGHT to change the selected device.\n\n");

			printf("\t   Press A button to continue.\n");
			printf("\t   Press HOME button to restart.\n\n");

			u32 buttons = WaitButtons();

			/* LEFT/RIGHT buttons */
			if (buttons & WPAD_BUTTON_LEFT) {
				if ((--selected) <= -1)
					selected = (NB_NAND_DEVICES - 1);
			}
			if (buttons & WPAD_BUTTON_RIGHT) {
				if ((++selected) >= NB_NAND_DEVICES)
					selected = 0;
			}

			/* HOME button */
			if (buttons & WPAD_BUTTON_HOME)
				Restart();

			/* A button */
			if (buttons & WPAD_BUTTON_A)
				break;
		}
	}
	else
	{
		ndev = &ndevList[gConfig.nandDeviceIndex];
	}

	/* No NAND device */
	if (!ndev->mode)
		return;

	printf("[+] Enabling NAND emulator...");
	fflush(stdout);

	/* Mount NAND device */
	ret = Nand_Mount(ndev);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	/* Enable NAND emulator */
	ret = Nand_Enable(ndev);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	} else
		printf(" OK!\n");

	return;

err:
	printf("\n");
	printf("    Press any button to continue...\n");

	WaitButtons();

	/* Prompt menu again */
	Menu_NandDevice();
}

char gTmpFilePath[MAX_FILE_PATH_LEN];
/* Install and/or Uninstall multiple WADs - Leathl */
int Menu_BatchProcessWads(fatFile *files, int fileCount, char *inFilePath, int installCnt, int uninstallCnt)
{
	int count;

	for (;;)
	{
		Con_Clear();

		if ((installCnt > 0) & (uninstallCnt == 0)) {
			printf("[+] %d file%s marked for installation.\n", installCnt, (installCnt == 1) ? "" : "s");
			printf("    Do you want to proceed?\n");
		}
		else if ((installCnt == 0) & (uninstallCnt > 0)) {
			printf("[+] %d file%s marked for uninstallation.\n", uninstallCnt, (uninstallCnt == 1) ? "" : "s");
			printf("    Do you want to proceed?\n");
		}
		else {
			printf("[+] %d file%s marked for installation and %d file%s for uninstallation.\n", installCnt, (installCnt == 1) ? "" : "s", uninstallCnt, (uninstallCnt == 1) ? "" : "s");
			printf("    Do you want to proceed?\n");
		}

		printf("\n\n    Press A to continue.\n");
		printf("    Press B to go back to the menu.\n\n");

		u32 buttons = WaitButtons();

		if (buttons & WPAD_BUTTON_A)
			break;

		if (buttons & WPAD_BUTTON_B)
			return 0;
	}

	WiiLightControl (WII_LIGHT_ON);
	int errors = 0;
	int success = 0;
	s32 ret;

	for (count = 0; count < fileCount; count++)
	{
		fatFile *thisFile = &files[count];

		if ((thisFile->install == 1) | (thisFile->install == 2)) {
			int mode = thisFile->install;
			Con_Clear();
			printf("[+] Opening \"%s\", please wait...\n\n", thisFile->filename);

			sprintf(gTmpFilePath, "%s/%s", inFilePath, thisFile->filename);

			FILE *fp = fopen(gTmpFilePath, "rb");
			if (!fp) {
				printf(" ERROR!\n");
				errors += 1;
				continue;
				}

			printf("[+] %s WAD, please wait...\n", (mode == 2) ? "Uninstalling" : "Installing");
			if (mode == 2) {
				ret = Wad_Uninstall(fp);
			}
			else {
				ret = Wad_Install(fp);
			}

			if (ret < 0) errors += 1;
			else success += 1;

			thisFile->installstate = ret;

			if (fp)
				fclose(fp);
		}
	}

	WiiLightControl (WII_LIGHT_OFF);

	printf("\n");
	printf("    %d titles succeeded and %d failed...\n", success, errors);

	if (errors > 0)
	{
		printf("\n    Some operations failed");
		printf("\n    Press A to list.\n");
		printf("    Press B skip.\n");

		u32 buttons = WaitButtons();

		if ((buttons & WPAD_BUTTON_A))
		{
			Con_Clear();

			int i=0;
			for (count = 0; count < fileCount; count++)
			{
				fatFile *thisFile = &files[count];

				if (thisFile->installstate <0)
				{
					char str[41];
					strncpy(str, thisFile->filename, 40); //Only 40 chars to fit the screen
					str[40]=0;
					i++;
					if(thisFile->installstate == -999) printf("    %s BRICK BLOCKED\n", str);
					else if(thisFile->installstate == -998) printf("    %s Skipped\n", str);
					else if(thisFile->installstate == -106) printf("    %s Not installed?\n", str);
					else if(thisFile->installstate == -1036 ) printf("    %s Needed IOS missing\n", str);
					else if(thisFile->installstate == -4100 ) printf("    %s No trucha bug?\n", str);
					else printf("    %s error %d\n", str, thisFile->installstate);
					if( i == 17 )
					{
						printf("\n    Press any button to continue\n");
						WaitButtons();
						i = 0;
					}
				}
			}
		}
	}
	printf("\n    Press any button to continue...\n");
	WaitButtons();

	return 1;
}

/* File Operations - Leathl */
int Menu_FileOperations(fatFile *file, char *inFilePath)
{
	f32 filesize = (file->fsize / MB_SIZE);

	for (;;)
	{
		Con_Clear();

		printf("[+] WAD Filename : %s\n",          file->filename);
		printf("    WAD Filesize : %.2f MB\n\n\n", filesize);


		printf("[+] Select action: < %s WAD >\n\n", "Delete"); //There's yet nothing else than delete

		printf("    Press LEFT/RIGHT to change selected action.\n\n");

		printf("    Press A to continue.\n");
		printf("    Press B to go back to the menu.\n\n");

		u32 buttons = WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return 0;
	}

	Con_Clear();

	printf("[+] Deleting \"%s\", please wait...\n", file->filename);

	sprintf(gTmpFilePath, "%s/%s", inFilePath, file->filename);
	int error = remove(gTmpFilePath);
	if (error != 0)
		printf("    ERROR!");
	else
		printf("    Successfully deleted!");

	printf("\n");
	printf("    Press any button to continue...\n");

	WaitButtons();

	return !error;
}

void Menu_WadManage(fatFile *file, char *inFilePath)
{
	FILE *fp  = NULL;

	//char filepath[128];
	f32  filesize;

	u32  mode = 0;

	/* File size in megabytes */
	filesize = (file->fsize / MB_SIZE);

	for (;;) {
		/* Clear console */
		Con_Clear();

		printf("[+] WAD Filename : %s\n",          file->filename);
		printf("    WAD Filesize : %.2f MB\n\n\n", filesize);


		printf("[+] Select action: < %s WAD >\n\n", (!mode) ? "Install" : "Uninstall");

		printf("    Press LEFT/RIGHT to change selected action.\n\n");

		printf("    Press A to continue.\n");
		printf("    Press B to go back to the menu.\n\n");

		u32 buttons = WaitButtons();

		/* LEFT/RIGHT buttons */
		if (buttons & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT))
			mode ^= 1;

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return;
	}

	/* Clear console */
	Con_Clear();

	printf("[+] Opening \"%s\", please wait...", file->filename);
	fflush(stdout);

	/* Generate filepath */
	// sprintf(filepath, "%s:" WAD_DIRECTORY "/%s", fdev->mount, file->filename);
	sprintf(gTmpFilePath, "%s/%s", inFilePath, file->filename); // wiiNinja

	/* Open WAD */
	fp = fopen(gTmpFilePath, "rb");
	if (!fp) {
		printf(" ERROR!\n");
		goto out;
	} else
		printf(" OK!\n\n");

	printf("[+] %s WAD, please wait...\n", (!mode) ? "Installing" : "Uninstalling");

	/* Do install/uninstall */
	WiiLightControl (WII_LIGHT_ON);
	if (!mode)
		Wad_Install(fp);
	else
		Wad_Uninstall(fp);
	WiiLightControl (WII_LIGHT_OFF);

out:
	/* Close file */
	if (fp)
		fclose(fp);

	printf("\n");
	printf("    Press any button to continue...\n");

	/* Wait for button */
	WaitButtons();
}

void Menu_WadList(void)
{
	char str [100];
	fatFile *fileList = NULL;
	u32      fileCnt;
	int ret, selected = 0, start = 0;
    char *tmpPath = malloc (MAX_FILE_PATH_LEN);
	int installCnt = 0;
	int uninstallCnt = 0;

	//fatFile *installFiles = malloc(sizeof(fatFile) * 50);
	//int installCount = 0;

    // wiiNinja: check for malloc error
    if (tmpPath == NULL)
    {
        ret = -997; // What am I gonna use here?
		printf(" ERROR! Out of memory (ret = %d)\n", ret);
        return;
    }

	printf("[+] Retrieving file list...");
	fflush(stdout);

	gDirLevel = 0;

	// push root dir as base folder
	sprintf(tmpPath, "%s:%s", fdev->mount, WAD_DIRECTORY);
	PushCurrentDir(tmpPath,0,0);
	// if user provides startup directory, try it out first
	if (strcmp (WAD_DIRECTORY, gConfig.startupPath) != 0)
	{
		// replace root dir with provided startup directory
		sprintf(tmpPath, "%s:%s", fdev->mount, gConfig.startupPath);
        // If the directory can be successfully opened, it must exists
        DIR *tmpDirPtr = opendir(tmpPath);
        if (tmpDirPtr)
        {
			closedir (tmpDirPtr);
			PushCurrentDir(tmpPath,0,0);
        }
		else // unable to open provided dir, stick with root dir
			sprintf(tmpPath, "%s:%s", fdev->mount, WAD_DIRECTORY);
	}

	/* Retrieve filelist */
getList:
    free (fileList);
    fileList = NULL;

	ret = __Menu_RetrieveList(tmpPath, &fileList, &fileCnt);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	/* No files */
	if (!fileCnt) {
		printf(" No files found!\n");
		goto err;
	}

	/* Set install-values to 0 - Leathl */
	int counter;
	for (counter = 0; counter < fileCnt; counter++) {
		fatFile *file = &fileList[counter];
		file->install = 0;
	}

	for (;;)
	{
		u32 cnt;
		s32 index;

		/* Clear console */
		Con_Clear();

		/** Print entries **/
		cnt = strlen(tmpPath);
		if(cnt>30)
			index = cnt-30;
		else
			index = 0;

		printf("[+] WAD files on [%s]:\n\n", tmpPath+index);

		/* Print entries */
		for (cnt = start; cnt < fileCnt; cnt++)
		{
			fatFile *file     = &fileList[cnt];
			f32      filesize = file->fsize / MB_SIZE;

			/* Entries per page limit */
			if ((cnt - start) >= ENTRIES_PER_PAGE)
				break;

			strncpy(str, file->filename, 40); //Only 40 chars to fit the screen
			str[40]=0;

			/* Print filename */
			//printf("\t%2s %s (%.2f MB)\n", (cnt == selected) ? ">>" : "  ", file->filename, filesize);
            if (file->isdir) // wiiNinja
				printf("\t%2s [%s]\n", (cnt == selected) ? ">>" : "  ", str);
            else
                printf("\t%2s%s%s (%.2f MB)\n", (cnt == selected) ? ">>" : "  ", (file->install == 1) ? "+" : ((file->install == 2) ? "-" : " "), str, filesize);

		}

		printf("\n");

		printf("[+] Press A to (un)install.");
		if(gDirLevel>1)
			printf(" Press B to go up-level DIR.\n");
		else
			printf(" Press B to select a device.\n");
		printf("    Use +/X and -/Y to (un)mark. Press 1/Z/ZR for delete menu.");

			/** Controls **/
		u32 buttons = WaitButtons();

		/* DPAD buttons */
		if (buttons & WPAD_BUTTON_UP) {
			selected--;

			if (selected <= -1)
				selected = (fileCnt - 1);
		}
		if (buttons & WPAD_BUTTON_LEFT) {
			selected = selected + ENTRIES_PER_PAGE;

			if (selected >= fileCnt)
				selected = 0;
		}
		if (buttons & WPAD_BUTTON_DOWN) {
			selected ++;

			if (selected >= fileCnt)
				selected = 0;
		}
			if (buttons & WPAD_BUTTON_RIGHT) {
				selected = selected - ENTRIES_PER_PAGE;

			if (selected <= -1)
				selected = (fileCnt - 1);
		}

		/* HOME button */
		if (buttons & WPAD_BUTTON_HOME)
			Restart();

		/* Plus Button - Leathl */
		if (buttons & WPAD_BUTTON_PLUS)
		{
			if(Wpad_TimeButton())
			{
			  installCnt = 0;
			  int i = 0;
			  while( i < fileCnt)
			  {
			  fatFile *file = &fileList[i];
			  if (((file->isdir) == false) & (file->install == 0)) {
				  file->install = 1;

				  installCnt += 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 1)) {
				  file->install = 0;

				  installCnt -= 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 2)) {
				  file->install = 1;

				  installCnt += 1;
				  uninstallCnt -= 1;
			  }
			  i++;
			  }

			}
			else
			{
			  fatFile *file = &fileList[selected];
			  if (((file->isdir) == false) & (file->install == 0)) {
				  file->install = 1;

				  installCnt += 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 1)) {
				  file->install = 0;

				  installCnt -= 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 2)) {
				  file->install = 1;

				  installCnt += 1;
				  uninstallCnt -= 1;
			  }
			  selected++;

			  if (selected >= fileCnt)
				selected = 0;
			}
		}

		/* Minus Button - Leathl */
		if (buttons & WPAD_BUTTON_MINUS)
		{
			if(Wpad_TimeButton())
			{
			  installCnt = 0;
			  int i = 0;
			  while( i < fileCnt)
			  {
			  fatFile *file = &fileList[i];
			  if (((file->isdir) == false) & (file->install == 0)) {
				file->install = 2;

				uninstallCnt += 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 1)) {
				file->install = 2;

				uninstallCnt += 1;
				installCnt -= 1;
			  }
			  else if (((file->isdir) == false) & (file->install == 2)) {
				file->install = 0;

				uninstallCnt -= 1;
			  }
			  i++;
			  }

			}
			else
			{
			fatFile *file = &fileList[selected];
			if (((file->isdir) == false) & (file->install == 0)) {
				file->install = 2;

				uninstallCnt += 1;
			}
			else if (((file->isdir) == false) & (file->install == 1)) {
				file->install = 2;

				uninstallCnt += 1;
				installCnt -= 1;
			}
			else if (((file->isdir) == false) & (file->install == 2)) {
				file->install = 0;

				uninstallCnt -= 1;
			}
			 selected++;

			  if (selected >= fileCnt)
				selected = 0;
			}
		}

		/* 1 Button - Leathl */
		if (buttons & WPAD_BUTTON_1)
		{
			fatFile *tmpFile = &fileList[selected];
			char *tmpCurPath = PeekCurrentDir ();
            if (tmpCurPath != NULL) {
				int res = Menu_FileOperations(tmpFile, tmpCurPath);
                if (res != 0)
					goto getList;
			}
		}


		/* A button */
		if (buttons & WPAD_BUTTON_A)
		{
				fatFile *tmpFile = &fileList[selected];
				char *tmpCurPath;
				if (tmpFile->isdir) // wiiNinja
				{
					if (strcmp (tmpFile->filename, "..") == 0)
					{
						selected = 0;
						start = 0;

						// Previous dir
						tmpCurPath = PopCurrentDir(&selected, &start);
						if (tmpCurPath != NULL)
							sprintf(tmpPath, "%s", tmpCurPath);

						installCnt = 0;
						uninstallCnt = 0;

						goto getList;
					}
					else if (IsListFull () == true)
					{
						WaitPrompt ("Maximum number of directory levels is reached.\n");
					}
					else
					{
						tmpCurPath = PeekCurrentDir ();
						if (tmpCurPath != NULL)
						{
							if(gDirLevel>1)
								sprintf(tmpPath, "%s/%s", tmpCurPath, tmpFile->filename);
							else
								sprintf(tmpPath, "%s%s", tmpCurPath, tmpFile->filename);
						}
						// wiiNinja: Need to PopCurrentDir
						PushCurrentDir (tmpPath, selected, start);
						selected = 0;
						start = 0;

						installCnt = 0;
						uninstallCnt = 0;

						goto getList;
					}
				}
				else
				{
					//If at least one WAD is marked, goto batch screen - Leathl
					if ((installCnt > 0) | (uninstallCnt > 0)) {
						char *thisCurPath = PeekCurrentDir ();
						if (thisCurPath != NULL) {
							int res = Menu_BatchProcessWads(fileList, fileCnt, thisCurPath, installCnt, uninstallCnt);

							if (res == 1) {
								int counter;
								for (counter = 0; counter < fileCnt; counter++) {
									fatFile *temp = &fileList[counter];
									temp->install = 0;
								}

								installCnt = 0;
								uninstallCnt = 0;
							}
						}
					}
					//else use standard wadmanage menu - Leathl
					else {
						tmpCurPath = PeekCurrentDir ();
						if (tmpCurPath != NULL)
							Menu_WadManage(tmpFile, tmpCurPath);
					}
				}
		}

		/* B button */
		if (buttons & WPAD_BUTTON_B)
		{
			if(gDirLevel<=1)
			{
				return;
			}

			char *tmpCurPath;
			selected = 0;
			start = 0;
			// Previous dir
			tmpCurPath = PopCurrentDir(&selected, &start);
			if (tmpCurPath != NULL)
				sprintf(tmpPath, "%s", tmpCurPath);
			goto getList;
			//return;
		}

		/** Scrolling **/
		/* List scrolling */
		index = (selected - start);

		if (index >= ENTRIES_PER_PAGE)
			start += index - (ENTRIES_PER_PAGE - 1);
		if (index <= -1)
			start += index;
	}

err:
	printf("\n");
	printf("    Press any button to continue...\n");

	free (tmpPath);

	/* Wait for button */
	WaitButtons();
}


void Menu_Loop(void)
{
	u8 iosVersion;
	if(AHBPROT_DISABLED)
		IOSPATCH_Apply();
	else
	{
		/* Select IOS menu */
		Menu_SelectIOS();
	}

	/* Retrieve IOS version */
	iosVersion = IOS_GetVersion();

	ndev = &ndevList[0];

	/* NAND device menu */
	if ((iosVersion == CIOS_VERSION || iosVersion == 250) && IOS_GetRevision() >13)
	{
		Menu_NandDevice();
	}
	for (;;) {
		/* FAT device menu */
		Menu_FatDevice();

		/* WAD list menu */
		Menu_WadList();
	}
}

// Start of wiiNinja's added routines

int PushCurrentDir (char *dirStr, int Selected, int Start)
{
    int retval = 0;

    // Store dirStr into the list and increment the gDirLevel
    // WARNING: Make sure dirStr is no larger than MAX_FILE_PATH_LEN
    if (gDirLevel < MAX_DIR_LEVELS)
    {
        strcpy (gDirList [gDirLevel], dirStr);
		gSeleted[gDirLevel]=Selected;
		gStart[gDirLevel]=Start;
        gDirLevel++;
        //if (gDirLevel >= MAX_DIR_LEVELS)
        //    gDirLevel = 0;
    }
    else
        retval = -1;

    return (retval);
}

char *PopCurrentDir(int *Selected, int *Start)
{
	if (gDirLevel > 1)
        gDirLevel--;
    else {
        gDirLevel = 0;
	}
	*Selected = gSeleted[gDirLevel];
	*Start = gStart[gDirLevel];
	return PeekCurrentDir();
}

bool IsListFull (void)
{
    if (gDirLevel < MAX_DIR_LEVELS)
        return (false);
    else
        return (true);
}

char *PeekCurrentDir (void)
{
    // Return the current path
    if (gDirLevel > 0)
        return (gDirList [gDirLevel-1]);
    else
        return (NULL);
}

void WaitPrompt (char *prompt)
{
	printf("\n%s", prompt);
	printf("    Press any button to continue...\n");

	/* Wait for button */
	WaitButtons();
}

u32 Pad_GetButtons(void)
{
	u32 buttons = 0, cnt;

	/* Scan pads */
	PAD_ScanPads();

	/* Get pressed buttons */
	//for (cnt = 0; cnt < MAX_WIIMOTES; cnt++)
	for (cnt = 0; cnt < 4; cnt++)
		buttons |= PAD_ButtonsDown(cnt);

	return buttons;
}

u32 WiiDRC_GetButtons(void)
{
	if(!WiiDRC_Inited() || !WiiDRC_Connected())
		return 0;

	/* Scan pads */
	WiiDRC_ScanPads();

	/* Get pressed buttons */
	return WiiDRC_ButtonsDown();
}

// Routine to wait for a button from either the Wiimote or a gamecube
// controller. The return value will mimic the WPAD buttons to minimize
// the amount of changes to the original code, that is expecting only
// Wiimote button presses. Note that the "HOME" button on the Wiimote
// is mapped to the "SELECT" button on the Gamecube Ctrl. (wiiNinja 5/15/2009)
u32 WaitButtons(void)
{
	u32 buttons = 0;
    u32 buttonsGC = 0;
	u32 buttonsDRC = 0;

	/* Wait for button pressing */
	while (!buttons && !buttonsGC && !buttonsDRC)
    {
        // Wii buttons
		buttons = Wpad_GetButtons();

        // GC buttons
        buttonsGC = Pad_GetButtons();

        // DRC buttons
        buttonsDRC = WiiDRC_GetButtons();

		VIDEO_WaitVSync();
	}

	if(buttons & WPAD_CLASSIC_BUTTON_A)
		buttons |= WPAD_BUTTON_A;
	else if(buttons & WPAD_CLASSIC_BUTTON_B)
		buttons |= WPAD_BUTTON_B;
	else if(buttons & WPAD_CLASSIC_BUTTON_LEFT)
		buttons |= WPAD_BUTTON_LEFT;
	else if(buttons & WPAD_CLASSIC_BUTTON_RIGHT)
		buttons |= WPAD_BUTTON_RIGHT;
	else if(buttons & WPAD_CLASSIC_BUTTON_DOWN)
		buttons |= WPAD_BUTTON_DOWN;
	else if(buttons & WPAD_CLASSIC_BUTTON_UP)
		buttons |= WPAD_BUTTON_UP;
	else if(buttons & WPAD_CLASSIC_BUTTON_HOME)
		buttons |= WPAD_BUTTON_HOME;
	else if(buttons & (WPAD_CLASSIC_BUTTON_X | WPAD_CLASSIC_BUTTON_PLUS))
		buttons |= WPAD_BUTTON_PLUS;
	else if(buttons & (WPAD_CLASSIC_BUTTON_Y | WPAD_CLASSIC_BUTTON_MINUS))
		buttons |= WPAD_BUTTON_MINUS;
	else if(buttons & WPAD_CLASSIC_BUTTON_ZR)
		buttons |= WPAD_BUTTON_1;

    if (buttonsGC)
    {
        if(buttonsGC & PAD_BUTTON_A)
            buttons |= WPAD_BUTTON_A;
        else if(buttonsGC & PAD_BUTTON_B)
            buttons |= WPAD_BUTTON_B;
        else if(buttonsGC & PAD_BUTTON_LEFT)
            buttons |= WPAD_BUTTON_LEFT;
        else if(buttonsGC & PAD_BUTTON_RIGHT)
            buttons |= WPAD_BUTTON_RIGHT;
        else if(buttonsGC & PAD_BUTTON_DOWN)
            buttons |= WPAD_BUTTON_DOWN;
        else if(buttonsGC & PAD_BUTTON_UP)
            buttons |= WPAD_BUTTON_UP;
        else if(buttonsGC & PAD_BUTTON_START)
            buttons |= WPAD_BUTTON_HOME;
        else if(buttonsGC & PAD_BUTTON_X)
            buttons |= WPAD_BUTTON_PLUS;
        else if(buttonsGC & PAD_BUTTON_Y)
            buttons |= WPAD_BUTTON_MINUS;
        else if(buttonsGC & PAD_TRIGGER_Z)
            buttons |= WPAD_BUTTON_1;
    }

    if (buttonsDRC)
    {
        if(buttonsDRC & WIIDRC_BUTTON_A)
            buttons |= WPAD_BUTTON_A;
        else if(buttonsDRC & WIIDRC_BUTTON_B)
            buttons |= WPAD_BUTTON_B;
        else if(buttonsDRC & WIIDRC_BUTTON_LEFT)
            buttons |= WPAD_BUTTON_LEFT;
        else if(buttonsDRC & WIIDRC_BUTTON_RIGHT)
            buttons |= WPAD_BUTTON_RIGHT;
        else if(buttonsDRC & WIIDRC_BUTTON_DOWN)
            buttons |= WPAD_BUTTON_DOWN;
        else if(buttonsDRC & WIIDRC_BUTTON_UP)
            buttons |= WPAD_BUTTON_UP;
        else if(buttonsDRC & WIIDRC_BUTTON_HOME)
            buttons |= WPAD_BUTTON_HOME;
        else if(buttonsDRC & (WIIDRC_BUTTON_X | WIIDRC_BUTTON_PLUS))
            buttons |= WPAD_BUTTON_PLUS;
        else if(buttonsDRC & (WIIDRC_BUTTON_Y | WIIDRC_BUTTON_MINUS))
            buttons |= WPAD_BUTTON_MINUS;
        else if(buttonsDRC & WIIDRC_BUTTON_ZR)
            buttons |= WPAD_BUTTON_1;
    }

	return buttons;
} // WaitButtons


void WiiLightControl (int state)
{
	switch (state)
	{
		case WII_LIGHT_ON:
			/* Turn on Wii Light */
			WIILIGHT_SetLevel(255);
			WIILIGHT_TurnOn();
			break;

		case WII_LIGHT_OFF:
		default:
			/* Turn off Wii Light */
			WIILIGHT_SetLevel(0);
			WIILIGHT_TurnOn();
			WIILIGHT_Toggle();
			break;
	}
} // WiiLightControl

