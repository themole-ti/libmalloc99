# Program name
NAME=malloctest8
LIBNAME=malloc99

# Paths to TMS9900 compilation tools
BASE_PATH=../../toolchain/tms9900/bin
AS=$(BASE_PATH)/tms9900-as
LD=$(BASE_PATH)/tms9900-ld
CC=$(BASE_PATH)/tms9900-gcc
AR=$(BASE_PATH)/tms9900-ar
INCLUDE_PATH=../include
LIB_PATH=../lib


# List of compiled objects used in executable
OBJECT_LIST_LIBRARY=					\
	bank0/malloc.o 							\

OBJECT_LIST= 									\
	$(OBJECT_LIST_LIBRARY)			\
  persistent/trampolines.o		\
  bank0/cart_header.o 				\
  bank0/crt0.o 								\
  bank0/main.o 								\


# List of all files needed in executable
PREREQUISITES= $(OBJECT_LIST) library

# Compiler flags
CCFLAGS= -lti99 -std=c99 -Werror -Wall -Os -s -Iinclude -I$(INCLUDE_PATH) -c

# Linker flags for flat cart binary
# Most of this is defined in the linker script
LDFLAGS= --cref --verbose --script cart.ld -Lpersistent -Lbank0 -Lbank1 -Lbank2 -Lbank3

# Recipe to compile the executable
all: $(PREREQUISITES)
	@echo
	@echo "\t[LD] $(OBJECT_LIST) -> $(NAME).bin"
	@$(LD) $(LDFLAGS) $(OBJECT_LIST) -lti99 -o $(NAME).bin -M > link.map
	@echo

# Recipe to compile all assembly files
%.o: %.asm
	@echo "\t[AS] $<..."
	@$(AS) $< -o $@

# Recipe to compile all C files
%.o: %.c
	@echo "\t[CC] $<..."
	@$(CC) $(CCFLAGS) $< -o $@

# Create static library
library: $(OBJECT_LIST_LIBRARY)
	@echo "\t[AR] $<... -> lib$(LIBNAME).a"
	@$(AR) rcs lib$(LIBNAME).a $(OBJECT_LIST_LIBRARY)

# Install binary files where needed
install: all
	@echo "\tCopying library lib$(LIBNAME).a to $(LIB_PATH)"
	@cp lib$(LIBNAME).a $(LIB_PATH)/
	@echo

# Recipe to clean all compiled objects
.phony clean:
	@echo "\tremoving compiled and intermediary files..."
	@rm -f persistent/*.o
	@rm -f bank?/*.o
	@rm -f *.rpk
	@rm -f link.map
	@rm -f lib$(LIBNAME).a
	@rm -f $(NAME).bin
	@echo
