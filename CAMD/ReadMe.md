Requirements:
-------------

* Point CAMDDeveloper: at /Extras/MIDI/camd-37.1/development from Amiga Developer CD 2.1
  * https://aminet.net/mus/midi/camd.lha
  * https://aminet.net/mus/midi/camd40.lha
* OR:
  git clone -n --depth=1 --filter=tree:0 https://github.com/aros-development-team/AROS.git
  cd AROS
  git sparse-checkout set --no-cone /compiler/include/midi compiler/include/SDI
  git checkout
* make from https://aminet.net/dev/c/make-4.4.1.lha in path
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
* vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha


