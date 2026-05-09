# to generate assembler listing with LTO, add to LDFLAGS: -Wa,-adhln=$@.listing,--listing-rhs-width=200
# for better annotations add -dA -dP
# to generate assembler source with LTO, add to LDFLAGS: -save-temps=cwd

ifdef OS
	WINDOWS = 1
	SHELL = cmd.exe
endif

NETDEV_DIR := ../NetDevice
NETDEV_SOURCES := $(NETDEV_DIR)/paulanetio.c $(NETDEV_DIR)/paulanetstruct.c
NETDEV_OBJECTS := $(addprefix obj/,$(patsubst %.c,%.o,$(notdir $(NETDEV_SOURCES))))

subdirs := $(wildcard */)
VPATH = $(subdirs)
cpp_sources := $(wildcard *.cpp) $(wildcard $(addsuffix *.cpp,$(subdirs)))
cpp_objects := $(addprefix obj/,$(patsubst %.cpp,%.o,$(notdir $(cpp_sources))))
c_sources := $(wildcard *.c) $(wildcard $(addsuffix *.c,$(subdirs)))
c_objects := $(addprefix obj/,$(patsubst %.c,%.o,$(notdir $(c_sources))))
# s_sources := support/gcc8_a_support.s support/depacker_doynax.s
s_objects := $(addprefix obj/,$(patsubst %.s,%.o,$(notdir $(s_sources))))
vasm_sources := $(wildcard *.asm) $(wildcard $(addsuffix *.asm, $(subdirs)))
vasm_objects := $(addprefix obj/, $(patsubst %.asm,%.o,$(notdir $(vasm_sources))))
objects := $(cpp_objects) $(c_objects) $(s_objects) $(vasm_objects) $(NETDEV_OBJECTS)
# This is the place to declare the static libraries
static_libs := -lc -lm -ldebug -lnet -lunix -lc -lm -lamiga # to use Amiga paths
#static_libs := -lm -ldebug -lnet -lunix -lc -lm -lamiga # to use UNIX paths

# https://stackoverflow.com/questions/4036191/sources-from-subdirectories-in-makefile/4038459
# http://www.microhowto.info/howto/automatically_generate_makefile_dependencies.html

program = out/a
OUT = $(program)
CC = m68k-amiga-elf-gcc
VASM = vasmm68k_mot

ifdef WINDOWS
# This is the path to the Amiga OS 3.1 SDK
	SDKDIR = $(abspath $(dir $(shell where $(CC)))..\m68k-amiga-elf\sys-include)
# And this is the path to the Amiga OS 3.2 NDK
#	SDKDIR = $(abspath $(dir $(shell where $(CC)))..\m68k-amiga-elf\ndk-include)
else
# This is the path to the Amiga OS 3.1 SDK
	SDKDIR = $(abspath $(dir $(shell which $(CC)))../m68k-amiga-elf/sys-include)
# And this is the path to the Amiga OS 3.2 NDK
#	SDKDIR = $(abspath $(dir $(shell which $(CC)))../m68k-amiga-elf/ndk-include)
endif

# The directory where the static libraries are located
LIBSDIR = $(abspath libs)

CCFLAGS = -g -MP -MMD -m68000 -Ofast -nostdlib -Wextra -Wno-unused-function -Wno-volatile-register-var -fomit-frame-pointer -fno-tree-loop-distribution -Wno-builtin-declaration-mismatch -Wno-incompatible-pointer-types -flto -fwhole-program -fno-exceptions -D__NOLIBBASE__ -I${SDKDIR} -I$(LIBSDIR)/library -I$(LIBSDIR)/library/include -I$(NETDEV_DIR) -D__stdargs=
CPPFLAGS= $(CCFLAGS) -fno-rtti -fcoroutines -fno-use-cxa-atexit --Wno-incompatible-pointer-types -Wno-builtin-declaration-mismatch -D__NOLIBBASE__
ASFLAGS = -Wa,-g,--register-prefix-optional,-I$(SDKDIR),-D
LDFLAGS = -Wl,--emit-relocs,-Ttext=0,-Map=$(OUT).map -L${LIBSDIR}
VASMFLAGS = -m68000 -Felf -opt-fconst -nowarn=62 -dwarf=3 -quiet -x -I. -I$(SDKDIR)

all: $(OUT).exe

$(OUT).exe: $(OUT).elf
	$(info Elf2Hunk $(program).exe)
	@elf2hunk $(OUT).elf $(OUT).exe -s

$(OUT).elf: $(objects)
	$(info Linking $(program).elf)
	@$(CC) $(CCFLAGS) $(LDFLAGS) $(objects) $(static_libs) -o $@
	@m68k-amiga-elf-objdump --disassemble --no-show-raw-ins --visualize-jumps -S $@ >$(OUT).s

clean:
	$(info Cleaning...)
ifdef WINDOWS
	@del /q obj\* out\*
else
	@$(RM) obj/* out/*
endif

-include $(objects:.o=.d)

$(cpp_objects) : obj/%.o : %.cpp
	$(info Compiling $<)
	@$(CC) $(CPPFLAGS) -c -o $@ $<

$(c_objects) : obj/%.o : %.c
	$(info Compiling $<)
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(NETDEV_OBJECTS) : obj/%.o : $(NETDEV_DIR)/%.c
	$(info Compiling $<)
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(s_objects): obj/%.o : %.s
	$(info Assembling $<)
	@$(CC) $(CCFLAGS) $(ASFLAGS) -c -o $@ $<

$(vasm_objects): obj/%.o : %.asm
	$(info Assembling $<)
	@$(VASM) $(VASMFLAGS) -o $@ $<