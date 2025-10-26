#
# This file is part of the CAMD keyboard.
#
# CAMD keyboard is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# CAMD keyboard is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CAMD keyboard.
#
# If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(findstring /home,$(HOME)), /home)
  LINUX         = 1

  NDK           = $(VBCC)/NDK3.2/
  WILDCARD      = *

  MKDIR         = mkdir -p
  MKDIR_SUFFIX  =
  RM            = rm -rf
  RM_SUFFIX     =
  CP            = cp
  RUNNER        = vamos -c ../../.vamosrc
  IGNORE_VAMOS  = -
  LHA           = jlha

  APP_HOST      = cross

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL1      = curl --netrc --upload-file $(APP_FILE_DEST)       ftp://192.168.0.4/cf0/Expansion/AmiGUS/$(APP_NAME)
  INSTALL0      = cp $(APP_FILE_DEST) ~/Documents/FS-UAE/Shared/CAMD/$(APP_NAME)
else
  AMIGAOS       = 1
  
  NDK           = NDK_3.2:
  WILDCARD      = \#?

  MKDIR         = makedir
  MKDIR_SUFFIX  = ALL >NIL:
  RM            = delete
  RM_SUFFIX     = ALL >NIL:
  CP            = copy
  RUNNER        =
  IGNORE_VAMOS  =
  LHA           = lha

  APP_HOST      = native

  INSTALL0      = copy FROM $(APP_FILE_DEST)	TO	SYS:Expansion/AmiGUS/$(APP_NAME)		CLONE
  INSTALL1      = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

APP_CPU        ?= 000
APP_LOG        ?= FILE_LOG
# NO_LOG
# SER_LOG
# FILE_LOG
# MEM_LOG
APP_NAME        = CAMDkeyboard
APP_FILE        = $(APP_NAME)
APP_VERSION     = 1
APP_REVISION    = 0

APP_LIBS        =
APP_FILE_DEST   = $(DDIR)$(APP_FILE)-$(APP_CPU)-$(APP_LOG)

###########################

CAMD_INCLUDE    = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

C_SRCS          = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_C_SRCS     = $(foreach C_SRC,$(C_SRCS),$(subst $(SDIR),$(BDIR),$(C_SRC)))
BDIR_C_OBJS     = $(BDIR_C_SRCS:%.c=%-$(APP_CPU)-$(APP_LOG).o)

A_SRCS          = $(wildcard $(SDIR)$(WILDCARD).asm)
BDIR_A_SRCS     = $(foreach A_SRC,$(A_SRCS),$(subst $(SDIR),$(BDIR),$(A_SRC)))
BDIR_A_OBJS     = $(BDIR_A_SRCS:%.asm=%-$(APP_CPU)-$(APP_LOG).o)

APP_OBJS        = $(BDIR_C_OBJS) $(BDIR_A_OBJS)

###########################

# activate VBCC builds by adding `USEVBCC=1` to the make call!
ifeq (34,$(USEVBCC))
  ifdef AMIGAOS
    LDIR        = vlibos3:
  endif
  ifdef LINUX
    LDIR        = $(VBCC)/targets/m68k-amigaos/lib/
  endif

  AFLAGS        = -I$(NDK_ASM_INCLUDE) -quiet -phxass                  \
                  -DAPP_FILE=$(APP_FILE) -DAPP_VERSION=$(APP_VERSION)  \
                  -DAPP_REVISION=$(APP_REVISION)                       \
                  -Fhunk -o
  CFLAGS        = -c -Os -+ -c99 -cpu=68$(APP_CPU)                     \
                  -I$(CAMD_INCLUDE) -I$(HDIR)                          \
                  -D__NOLIBBASE__ -D$(APP_LOG)                         \
                  -DAPP_FILE=$(APP_FILE) -DAPP_DATE=__AMIGADATE__      \
                  -DAPP_VERSION=$(APP_VERSION)                         \
                  -DAPP_REVISION=$(APP_REVISION) -DAPP_CPU=$(APP_CPU)  \
                  -DAPP_COMPILER=VBCC -DAPP_HOST=$(APP_HOST)           \
                  -o
  LFLAGS        = -bamigahunk -x -s -mrel -Cvbcc -Bstatic -nostdlib -o
  LFLAGS1       =
  LFLAGST1      = $(LDIR)startup.o
  LFLAGST2      = $(LDIR)auto.lib $(LDIR)vc.lib
  # add -v below to see what vbcc does under the hood :)
  CC            = vc +kick13
  AS            = vasmm68k_mot
  LD            = vlink 
else ifdef USEVBCC
  ifdef AMIGAOS
    LDIR        = vlibos3:
  endif
  ifdef LINUX
    LDIR        = $(VBCC)/targets/m68k-amigaos/lib/
  endif

  AFLAGS        = -I$(NDK_ASM_INCLUDE) -quiet -phxass                  \
                  -DAPP_FILE=$(APP_FILE) -DAPP_VERSION=$(APP_VERSION)  \
                  -DAPP_REVISION=$(APP_REVISION)                       \
                  -Fhunk -o
  CFLAGS        = -O2 -c -cpu=68$(APP_CPU)                             \
                  -I$(CAMD_INCLUDE) -I$(HDIR)                          \
                  -D__NOLIBBASE__ -D$(APP_LOG) -DAPP_NAME=$(APP_NAME)  \
                  -DAPP_FILE=$(APP_FILE) -DAPP_DATE=__AMIGADATE__      \
                  -DAPP_VERSION=$(APP_VERSION)                         \
                  -DAPP_REVISION=$(APP_REVISION) -DAPP_CPU=$(APP_CPU)  \
                  -DAPP_COMPILER=VBCC -DAPP_HOST=$(APP_HOST)           \
                  -o
  LFLAGS        = -bamigahunk -x -s -mrel -Cvbcc -Bstatic -nostdlib -o
  LFLAGS1       = $(LDIR)startup.o
  LFLAGS2       = $(LDIR)auto.lib $(LDIR)vc.lib
  # add -v below to see what vbcc does under the hood :)
  CC            = vc +aos68k
  AS            = vasmm68k_mot
  LD            = vc +aos68k
else
  AFLAGS        = RESETOPTIONS DEFINE=__NOLIBBASE__ DEF APP_HOST=$(APP_HOST)   \
                  DEF APP_FILE=$(APP_FILE) DEF APP_VERSION=$(APP_VERSION)      \
                  DEF APP_REVISION=$(APP_REVISION)                             \
                  IDIR=$(CAMD_INCLUDE) IDIR=NDK_3.2:Include_I IDIR=$(HDIR)     \
                  OBJNAME

  CFLAGS        = RESETOPTIONS                                                 \
                  PARMS=STACK DATA=FAR CODE=FAR                                \
                  NOSTACKCHECK NOCHECKABORT NOICONS                            \
                  STRINGMERGE UNSIGNEDCHAR CPU=68$(APP_CPU)                    \
                  OPTIMIZE OPTIMIZERGLOBAL OPTIMIZERINLINE OPTIMIZERINLOCAL    \
                  OPTIMIZERLOOP OPTIMIZERPEEPHOLE OPTIMIZERSCHEDULE            \
                  IDIR=$(CAMD_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)      \
                  DEF __NOLIBBASE__ DEF $(APP_LOG) DEF APP_NAME=$(APP_NAME)    \
                  DEF APP_FILE=$(APP_FILE) DEF APP_DATE=__AMIGADATE__          \
                  DEF APP_VERSION=$(APP_VERSION)                               \
                  DEF APP_REVISION=$(APP_REVISION) DEF APP_CPU=$(APP_CPU)      \
                  DEF APP_COMPILER=SAS/C DEF APP_HOST=$(APP_HOST)              \
                  OBJNAME

  LFLAGS        = NOICONS STRIPDEBUG \
                  TO 
  LFLAGS1       = FROM LIB:c.o
  LFLAGS2       = LIBRARY LIB:scnb.lib LIB:amiga.lib
  CC            = $(RUNNER) sc
  AS            = $(RUNNER) sc
  LD            = $(RUNNER) slink
  FD            = $(RUNNER) sc:c/fd2pragma
endif

##################################

.PHONY: all install test support INCLUDES FOLDERS clean dist-clean dist

all: FOLDERS $(APP_FILE_DEST)

ECHO:
	@echo "C-sources: "$(C_SRCS)
	@echo "C-objects: "$(BDIR_C_OBJS)
	@echo "ASM-sources: "$(A_SRCS)
	@echo "USEVBCC: "$(USEVBCC)

install: all
	$(INSTALL0)
	$(INSTALL1)

clean-intermediate:
	-$(RM) $(BDIR) $(RM_SUFFIX)

clean: clean-intermediate
	-$(RM) $(DDIR) adf $(RM_SUFFIX)
	-$(RM) support/$(WILDCARD).o

dist-clean: clean
	make -f Makefile-Includes clean

dist: all
	$(LHA) a target/CAMDbin.lha target/$(WILDCARD)

INCLUDES:
	make -f Makefile-Includes all 

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

$(APP_FILE_DEST): $(APP_OBJS)
	$(LD) $(LFLAGS) $@ $(LFLAGS1) $^ $(LFLAGS2)

###############################################################################

$(BDIR)%-$(APP_CPU)-$(APP_LOG).o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

$(BDIR)%-$(APP_CPU)-$(APP_LOG).o: $(SDIR)%.asm $(HDIR)*
	$(AS) $(AFLAGS) $@ $<

###############################################################################

test: 

###############################################################################

support: FOLDERS get_mem_log
#camd_lib_check

get_mem_log:
	$(CC) $(CFLAGS) support/$@.o support/$@.c
	$(LD) $(LFLAGS) target/$@ $(LFLAGS1) $(LFLAGST1) support/$@.o $(LFLAGST2)
	-cp target/$@ ~/Documents/FS-UAE/Shared/CAMD/GetCamdMemLog
	-curl --netrc --upload-file target/$@       ftp://192.168.0.4/cf0/Expansion/AmiGUS/GetCamdMemLog

camd_lib_check:
	$(CC) $(CFLAGS) support/$@.o support/$@.c
	$(LD) $(LFLAGS) target/$@ $(LFLAGS1) $(LFLAGST1) support/$@.o $(LFLAGST2)
	-cp target/$@ ~/Documents/FS-UAE/Shared/CAMD/
	-curl --netrc --upload-file target/$@       ftp://192.168.0.4/cf0/Expansion/AmiGUS/

###############################################################################
