# TI-84 CE Projects

This directory contains projects specifically designed for the TI-84 CE calculator platform, built using the CE C/C++ Toolchain.

## Overview

The TI-84 CE is a powerful graphing calculator that supports custom C/C++ applications. This folder contains calculator-specific implementations of mathematical tools and utilities that extend the calculator's capabilities beyond its built-in functionality.

## Projects

### MathSolver for TI-84 CE (`mathslvr/`)

A powerful mathematical expression evaluator for the TI-84 CE calculator that provides:

- Complex expression parsing and evaluation
- Step-by-step calculation tracking
- Multiple calculation modes (normal, truncated, rounded)
- Support for mathematical functions and constants
- Precision control by decimal places or significant digits

[View MathSolver Documentation](./MathSlvr/readme.md)

### ASCII Chart Viewer (`ascii-chart/`)

A utility program for browsing and identifying ASCII and extended ASCII characters (0-255):

- Display of all 256 ASCII/extended ASCII characters
- Organized view with hexadecimal values and character representations
- Special handling for control characters
- Easy navigation using calculator keys

[View ASCII Chart Viewer Documentation](./ASCII_chart/readme.md)

## Development Requirements

To build and work with these projects, you'll need:

- [CE C/C++ Toolchain](https://github.com/CE-Programming/toolchain)
- Make build system
- [CEmu](https://github.com/CE-Programming/CEmu) (recommended for testing)
- TI Connect CE (for transferring programs to a physical calculator)

## Building the Projects

Each project can be built individually:

```bash
# Navigate to a specific project directory
cd mathslvr

# Build the project
make

# The output will be a .8xp file in the bin directory
```

## Installation

Transfer the compiled `.8xp` files to your calculator using TI Connect CE or similar software.

## Common Utilities

Some projects may share common utilities or libraries. These shared components help maintain consistency across projects and reduce code duplication.

## Testing

The recommended approach for testing these projects is:

1. Build the project using `make`
2. Test in the CEmu emulator for initial verification
3. Transfer to a physical TI-84 CE calculator for final testing

## Acknowledgments

- CE Programming Team for the CE C/C++ Toolchain
- Texas Instruments for their calculator platform

---

*These projects are not affiliated with or endorsed by Texas Instruments.*