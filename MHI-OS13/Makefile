#
# This file is part of the mhiAmiGUS.library.
#
# mhiAmiGUS.library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# mhiAmiGUS.library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU LesserGeneral Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
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
  RUNNER        = vamos
  IGNORE_VAMOS  = -
  LHA           = jlha

  LIB_HOST      = cross

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL1      = curl --netrc --upload-file $(LIB_FILE_DEST)       ftp://192.168.0.4/cf0/Libs/MHI/
  INSTALL0      = cp $(LIB_FILE_DEST) ~/Documents/FS-UAE/Shared/MHI/
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

  LIB_HOST      = native

  INSTALL0      = copy FROM $(LIB_FILE_DEST)	TO	Libs:MHI/		CLONE
  INSTALL1      = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

LIB_NAME        = mhiamigus
LIB_FILE        = $(LIB_NAME).library
LIB_VERSION     = 1
LIB_REVISION    = 2
LIB_PREFIX      = _LIB_

LIB_ENT         = LIB:libent.o
ifeq (1, 1)
# just put all into NEAR section
  LIB_LIBS      =
  LIB_INIT      = LIB:libinit.o
else
# also have a FAR section
  LIB_LIBS      = LIB:sc.lib
  LIB_INIT      = LIB:libinitr.o
endif

LIB_FILE_DEST   = $(DDIR)$(LIB_FILE)

FD_FILE         = $(HDIR)$(LIB_NAME)_lib.fd
PRAGMA_FILE     = $(HDIR)$(LIB_NAME)_pragmas.h

###########################

MHI_INCLUDE     = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

C_SRCS          = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_C_SRCS     = $(foreach C_SRC,$(C_SRCS),$(subst $(SDIR),$(BDIR),$(C_SRC)))
BDIR_C_OBJS     = $(BDIR_C_SRCS:%c=%o)

A_SRCS          = $(wildcard $(SDIR)$(WILDCARD).asm)
BDIR_A_SRCS     = $(foreach A_SRC,$(A_SRCS),$(subst $(SDIR),$(BDIR),$(A_SRC)))
BDIR_A_OBJS     = $(BDIR_A_SRCS:%asm=%o)

LIB_OBJS        = $(BDIR_C_OBJS) $(BDIR_A_OBJS)

###########################

AFLAGS          = -I$(NDK_ASM_INCLUDE) -quiet -phxass -Fhunk -o
CFLAGS          = PARAMETERS=REGISTER DATA=FAR CODE=FAR                               \
                  STREQ STRMER NOSTACKCHECK NOMULTIPLEINCLUDES NOCHECKABORT NOICONS   \
                  IDIR=$(MHI_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)              \
                  DEF __NOLIBBASE__                                                   \
                  DEF LIB_FILE=$(LIB_FILE) DEF LIB_DATE=__AMIGADATE__                 \
                  DEF LIB_VERSION=$(LIB_VERSION) DEF BASE_GLOBAL                      \
                  DEF LIB_REVISION=$(LIB_REVISION) DEF LIB_CPU=000                    \
                  DEF LIB_COMPILER=SAS/C DEF LIB_HOST=$(LIB_HOST)                     \
                  OBJNAME

CC              = $(RUNNER) sc
AS              = $(RUNNER) sc
LD              = $(RUNNER) slink
FD              = $(RUNNER) sc:c/fd2pragma

##################################

.PHONY: all install test INCLUDES FOLDERS clean dist-clean dist

all: FOLDERS $(LIB_FILE_DEST) $(PRAGMA_FILE)

ECHO:
	echo $(SRCS)

install: all
	-$(INSTALL0)
	-$(INSTALL1)

clean:
	-$(RM) $(DDIR) $(BDIR) $(RM_SUFFIX)

dist-clean: clean
	make -f Makefile-Includes clean

dist: all
	$(LHA) a target/MHIbin.lha target/$(WILDCARD)

INCLUDES:
	make -f Makefile-Includes all 

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

$(LIB_FILE_DEST): $(LIB_FILE)
	$(CP) $(LIB_FILE) $(LIB_FILE_DEST)
	$(RM) $(LIB_FILE) $(RM_SUFFIX)

$(LIB_FILE): $(LIB_OBJS) $(PRAGMA_FILE)
	$(LD) NOICONS \
		TO $(LIB_FILE) \
		FROM $(LIB_ENT) $(LIB_INIT) $(LIB_OBJS) \
		LIBFD $(FD_FILE) LIBPREFIX $(LIB_PREFIX) \
		LIBVERSION $(LIB_VERSION) LIBREVISION $(LIB_REVISION) \
		LIB $(LIB_LIBS)

$(PRAGMA_FILE): $(FD_FILE)
	$(FD) $(FD_FILE) $(PRAGMA_FILE)

###############################################################################

$(BDIR)%.o: $(SDIR)%.c $(HDIR)*
	$(CC) LIBCODE $(CFLAGS) $@ $<

$(BDIR)%.o: $(SDIR)%.asm $(HDIR)*
	$(AS) $(AFLAGS) $@ $<

###############################################################################

test_library: $(PRAGMA_FILE)
	$(CC) DATA=far NOMINC $(CFLAGS) SAVEDS IGN=73 OBJNAME $(BDIR)$@.o test/$@.c
	$(LD) NOICONS FROM LIB:c.o,$(BDIR)$@.o TO test/$@ LIB LIB:sc.lib,LIB:amiga.lib
	-cp test/$@ ~/Documents/FS-UAE/Shared/MHI/
	-curl --netrc --upload-file test/test_library       ftp://192.168.0.4/cf0/Expansion/AmiGUS/
