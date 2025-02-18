# -*- makefile -*-

# ARCH detection in Make, used to pick out which trampoline assembly syntax we're gonna use
ARCH := $(shell $(CC) -dumpmachine | sed "s/\([^-]*\).*$$/\1/")

# Normalize ppc64le to powerpc64le
ifeq ($(ARCH),ppc64le)
  ARCH := powerpc64le
endif
# We map amd64 to x86_64 for compatibility with systems that identify 64-bit systems as such
ifeq ($(ARCH),amd64)
  ARCH := x86_64
endif
# We map arm64 (Apple spelling) to aarch64 to avoid having to deal with both spellings everywhere
ifeq ($(ARCH),arm64)
  ARCH := aarch64
endif

# Figure out OS and architecture (used to set `shlib_ext`)
OS := $(shell uname)
ifneq (,$(findstring MINGW,$(OS))$(findstring MSYS,$(OS))$(findstring CYGWIN,$(OS)))
  OS := WINNT
endif

ifeq ($(OS), WINNT)
  SHLIB_EXT := dll
  EXE := .exe
  binlib := bin
else ifeq ($(OS), Darwin)
  SHLIB_EXT := dylib
  binlib := lib
else
  SHLIB_EXT := so
  binlib := lib
endif

LBT_CFLAGS := -g -O2 -std=c99 -fPIC -DLIBRARY_EXPORTS -D_GNU_SOURCE $(CFLAGS)
LBT_LDFLAGS := $(LDFLAGS)

ifeq ($(OS),Linux)
# On linux, we need to link `libdl` to get `dlopen`
LBT_LDFLAGS += -ldl
endif

ifeq ($(OS),WINNT)
# On windows, we need to enable unicode mode
LBT_CFLAGS += -municode
endif

# On windows, we must generate import libraries
ifeq ($(OS),WINNT)
define IMPLIB_FLAGS
  -Wl,--out-implib,$(1).a
endef
else
define IMPLIB_FLAGS
endef
endif

# If we're on x86_64 or i686, we may need to convert back f2c-style bindings (such as Apple's Accelerate)
# from returning doubles to instead return float's.  We enable automatic F2C detection on those platforms.
F2C_AUTODETECTION := 0
ifeq ($(ARCH),x86_64)
  LBT_CFLAGS += -DF2C_AUTODETECTION
  F2C_AUTODETECTION := 1
endif


ifeq ($(VERBOSE),0)
ENDCOLOR := "\033[0m"
CCCOLOR := "\033[34m"
SRCCOLOR:="\033[33m"

PRINT_CC = printf '%b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR); $(1)
else
PRINT_CC = echo $(1); $(1)
endif

# Default installation location; we expect you to be overriding this
prefix ?= prefix
builddir ?= build
