# Makefile for TI-84 CE Math Solver
# Based on the CE C/C++ Toolchain standard template

# Application name
NAME = MATHSLVR

# Source files
C_FILES = main.c mathsolver.c ui.c
# You can rename the source files based on how you've organized your code

# Object files
OBJS = $(patsubst %.c,%.o,$(C_FILES))

# Include the CE Toolchain standard makefile setup
include $(shell cedev-config --makefile)

# Default to debug mode for development
DEBUG ?= 1

# Compiler flags
CFLAGS += -Wall -Wextra -pedantic -std=c11
CFLAGS += -Os # Optimize for size

# Additional libraries needed for the project
LIBS = -lc -lm -lgraphx -lkeypadc -lfileioc

# CEmu emulator support (if available)
ifndef CEmu
  ifeq ($(OS),Windows_NT)
    CEmu = cemu.exe
  else
    CEmu = cemu
  endif
endif

# Clean rule (overriding the default one from the toolchain)
clean:
	$(RM) $(OBJS) $(NAME).8xp $(NAME).hex obj/*.bc obj/*.src obj/*.rst

# Build and run in emulator
run: all
	$(CEmu) $(NAME).8xp &

# Extra rules for convenience

# Rebuild everything
rebuild: clean all

# Header file dependencies
main.o: headers/mathsolver.h headers/ui.h
mathsolver.o: headers/mathsolver.h
ui.o: headers/mathsolver.h headers/ui.h

# Custom help rule
help:
	@echo "TI-84 CE Math Solver Makefile Help"
	@echo "--------------------------------"
	@echo "make         - Build the application"
	@echo "make clean   - Remove all generated files"
	@echo "make rebuild - Clean and rebuild"
	@echo "make run     - Build and run in CEmu emulator (if available)"
	@echo "make debug   - Build with debug symbols"
	@echo "DEBUG=0 make - Build without debug symbols (for release)"