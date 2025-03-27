# TI-84 CE Mathematics & Utility Suite

A collection of powerful mathematical and utility programs for the TI-84 CE calculator, developed using the CE C/C++ Toolchain.

## Repository Overview

This repository contains multiple projects focused on enhancing the capabilities of TI-84 CE calculators:

1. **MathSolver2** - A comprehensive C# library for mathematical expression parsing and evaluation with step-by-step solutions
2. **MathSolver for TI-84 CE** - A TI-84 CE port of the MathSolver engine with a calculator-optimized interface
3. **ASCII Chart Viewer** - A utility program for viewing ASCII/extended ASCII characters on the TI-84 CE

## Project Summaries

### MathSolver2

A powerful C# library designed for parsing, evaluating, and displaying mathematical expressions with detailed step-by-step calculations and LaTeX notation support.

**Key Features:**
- Comprehensive expression parsing (arithmetic operations, functions, exponentiation, etc.)
- Step-by-step solutions with explanations
- Configurable arithmetic modes (normal, rounding, truncation)
- LaTeX notation support
- Variable substitution
- Mathematical constants (π, e, φ)

[View MathSolver2 Documentation](./MathSolver2/readme.md)

### MathSolver for TI-84 CE

A calculator-optimized port of the MathSolver engine, providing enhanced mathematical expression evaluation with a text-based UI for the TI-84 CE.

**Key Features:**
- Complex expression evaluation with proper operator precedence
- Step-by-step calculation tracking
- Variable support
- Built-in mathematical functions
- Multiple calculation modes
- Precision control

[View MathSolver for TI-84 CE Documentation](./TI84CE/mathslvr/readme.md)

### ASCII Chart Viewer

A simple utility program for browsing and identifying ASCII and extended ASCII characters (0-255) on the TI-84 CE.

**Key Features:**
- Browse all 256 ASCII/extended ASCII characters
- 16 characters per page organized display
- Hexadecimal value and character representation
- Special handling for control characters
- Simple navigation using calculator keys

[View ASCII Chart Viewer Documentation](./TI84CE/ASCII_chart/readme.md)

## Requirements

These projects require:

- TI-84 CE Calculator
- [CE C/C++ Toolchain](https://github.com/CE-Programming/toolchain)
- Make build system
- [CEmu](https://github.com/CE-Programming/CEmu) (for emulation)

The C# library (MathSolver2) additionally requires:
- .NET SDK

## Getting Started

### Building the Projects

Each project can be built separately:

```bash
# Navigate to a specific project directory
cd mathsolver-ti84ce

# Build the project
make

# For MathSolver2 (C# project)
cd mathsolver2
dotnet build
```

### Installation

Transfer the compiled `.8xp` files to your calculator using TI Connect CE or similar software.

## Repository Structure

```
.
├── mathsolver2/           # C# library version
│
├── ti84ce
│   ├── mathslvr/          # TI-84 CE calculator version
│   │
│   └── ascii-chart/       # ASCII Chart Viewer utility
│    
└── README.md              # This file (main repository documentation)
```

## Contributing

Contributions to any of the projects are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

These projects are available under the MIT License.

## Acknowledgments

- CE Programming Team for the CE C/C++ Toolchain
- Texas Instruments for their calculator platform
- TI-84 CE Documentation and community

---

*This repository is not affiliated with or endorsed by Texas Instruments.*