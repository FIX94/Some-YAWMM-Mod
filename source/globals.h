#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Constants
#define CIOS_VERSION		249
#define ENTRIES_PER_PAGE	14
#define MAX_FILE_PATH_LEN	1024
#define MAX_DIR_LEVELS		10
#define WAD_DIRECTORY		"/"
#define WAD_ROOT_DIRECTORY  "/wad"

#define MAX_PASSWORD_LENGTH  	10
#define MAX_FAT_DEVICE_LENGTH  	10
#define MAX_NAND_DEVICE_LENGTH  10

#define WM_CONFIG_FILE_PATH ":/wad/wm_config.txt"
#define WM_BACKGROUND_PATH ":/wad/background.png"

// These are indices into the fatDevice fdevList
#define FAT_DEVICE_INDEX_WII_SD  0
#define FAT_DEVICE_INDXE_USB     1
#define FAT_DEVICE_INDEX_USB2    2
#define FAT_DEVICE_INDEX_GC_SDA  3
#define FAT_DEVICE_INDEX_GC_SDB  4
#define FAT_DEVICE_INDEX_INVALID -1

// These are the indices into the nandDevice ndevList
#define NAND_DEVICE_INDEX_DISABLE   0
#define NAND_DEVICE_INDEX_SD        1
#define NAND_DEVICE_INDEX_USB	    2
#define NAND_DEVICE_INDEX_INVALID   -1

#define CIOS_VERSION_INVALID        -1

// For the WiiLight
#define WII_LIGHT_OFF                0
#define WII_LIGHT_ON                 1

typedef struct 
{
	char password[MAX_PASSWORD_LENGTH];
	char startupPath [256];
	int cIOSVersion;
	int fatDeviceIndex;
	int nandDeviceIndex;
	const char *smbuser;
	const char *smbpassword;
	const char *share;
	const char *ip;
} CONFIG;


extern CONFIG gConfig;
extern nandDevice ndevList[];
extern fatDevice fdevList[];


#endif
