# Makefile for TI-84 CE Math Solver
# Based on the CE C/C++ Toolchain standard template

# Application definition
NAME = MATHSLVR
DESCRIPTION = "TI-84 CE Math Solver"
#ICON = icon.png

# Output Configuration
COMPRESSED = YES			# Compress the output file
COMPRESSED_MODE = zx0		# Use the zx0 compression algorithm
ARCHIVED = YES				# Build the program to be stored in the archive

# Object files
OBJS = $(patsubst %.c,%.o,$(C_FILES))

# Include the CE Toolchain standard makefile setup
include $(shell cedev-config --makefile)

# Default to debug mode for development
DEBUG ?= 1

# Enable verbose output to see all commands
VERBOSE ?= 0
ifeq ($(VERBOSE),1)
	Q :=
else
	Q := @
endif

# Compiler flags
CFLAGS += -Wall        # Enable all compiler's warning messages
CFLAGS += -Wextra      # Enable additional warning messages not covered by -Wall
CFLAGS += -pedantic    # Enforce strict compliance with the C standard
CFLAGS += -std=c11     # Specify the C language standard to use (C11 in this case)
CFLAGS += -Os          # Optimize for size, reducing the binary size

# Add the DEBUG preprocessor definition when DEBUG=1
ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG	# Define the DEBUG preprocessor symbol
endif

# Additional libraries needed for the project
LIBS += -lc         # Standard C library
LIBS += -lm         # Math library for mathematical functions
LIBS += -lgraphx    # Graphics library for drawing on the TI-84 CE screen
LIBS += -lkeypadc   # Keypad control library for handling user input
LIBS += -lfileioc   # File I/O library for reading/writing files

release:
	$(MAKE) DEBUG=0 rebuild

# Clean rule (overriding the default one from the toolchain)
clean:
	@if exist bin rd bin /s /q
	@if exist obj rd obj /s /q

# Extra rules for convenience

# Rebuild everything
rebuild: clean all

# Custom help rule
help:
	@echo "TI-84 CE Math Solver Makefile Help"
	@echo "--------------------------------"
	@echo "make         - Build the application"
	@echo "make clean   - Remove all generated files"
	@echo "make rebuild - Clean and rebuild"
	@echo "make debug   - Build with debug symbols"
	@echo "make release - Build without debug symbols"