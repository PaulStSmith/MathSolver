using System;

namespace MathSolver
{
    /// <summary>
    /// Handles arithmetic operations and number formatting based on configuration settings
    /// </summary>
    public class ArithmeticHandler
    {
        private readonly ArithmeticType _arithmeticType;
        private readonly int _precision;
        private readonly bool _useSignificantDigits;

        public ArithmeticType ArithmeticType => _arithmeticType;

        public int Precision => _precision;

        public bool UseSignificantDigits => _useSignificantDigits;

        public string PrecisionDescription => UseSignificantDigits ? $"{Precision} significant digits" : $"{Precision} decimal places";

        /// <summary>
        /// Creates a new arithmetic handler with specified formatting settings
        /// </summary>
        /// <param name="arithmeticType">Type of arithmetic to use (Normal, Truncate, Round)</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        public ArithmeticHandler(
            ArithmeticType arithmeticType = ArithmeticType.Normal,
            int precision = 10,
            bool useSignificantDigits = false)
        {
            _arithmeticType = arithmeticType;
            _precision = precision;
            _useSignificantDigits = useSignificantDigits;
        }

        /// <summary>
        /// Performs a binary operation and formats the result according to settings
        /// </summary>
        /// <param name="a">First operand</param>
        /// <param name="b">Second operand</param>
        /// <param name="operation">Operator symbol</param>
        /// <returns>Formatted result of the operation</returns>
        public double PerformOperation(double a, double b, string operation)
        {
            double result;

            switch (operation)
            {
                case "+":
                    result = a + b;
                    break;
                case "-":
                    result = a - b;
                    break;
                case "*":
                    result = a * b;
                    break;
                case "/":
                    if (Math.Abs(b) < double.Epsilon)
                    {
                        throw new DivideByZeroException("Cannot divide by zero");
                    }
                    result = a / b;
                    break;
                case "^":
                    result = Math.Pow(a, b);
                    break;
                default:
                    throw new ArgumentException($"Unsupported operator: {operation}");
            }

            return FormatNumber(result);
        }

        /// <summary>
        /// Calculates factorial of an integer
        /// </summary>
        /// <param name="n">The number to calculate factorial for</param>
        /// <returns>The factorial result</returns>
        public double Factorial(int n)
        {
            if (n < 0)
            {
                throw new ArgumentException("Factorial is not defined for negative numbers");
            }

            if (n == 0 || n == 1)
            {
                return 1;
            }

            double result = 1;
            for (int i = 2; i <= n; i++)
            {
                result *= i;
            }

            return FormatNumber(result);
        }

        /// <summary>
        /// Formats a number according to the configured arithmetic settings
        /// </summary>
        /// <param name="value">The number to format</param>
        /// <returns>The formatted number</returns>
        public double FormatNumber(double value)
        {
            if (double.IsNaN(value) || double.IsInfinity(value))
            {
                return value;
            }

            if (ArithmeticType == ArithmeticType.Normal)
            {
                return value;
            }

            if (UseSignificantDigits)
            {
                // Handle significant digits
                return ArithmeticType == ArithmeticType.Truncate
                    ? TruncateToSignificantDigits(value, Precision)
                    : RoundToSignificantDigits(value, Precision);
            }
            else
            {
                // Handle decimal places
                return ArithmeticType == ArithmeticType.Truncate
                    ? TruncateToDecimalPlaces(value, Precision)
                    : Math.Round(value, Precision, MidpointRounding.AwayFromZero);
            }
        }

        /// <summary>
        /// Formats a number as a string with appropriate representation
        /// </summary>
        /// <param name="value">The number to format</param>
        /// <returns>String representation of the number</returns>
        public string FormatNumberAsString(double value)
        {
            if (double.IsNaN(value))
            {
                return "Error: Not a number";
            }

            if (double.IsInfinity(value))
            {
                return value > 0 ? "Infinity" : "-Infinity";
            }

            // Format the string representation appropriately
            if (UseSignificantDigits)
            {
                return value.ToString($"G{Precision}");
            }
            else
            {
                string formatted = value.ToString($"F{Precision}");
                // Trim trailing zeros and decimal point if appropriate
                return formatted.TrimEnd('0').TrimEnd('.');
            }
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

            var absValue = Math.Abs(value);
            var sign = value < 0 ? -1 : 1;

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

            var absValue = Math.Abs(value);
            var sign = value < 0 ? -1 : 1;

            // Find the magnitude of the number
            int magnitude = (int)Math.Floor(Math.Log10(absValue)) + 1;

            // Calculate the appropriate scaling factor
            double scale = Math.Pow(10, sigDigits - magnitude);

            // Scale, round, and scale back
            return sign * Math.Round(absValue * scale, MidpointRounding.AwayFromZero) / scale;
        }
    }
}