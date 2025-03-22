# MathSolver2

A powerful mathematical expression parser, evaluator, and step-by-step calculator with support for various arithmetic modes and LaTeX notation.

## Overview

MathSolver2 is a C# library designed for parsing, evaluating, and displaying mathematical expressions with detailed step-by-step calculations. It supports various arithmetic operations, mathematical functions, and LaTeX-style mathematical notation.

## Features

- **Comprehensive Expression Parsing**: Parse complex mathematical expressions with support for:
  - Basic arithmetic operations (addition, subtraction, multiplication, division)
  - Functions (sin, cos, tan, log, ln, sqrt)
  - Exponentiation and factorial operations
  - Parentheses for grouping expressions
  - LaTeX notation support

- **Step-by-Step Solutions**: View detailed steps of calculations with explanations

- **Configurable Arithmetic Modes**:
  - Normal (no rounding)
  - Rounding to specified precision
  - Truncation to specified precision
  - Support for decimal places or significant digits

- **LaTeX Notation Support**:
  - Fractions: `\frac{a}{b}`
  - Summations: `\sum_{i=1}^{n} i`
  - Products: `\prod_{i=1}^{n} i`
  - Square roots: `\sqrt{x}`

- **Variable Substitution**: Use variables in expressions with custom values

- **Mathematical Constants**: Built-in support for π (pi), e, and φ (phi)

## Code Examples

### Basic Usage

```csharp
// Create a solver with default settings
var solver = new EnhancedMathSolver();

// Evaluate a simple expression
string expression = "2 * 3 + 4";
decimal result = solver.Evaluate(expression);
Console.WriteLine($"Result: {result}"); // Output: Result: 10
```

### Configuring Arithmetic Modes

```csharp
// Create a solver with specific arithmetic settings
// Round to 3 significant digits
var solver = new EnhancedMathSolver(
    arithmeticType: ArithmeticType.Round,
    precision: 3,
    useSignificantDigits: true);

// Evaluate an expression with e
CalculationResult result = solver.EvaluateWithSteps("e^0.15");
Console.WriteLine($"Result: {result.FormattedResult}");
Console.WriteLine($"Arithmetic Mode: {result.ArithmeticMode}");
Console.WriteLine($"Precision: {result.PrecisionInfo}");

// Print the calculation steps
foreach (var step in result.Steps)
{
    Console.WriteLine($"{step.Expression} => {step.Operation} => {step.Result}");
}
```

### Using Variables

```csharp
var solver = new EnhancedMathSolver();

// Define variables
solver.SetVariable("x", 10);
solver.SetVariable("y", 5);

// Use variables in expressions
decimal result = solver.Evaluate("x + y");
Console.WriteLine($"x + y = {result}"); // Output: x + y = 15
```

### LaTeX Notation

```csharp
var solver = new EnhancedMathSolver();

// Parse and evaluate a fraction
decimal result = solver.Evaluate("\\frac{1}{2} + \\frac{1}{3}");
Console.WriteLine($"Result: {result}"); // Output: Result: 0.8333333...

// Format expressions in LaTeX
string latex = solver.Format("(a + b)^2", OutputFormat.LaTeX);
Console.WriteLine(latex); // Output: (a + b)^{2}
```

## Project Structure

The codebase follows the visitor design pattern to separate the expression tree structure from the operations performed on it:

- **Node Classes**: 
  - `ExpressionNode` - Base class for all nodes
  - `OperatorNode` - Base class for binary operations
  - `FunctionNode` - Base class for function calls
  - Specific nodes for operations (AdditionNode, MultiplicationNode, etc.)

- **Visitors**:
  - `ArithmeticVisitor` - Evaluates expressions
  - `StepByStepArithmeticVisitor` - Evaluates with detailed steps
  - `FormattingVisitor` - Formats expressions as text

- **Core Components**:
  - `Tokenizer` - Breaks expressions into tokens
  - `ExpressionParser` - Builds an expression tree from tokens
  - `EnhancedMathSolver` - Main facade for using the library

- **Math Support**:
  - `MathFunctions` - Standard functions (sin, cos, etc.)
  - `MathConstants` - Standard constants (Pi, E, Phi)

## Example Demo

The included `Program.cs` demonstrates the library with a simple expression:

```csharp
// Test with Normal arithmetic (no formatting)
Console.WriteLine("Normal Arithmetic:");
DemoArithmeticMode(expression, ArithmeticType.Normal, 4, false);

// Test with Round to significant digits
Console.WriteLine("Round to 3 Significant Digits:");
DemoArithmeticMode(expression, ArithmeticType.Round, 3, true);
```

## Getting Started

### Prerequisites
- .NET SDK

### Running the Demo
1. Clone the repository
2. Navigate to the project directory
3. Run `dotnet run` to execute the demo in Program.cs

## Features to Be Added
- Symbolic differentiation and integration
- Equation solving
- Matrix operations
- User-defined functions
- Graphing capabilities

## License
This project is available under the MIT License.