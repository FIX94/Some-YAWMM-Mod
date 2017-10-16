
                     =YET ANOTHER WAD MANAGER MOD (YAWMM)=

==[ DISCLAIMER ]:==

  THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESSED NOR IMPLIED.
  NO ONE BUT YOURSELF IS RESPONSIBLE FOR ANY DAMAGE TO YOUR WII CONSOLE
  BECAUSE OF A IMPROPER USAGE OF THIS SOFTWARE.


==[ DESCRIPTION ]:==

  This is an application that allows you to (un)install WADs.

  It lists all the available WADs in a storage device
  so you can select which one to (un)install.


==[ SUPPORTED DEVICES ]:==

  * SDGecko.
  * Internal SD slot (with SDHC support).
  * USB device (1.1 and 2.0).


==[ USAGE ]:==

  * Copy the WAD(s) you wish to insall to your storage device.
  * Run the application with any method to load homebrew.

  * The following 3 options can be skipped/pre-set, see the [ CONFIG FILE USAGE ] Section for details
    # Select an IOS to use (must have proper patches), 2 commonly used IOSs are 249 and 36.
    # Select the device where you have saved the WADs.
    # Choose Nand Emulation Device (leave "disabled" to install to WADs to the Wii's real Nand)

  * Browse device to locate WADs ("A" to open folders, and "B" to go back)

  * Press the "A" Button on an individual WAD to (un)install.
    * If no file is marked, the normal single file (un)install menu will appear.
    * If at least one file is marked, the batch (un)install menu will appear.

  * Press the "+" Button to (un)mark the selected WAD for batch installation
  * Press the "-" Button to (un)mark the selected WAD for batch uninstallation
  * Hold +/- for 2 seconds to (un)mark all items in a directory.
    * A "+" will appear in front of the name of marked WADs for installation
    * A "-" will appear in front of the name of marked WADs for uninstallation

  * Press the "1" Button to go to the operations menu (currently can only delete single WADs)


==[ NOTES ]:==

  To use the NAND emulation is necessary to have a COMPLETE copy
  of the NAND filesystem in the root of the FAT device.


==;[ CONFIG FILE USAGE ]:==

; wm_config.txt resides in sd:/wad, and it is optional. You will get all the prompts if you don't have this file. If you are missing this file, copy and paste the entire [ CONFIG FILE USAGE ] Section to a new text file, rename it wm_config.txt, and Save it to sd:/wad.

*; To bypass any of the params, just comment out the line using a ";" at the beginning of the line*

; If you don't have any of the other parameters, it will prompt you for it

; The param keywords are case-sensitive at this point.

; No spaces precede the keyword on a line

; If you don't have a password in the config file, the default is no password

; If you don't have a startupPath, the default is /wad

; Blank lines are ignored.

; Password=your_password ("LRUD" only, where L=left, R=right, U=up, D=down on the WiiMote or GC Controller, max 10 characters)

; StartupPath=startupPath (starting at the root dir "/"). Be sure that the path exists, else you will get an error.



*;Password=UDLR*

*;StartupPath=/myWAD*

; Example of StartupPath at the root of the SD card

;StartupPath=/

; cIOS: 249, 222, whatever

*;cIOSVersion=249*

; FatDevice: sd usb usb2 gcsda gcsdb

*;FatDevice=sd*

; NANDDevice: Disable SD USB

; Note that WM will prompt for NAND device only if you selected cIOS=249

*;NANDDevice=Disable*




==[ CHANGELOG ]:==

  * YAWMM (cwstjdenobs)
    * Hold +/- for 2 seconds to select all items in a directory.
    * Supports Hermes v4/v5 cIOS. Mainly useful if 202 works best for your HDD/SDHC card.
    * More detailed failed report after batch un/installs.
    * Will not uninstall The System Menu, the SM's region EULA or rgsel, or their IOSs.
    * Will not install the wrong regions SM.
    * Will not load stub IOS.
    * Will not install titles if they rely on a stub IOS.
    * Will not install stub SM, EULA and rgsel IOS.
	* Will not install a SM lower than 4.0 on a boot2v4 Wii.
    * Gives a warning when uninstalling the HBCs IOS.
    * Read config file from usb.
    * Can load an alternative background from /wad/background.png.
    * Won't load incompatible cIOS in SNEEK.
  
  * Wad Manager Multi-Mod (Leathl) v3:
    * Reassigned the buttons again (Read Usage section)
    * Shortened on-screen instructions down to 2 lines
    * Fixed bug when changing directory with marked WADs

  * Wad Manager Multi-Mod (Leathl) v2:
    * Reassigned the buttons (Read the Usage section)
    * Batch un- and installation is now possible in one turn
    * Added confirmation screen before (un-)installation
    * Added file operations (can yet only delete single files)

  * Wad Manager Multi-Mod (Leathl) v1:
    * Added batch (un)installation

  * Wad Manager Folder Mod (WiiNinja) v3:
    * Config file contains additional params to automate the selection of IOS, Storage Device, and NAND Emulation
    * WiiLight mod by mariomaniac33

  * Wad Manager Folder Mod (WiiNinja) v2:
    * Config file in sd:/wad/wm_config.txt
    * Optional startup password. Very simple password using the D-Pads
    * Optional startup path

  * Wad Manager Folder Mod (WiiNinja) v1:
    * Folder support (10 levels deep)
    * GC Controller support
    * Removed disclaimer prompt
    * Sorg's enhancements are included

  * Wad Manager (Waninkoko) v1.5:
    * Allows NAND Emulation.

  * Wad Manager (Waninkoko) v1.4:
    * Allows user to choose which IOS to install WAD Manager.


==[ SOURCE ]:==
  * http://code.google.com/p/yawmm/source/checkout


==[ THANKS ]:==
  * X-Flak
  * Pepxl

==[ KUDOS ]:==
  * Leathl
  * WiiNinja
  * Sorg
  * Waninkoko
  * Team Twiizers/devkitPRO