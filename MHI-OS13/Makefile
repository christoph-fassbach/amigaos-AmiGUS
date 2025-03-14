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
  LINUX          = 1

  NDK            = $(VBCC)/NDK3.2/
  WILDCARD       = *

  MKDIR          = mkdir -p
  MKDIR_SUFFIX   =
  RM             = rm -rf
  RM_SUFFIX      =
  CP             = cp
  RUNNER         = vamos
  IGNORE_VAMOS   = -
  LHA            = jlha

  HOST_TOOLCHAIN = CROSS_TOOLCHAIN

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL1       = curl --netrc --upload-file $(LIB_DST)       ftp://192.168.0.4/cf0/Libs/MHI/
  INSTALL0       = cp $(LIB_DST) ~/Documents/FS-UAE/Shared/MHI/
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

  HOST_TOOLCHAIN = NATIVE_TOOLCHAIN

  INSTALL0       = copy FROM $(LIB_DST)	TO	Libs:MHI/		CLONE
  INSTALL1       = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

MODNAME         = mhiamigus
VERSION         = 1
REVISION        = 0

LIB_FILE        = $(MODNAME).library
LIB_DST         = $(DDIR)$(LIB_FILE)

FD_FILE         = $(HDIR)$(MODNAME)_lib.fd
PRAGMA_FILE     = $(HDIR)$(MODNAME)_pragmas.h

LIB_PREFIX         = _LIB_
LIB_ENT            = LIB:libent.o
ifeq (1, 1)
# just put all into NEAR section
  LIB_LIBS         =
  LIB_INIT         = LIB:libinit.o
else
# also have a FAR section
  LIB_LIBS         = LIB:sc.lib
  LIB_INIT         = LIB:libinitr.o
endif

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

CFLAGS          = STREQ STRMER NOSTKCHK LIBCODE                           \
                  IDIR=$(MHI_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR) \
                  OBJNAME

CC            = $(RUNNER) sc
AS            = $(RUNNER) sc
LD            = $(RUNNER) slink
FD            = $(RUNNER) sc:c/fd2pragma

##################################

all: FOLDERS $(LIB_DST) $(PRAGMA_FILE)

install: all
	-$(INSTALL0)
	-$(INSTALL1)

$(LIB_DST): $(LIB_FILE)
	$(CP) $(LIB_FILE) $(LIB_DST)
	$(RM) $(LIB_FILE) $(RM_SUFFIX)

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

clean:
	-@$(RM) $(LIB_FILE) $(RM_SUFFIX)
	-@$(RM) $(LIB_OBJS) $(RM_SUFFIX)
	-@$(RM) $(MODNAME).map $(RM_SUFFIX)
	-@$(RM) $(PRAGMA_FILE) $(RM_SUFFIX)

###############################################################################

$(LIB_FILE): $(LIB_OBJS) $(PRAGMA_FILE)
	$(LD) NOICONS \
		TO $(LIB_FILE) \
		FROM $(LIB_ENT) $(LIB_INIT) $(LIB_OBJS) \
		LIBFD $(FD_FILE) LIBPREFIX $(LIB_PREFIX) \
		LIBVERSION $(VERSION) LIBREVISION $(REVISION) \
		LIB $(LIB_LIBS)

$(PRAGMA_FILE): $(FD_FILE)
	$(FD) $(FD_FILE) $(PRAGMA_FILE)

###############################################################################

$(BDIR)%.o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

$(BDIR)%.o: $(SDIR)%.asm $(HDIR)*
	$(AS) $(AFLAGS) $@ $<

###############################################################################

test_library: $(PRAGMA_FILE)
	$(CC) DATA=far NOMINC STRMER STREQ NOSTKCHK SAVEDS IGN=73 IDIR=$(HDIR) OBJNAME $(BDIR)$@.o test/$@.c
	$(LD) NOICONS FROM LIB:c.o,$(BDIR)$@.o TO test/$@ LIB LIB:sc.lib,LIB:amiga.lib
