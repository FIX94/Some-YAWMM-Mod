# YAWM ModMii Edition
Yet Another Wad Manager ModMii Edition (yawmME) is based on Some-YAWMM-Mod, which is based on YAWMM, which is based on WAD Manager, modded up by various people.

**Changes since Some YAWMM Mod:**
- Can now also launch apps (both dols and elfs).
- Priiloader can be retained when installing a new System Menu.
- Temporarily disable region checks/protection by entering the Konami code instead A when selecting a device, allowing installation of WADs to permit region changing.
- Improved region detection to allow those with custom system menu versions installed to install a different system menu without requiring the Konami code.
- System information added to device selection screen (i.e. loaded IOS, System Menu, Region, AHB access & Priiloader installed).
- The device select screen will now only let you choose between available devices.
- Added an option to remount the source devices to the device selection menu.
- Counter added to onscreen progress (i.e. Processing WAD: X/X)
- Power and reset buttons are now inactive when installing WADs.
- Fixed some ticket handling that was preventing a stock IOS from installing other stock IOSs with version numbers equal to or greater than those already installed.
- Pressing 1 on a folder no longer gives to option to "delete a wad".
- Improved error handling\codes and messaging.
- Updated name and graphics.
- yawmME is brought to you by OverjoY, blackb0x, scooby74029, Peter0x44 and XFlak; graphics by AuroraNemoia.

For more info on Some YAWMM Mod, check its readme below.


### Some YAWMM Mod
Based on YAWMM which itself is based on WAD Manager, modded up by various people.

**Changes from the last YAWMM googlecode version:**
- Updated to be compiled in the latest devkitppc, libogc versions
- Added on-the-fly IOS patches when AHBPROT is disabled, so no cIOS is required in those cases
- Support for classic controller, wiiu pro controller (on both wii and vwii) and wiiu gamepad (in wii vc mode)
- Small corrections in how the root path is selected (having no "wad" folder now correctly displays on device root)

For more info on YAWMM itself, check its [original readme](README_YAWMM.txt).
