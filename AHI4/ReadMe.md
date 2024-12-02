## Usage 
### Requirements:
* Software
  * AmigaOS 2.0.4 or higher
  * AHI 4.18 - e.g. from https://aminet.net/driver/audio/ahiusr_4.18.lha
* Hardware
  * AmiGUS board - :D - yeah, really, does not make sense without it
  * 68000 - 68020 or higher recommended
  * Some FastRAM - hard to tell how much, some for AHI, some for OS, some for the audio application you are using, overall at least 4MB maybe?
  * HardDisk - >10MB, do not even think about getting all to work on 880kB floppies!
### Installation:
Some easy manual steps for the time being, an installer for everything AmiGUS may be added later.
* Download a binary archive from the releases section.
* Transfer the archive to your Amiga.
* Extract the archive somewhere somehow. If you have never seen an lha file before please find yourself a manual about it.
* Copy `AMIGUS` modefile to `DEVS:AudioModes`
* Copy `AmiGUS.audio` to `DEVS:AHI`
* Reboot.
* Open AHI settings from `Preferences` and set some AmiGUS audio modes.
### Tested Sofware:
AHI shall be the last resort, prefer MHI or CAMD for God's sake!
* HippoPlayer
  * 2.45 from https://aminet.net/mus/play/hippoplayer.lha
  * 2.61 (recommended - ! - can do MHI for MP3 playback as well) from https://aminet.net/mus/play/hippoplayerupdate.lha
* Play16
  * 1.9 for 68000, from https://aminet.net/mus/play/Play16_v1.9.lha
  * 1.10 otherwise, from https://aminet.net/mus/play/Play16.lha
* AmigaAMP
  * 2.25 from https://aminet.net/mus/play/AmigaAMP.lha
  * 3.33 from from https://aminet.net/mus/play/AmigaAMP3-68k.lha
* Duke Nukem 3D
  * 1.3 from http://aminet.net/game/shoot/jfduke3d.lha
* Advanced AHI HD-Recorder
  * 1.21 from http://aminet.net/mus/play/AHIRecord-68k.lha
* MTRS - Multi Track Recording System
  * 2.16 from http://aminet.net/mus/edit/MTRS_V2.16.lha

## Building
### Requirements:
* AHI4 DevKit, from https://www.lysator.liu.se/ahi/v4-site/files/ahi/system/ahidev.lha
* make 4.4.1 from https://aminet.net/dev/c/make-4.4.1.lha
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc 0.9h patch 2, newer untested
  * vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
  * vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha
* OPTIONAL: SASC 6.58
  * SAS/C 6.50 or later
  * Applied patch to 6.55 from https://aminet.net/dev/c/sc655pch.lha
  * Applied patch to 6.56 from https://aminet.net/dev/c/sc656pch.lha
  * Applied patch to 6.57 from https://aminet.net/dev/c/sc657pch.lha
  * Applied patch to 6.58 from https://aminet.net/dev/c/sc658pch.lha
  * Applied Y2K patch from https://aminet.net/dev/c/SASC_Y2k_patch.lha

### Setup:
* All of the requirements above installed somewhere ...
* ... and `path ... ADD`ed accordingly to make it callable.

----------------------
* Point AHI4Developer: at AHI/Developer from https://www.lysator.liu.se/ahi/v4-site/files/ahi/system/ahidev.lha - no installation needed
* make from https://aminet.net/dev/c/make-4.4.1.lha in path
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
* vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha

TODO:
-----
- Use fd2pragma on ahi_sub_lib.fd to create needed headers!!!
