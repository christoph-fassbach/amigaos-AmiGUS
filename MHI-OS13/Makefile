MODNAME=            mhiamigus
VERSION=            1
REVISION=           0

LIBFILE=            $(MODNAME).library

FD_FILE=            header/$(MODNAME)_lib.fd
PRAGMA_FILE=        header/$(MODNAME)_pragmas.h

LIBS=               LIB:sc.lib LIB:amiga.lib

LIBENT=             LIB:libent.o
LIBINIT=            LIB:libinitr.o
LIBPREFIX=          _LIB_

###########################

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

LIB_DST         = $(DDIR)$(LIBFILE)

MHI_INCLUDE     = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

C_SRCS          = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_C_SRCS     = $(foreach C_SRC,$(C_SRCS),$(subst $(SDIR),$(BDIR),$(C_SRC)))
BDIR_C_OBJS     = $(BDIR_C_SRCS:%c=%o)

A_SRCS          = $(wildcard $(SDIR)$(WILDCARD).asm)
BDIR_A_SRCS     = $(foreach A_SRC,$(A_SRCS),$(subst $(SDIR),$(BDIR),$(A_SRC)))
BDIR_A_OBJS     = $(BDIR_A_SRCS:%asm=%o)

OBJECTS         = $(BDIR_C_OBJS) $(BDIR_A_OBJS)

CFLAGS          = STREQ STRMER NOSTKCHK LIBCODE                           \
                  IDIR=$(MHI_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR) \
                  OBJNAME

CC            = $(RUNNER) sc
AS            = $(RUNNER) sc
LD            = $(RUNNER) slink
FD            = $(RUNNER) sc:c/fd2pragma

##################################

all: FOLDERS $(LIB_DST) $(PRAGMA_FILE)

$(LIB_DST): $(LIBFILE)
	$(CP) $(LIBFILE) $(LIB_DST)
	$(RM) $(LIBFILE) $(RM_SUFFIX)

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

clean:
	-@$(RM) $(LIBFILE) $(RM_SUFFIX)
	-@$(RM) $(OBJECTS) $(RM_SUFFIX)
	-@$(RM) $(MODNAME).map $(RM_SUFFIX)
	-@$(RM) $(PRAGMA_FILE) $(RM_SUFFIX)

###############################################################################

$(LIBFILE): $(OBJECTS) $(PRAGMA_FILE)
	$(LD) NOICONS \
		TO $(LIBFILE) \
		FROM $(LIBENT) $(LIBINIT) $(OBJECTS) \
		LIBFD $(FD_FILE) LIBPREFIX $(LIBPREFIX) \
		LIBVERSION $(VERSION) LIBREVISION $(REVISION) \
		LIB $(LIBS)

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
	$(LD) NOICONS FROM LIB:c.o,$(BDIR)$@.o TO test/$@ LIB $(LIBS) 
