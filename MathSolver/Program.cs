using System;
using System.Text.RegularExpressions;
using System.Data;

namespace MathSolver
{
    /// <summary>
    /// Arithmetic modes available for calculation results
    /// </summary>
    public enum ArithmeticType
    {
        Normal,
        Truncate,
        Round
    }

    /// <summary>
    /// A solver for LaTeX mathematical expressions with configurable arithmetic
    /// </summary>
    public class LatexMathSolver
    {
        private ArithmeticType _arithmeticType;
        private int _precision;
        private bool _useSignificantDigits;

        /// <summary>
        /// Creates a new LaTeX math solver with the specified arithmetic settings
        /// </summary>
        /// <param name="arithmeticType">Type of arithmetic to use</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        public LatexMathSolver(ArithmeticType arithmeticType = ArithmeticType.Normal,
                              int precision = 10,
                              bool useSignificantDigits = false)
        {
            SetArithmeticMode(arithmeticType, precision, useSignificantDigits);
        }

        /// <summary>
        /// Sets the arithmetic mode for the solver
        /// </summary>
        /// <param name="type">Type of arithmetic to use</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        public void SetArithmeticMode(ArithmeticType type, int precision, bool useSignificantDigits)
        {
            _arithmeticType = type;
            _precision = precision;
            _useSignificantDigits = useSignificantDigits;
        }

        /// <summary>
        /// Converts a LaTeX string to a format that can be evaluated
        /// </summary>
        /// <param name="latexString">The LaTeX expression to convert</param>
        /// <returns>A string that can be evaluated</returns>
        private string ConvertLatexToCalculable(string latexString)
        {
            // For simple numbers, just return them directly
            if (double.TryParse(latexString, out _))
            {
                return latexString;
            }

            // Handle more complex LaTeX expressions
            string expr = latexString.Trim();

            // Replace LaTeX commands with C# equivalents
            expr = Regex.Replace(expr, @"\\frac\{([^{}]*)\}\{([^{}]*)\}", "($1)/($2)");
            expr = expr.Replace("\\cdot", "*");
            expr = expr.Replace("\\times", "*");
            expr = expr.Replace("\\div", "/");
            expr = Regex.Replace(expr, @"\\sqrt\{([^{}]*)\}", "Math.Sqrt($1)");
            expr = expr.Replace("\\pi", "Math.PI");
            expr = Regex.Replace(expr, @"\^", "**");

            // This is a simplified conversion - a real implementation would 
            // need a more robust LaTeX parser for complex expressions

            return expr;
        }

        /// <summary>
        /// Truncates a number to a specific number of decimal places
        /// </summary>
        /// <param name="value">The number to truncate</param>
        /// <param name="decimalPlaces">Number of decimal places to keep</param>
        /// <returns>The truncated number</returns>
        private double TruncateToDecimalPlaces(double value, int decimalPlaces)
        {
            double multiplier = Math.Pow(10, decimalPlaces);
            return Math.Truncate(value * multiplier) / multiplier;
        }

        /// <summary>
        /// Truncates a number to a specific number of significant digits
        /// </summary>
        /// <param name="value">The number to truncate</param>
        /// <param name="sigDigits">Number of significant digits to keep</param>
        /// <returns>The truncated number</returns>
        private double TruncateToSignificantDigits(double value, int sigDigits)
        {
            if (Math.Abs(value) < double.Epsilon)
            {
                return 0;
            }

            double absValue = Math.Abs(value);
            int sign = value < 0 ? -1 : 1;

            // Find the magnitude of the number (position of the first significant digit)
            int magnitude = (int)Math.Floor(Math.Log10(absValue)) + 1;

            // Calculate the appropriate scaling factor
            double scale = Math.Pow(10, sigDigits - magnitude);

            // Scale, truncate, and scale back
            return sign * Math.Truncate(absValue * scale) / scale;
        }

        /// <summary>
        /// Rounds a number to a specific number of significant digits
        /// </summary>
        /// <param name="value">The number to round</param>
        /// <param name="sigDigits">Number of significant digits to keep</param>
        /// <returns>The rounded number</returns>
        private double RoundToSignificantDigits(double value, int sigDigits)
        {
            if (Math.Abs(value) < double.Epsilon)
            {
                return 0;
            }

            double absValue = Math.Abs(value);
            int sign = value < 0 ? -1 : 1;

            // Find the magnitude of the number
            int magnitude = (int)Math.Floor(Math.Log10(absValue)) + 1;

            // Calculate the appropriate scaling factor
            double scale = Math.Pow(10, sigDigits - magnitude);

            // Scale, round, and scale back
            return sign * Math.Round(absValue * scale, MidpointRounding.AwayFromZero) / scale;
        }

        /// <summary>
        /// Formats a number according to the configured arithmetic settings
        /// </summary>
        /// <param name="value">The number to format</param>
        /// <returns>The formatted number</returns>
        private double FormatNumber(double value)
        {
            if (double.IsNaN(value) || double.IsInfinity(value))
            {
                return value;
            }

            if (_arithmeticType == ArithmeticType.Normal)
            {
                return value;
            }

            if (_useSignificantDigits)
            {
                // Handle significant digits
                if (_arithmeticType == ArithmeticType.Truncate)
                {
                    return TruncateToSignificantDigits(value, _precision);
                }
                else // Round
                {
                    return RoundToSignificantDigits(value, _precision);
                }
            }
            else
            {
                // Handle decimal places
                if (_arithmeticType == ArithmeticType.Truncate)
                {
                    return TruncateToDecimalPlaces(value, _precision);
                }
                else // Round
                {
                    return Math.Round(value, _precision, MidpointRounding.AwayFromZero);
                }
            }
        }

        /// <summary>
        /// Evaluates a simple numerical expression
        /// </summary>
        /// <param name="expression">Expression to evaluate</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluateExpression(string expression)
        {
            // For simple numerical values, parse directly
            if (double.TryParse(expression, out double result))
            {
                return result;
            }

            // For expressions like basic fractions - 
            // Note: This is a very simplified evaluator for demonstration
            if (expression.Contains("/"))
            {
                string[] parts = expression.Split('/');
                if (parts.Length == 2 && double.TryParse(parts[0], out double numerator) &&
                    double.TryParse(parts[1], out double denominator))
                {
                    if (Math.Abs(denominator) < double.Epsilon)
                    {
                        throw new DivideByZeroException("Cannot divide by zero");
                    }
                    return numerator / denominator;
                }
            }

            // For more complex expressions, we would need a proper expression evaluator
            // For this demonstration, we'll use a DataTable evaluator for simple expressions
            try
            {
                DataTable table = new DataTable();
                result = Convert.ToDouble(table.Compute(expression, ""));
                return result;
            }
            catch (Exception ex)
            {
                throw new Exception($"Error evaluating expression: {ex.Message}. " +
                    "For complex expressions, please use a proper math expression parser library.");
            }
        }

        /// <summary>
        /// Solves a LaTeX math expression
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>The result of evaluating the expression</returns>
        public double Solve(string latexExpression)
        {
            try
            {
                // First, handle direct numbers (already in decimal form)
                if (double.TryParse(latexExpression, out double directValue))
                {
                    return FormatNumber(directValue);
                }

                // Try to convert and evaluate the expression
                string calculableExpr = ConvertLatexToCalculable(latexExpression);
                double result = EvaluateExpression(calculableExpr);

                // Format the result according to settings
                return FormatNumber(result);
            }
            catch (Exception ex)
            {
                throw new Exception($"Error solving expression: {ex.Message}");
            }
        }

        /// <summary>
        /// Solves a LaTeX math expression and returns the result as a string
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>The result of evaluating the expression as a string</returns>
        public string SolveToString(string latexExpression)
        {
            try
            {
                double result = Solve(latexExpression);

                if (double.IsNaN(result))
                {
                    return "Error: Invalid expression or not a number";
                }

                if (double.IsInfinity(result))
                {
                    return result > 0 ? "Infinity" : "-Infinity";
                }

                // Format the string representation appropriately
                if (_useSignificantDigits)
                {
                    return result.ToString($"G{_precision}");
                }
                else
                {
                    string formatted = result.ToString($"F{_precision}");
                    // Trim trailing zeros and decimal point if appropriate
                    return formatted.TrimEnd('0').TrimEnd('.');
                }
            }
            catch (Exception ex)
            {
                return $"Error: {ex.Message}";
            }
        }
    }

    /// <summary>
    /// Example usage of the LaTeX Math Solver
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            string expression = "1.02500125";
            Console.WriteLine($"Expression: {expression}\n");

            // Normal arithmetic (full precision)
            var normalSolver = new LatexMathSolver(ArithmeticType.Normal);
            Console.WriteLine($"Normal Arithmetic: {normalSolver.SolveToString(expression)}");

            // Truncate to 3 decimal places
            var truncateDecimalSolver = new LatexMathSolver(ArithmeticType.Truncate, 3, false);
            Console.WriteLine($"Truncate Arithmetic, 3 decimal places: {truncateDecimalSolver.SolveToString(expression)}");

            // Truncate to 3 significant digits
            var truncateSigDigitsSolver = new LatexMathSolver(ArithmeticType.Truncate, 3, true);
            Console.WriteLine($"Truncate Arithmetic, 3 significant digits: {truncateSigDigitsSolver.SolveToString(expression)}");

            // Round to 3 decimal places
            var roundDecimalSolver = new LatexMathSolver(ArithmeticType.Round, 3, false);
            Console.WriteLine($"Rounded Arithmetic, 3 decimal places: {roundDecimalSolver.SolveToString(expression)}");

            // Round to 3 significant digits
            var roundSigDigitsSolver = new LatexMathSolver(ArithmeticType.Round, 3, true);
            Console.WriteLine($"Rounded Arithmetic, 3 significant digits: {roundSigDigitsSolver.SolveToString(expression)}");

            // Try with a simple fraction
            Console.WriteLine("\n=== Simple Fraction Example ===");
            string fractionExpr = "\\frac{2}{3}";
            Console.WriteLine($"Expression: {fractionExpr}");

            try
            {
                Console.WriteLine($"Normal Arithmetic: {normalSolver.SolveToString(fractionExpr)}");
                Console.WriteLine($"Truncate (3 decimal places): {truncateDecimalSolver.SolveToString(fractionExpr)}");
                Console.WriteLine($"Round (3 significant digits): {roundSigDigitsSolver.SolveToString(fractionExpr)}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Note: {ex.Message}");
                Console.WriteLine("For full expression support, please integrate a proper math expression parser library.");
            }

            Console.ReadLine();
        }
    }
}