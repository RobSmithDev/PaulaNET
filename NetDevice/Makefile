###############################################################################
# Date, version, extra objects to build
# 
###############################################################################
DEVICEVERSION=1
DEVICEREVISION=0
DEVICEDATE=2026-05-11

###############################################################################
# Devices to build (1 or 2, keep DEVICEID2 empty if only one build is desired)
#
###############################################################################

DEVICEID=PaulaNET.device
DEFINES = -DDRIVER 
ASMDEFS = #
CPU = 68000
# webadf LZ4 (spec 2026-10-18 §4): the LZ4 half (webadf_lz4.c) and the 68000 codec (m68k/lz4, MIT)
ASMOBJECTS = webadf_lz4.o lz4_68k.o

DEVICEID2=

###############################################################################
# import generic ruleset
# 
###############################################################################
include Common.mk

lz4_68k.o : lz4_68k.asm
	$(VASM) $(VASMFORMAT) -o $@ $<



