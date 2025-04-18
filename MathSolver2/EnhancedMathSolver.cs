﻿namespace MathSolver2;

/// <summary>
/// Enhanced facade class for parsing and evaluating mathematical expressions
/// with configurable arithmetic modes
/// </summary>
public class EnhancedMathSolver
{
    private readonly Dictionary<string, decimal> _variables;
    private ArithmeticType _arithmeticType;
    private int _precision;
    private bool _useSignificantDigits;
    private CalculationDirection _direction;

    /// <summary>
    /// Creates a new instance of the MathSolver with configurable arithmetic modes
    /// </summary>
    /// <param name="arithmeticType">The type of arithmetic to use</param>
    /// <param name="precision">The precision in decimal places or significant digits</param>
    /// <param name="useSignificantDigits">Whether to use significant digits instead of decimal places</param>
    /// <param name="direction">The direction for evaluating expressions</param>
    public EnhancedMathSolver(
        ArithmeticType arithmeticType = ArithmeticType.Normal,
        int precision = 10,
        bool useSignificantDigits = false,
        CalculationDirection direction = CalculationDirection.LeftToRight)
    {
        _variables = new Dictionary<string, decimal>();
        _arithmeticType = arithmeticType;
        _precision = precision;
        _useSignificantDigits = useSignificantDigits;
        _direction = direction;
    }

    /// <summary>
    /// Sets the arithmetic mode
    /// </summary>
    /// <param name="type">The type of arithmetic to use</param>
    /// <param name="precision">The precision in decimal places or significant digits</param>
    /// <param name="useSignificantDigits">Whether to use significant digits instead of decimal places</param>
    public void SetArithmeticMode(ArithmeticType type, int precision, bool useSignificantDigits)
    {
        _arithmeticType = type;
        _precision = precision;
        _useSignificantDigits = useSignificantDigits;
    }

    /// <summary>
    /// Sets the calculation direction
    /// </summary>
    /// <param name="direction">The direction for evaluating expressions</param>
    public void SetCalculationDirection(CalculationDirection direction)
    {
        _direction = direction;
    }

    /// <summary>
    /// Sets a variable value
    /// </summary>
    /// <param name="name">The name of the variable</param>
    /// <param name="value">The value of the variable</param>
    public void SetVariable(string name, decimal value)
    {
        _variables[name] = value;
    }

    /// <summary>
    /// Gets a variable value
    /// </summary>
    /// <param name="name">The name of the variable</param>
    /// <returns>The value of the variable</returns>
    /// <exception cref="ArgumentException">Thrown when the variable is not defined</exception>
    public decimal GetVariable(string name)
    {
        if (_variables.TryGetValue(name, out var value)) // Changed to var
        {
            return value;
        }

        throw new ArgumentException($"Variable '{name}' is not defined");
    }

    /// <summary>
    /// Evaluates an expression and returns the result
    /// </summary>
    /// <param name="expression">The expression to evaluate</param>
    /// <returns>The result of the evaluation</returns>
    /// <exception cref="ArgumentException">Thrown when there is a parse or evaluation error</exception>
    public decimal Evaluate(string expression)
    {
        try
        {
            // Parse the expression
            var parser = new ExpressionParser(expression); // Changed to var
            var root = parser.Parse(); // Changed to var

            // Evaluate the expression with arithmetic settings
            var visitor = new ArithmeticVisitor( // Changed to var
                _variables, _arithmeticType, _precision, _useSignificantDigits);
            return visitor.Evaluate(root);
        }
        catch (ParserException ex)
        {
            throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
        }
        catch (EvaluationException ex)
        {
            throw new ArgumentException($"Evaluation error: {ex.Message} at position {ex.Position}");
        }
    }

    /// <summary>
    /// Evaluates an expression to a string
    /// </summary>
    /// <param name="expression">The expression to evaluate</param>
    /// <returns>The result of the evaluation as a string</returns>
    /// <exception cref="ArgumentException">Thrown when there is a parse or evaluation error</exception>
    public string EvaluateToString(string expression)
    {
        decimal result = Evaluate(expression);
        return FormatNumber(result).ToString();
    }

    /// <summary>
    /// Evaluates an expression step by step and returns all calculation steps with arithmetic formatting
    /// </summary>
    /// <param name="expression">The expression to evaluate</param>
    /// <returns>The result of the evaluation with steps</returns>
    /// <exception cref="ArgumentException">Thrown when there is a parse or evaluation error</exception>
    public CalculationResult EvaluateWithSteps(string expression)
    {
        try
        {
            // Parse the expression
            var parser = new ExpressionParser(expression); // Changed to var
            var root = parser.Parse(); // Changed to var

            // Evaluate the expression step by step with arithmetic settings
            var visitor = new StepByStepArithmeticVisitor( // Changed to var
                _variables, _arithmeticType, _precision, _useSignificantDigits);
            var result = visitor.Evaluate(root); // Changed to var

            // Create the result object
            var arithmeticMode = _arithmeticType.ToString(); // Changed to var
            var precisionInfo = _arithmeticType == ArithmeticType.Normal ? "Maximum" :
                _useSignificantDigits ?
                    $"{_precision} significant digits" :
                    $"{_precision} decimal places";

            return new CalculationResult
            {
                OriginalExpression = expression,
                ArithmeticMode = arithmeticMode,
                PrecisionInfo = precisionInfo,
                Direction = _direction.ToString(),
                ActualResult = result.Value,
                FormattedResult = FormatNumber(result.Value),
                Steps = result.Steps
            };
        }
        catch (ParserException ex)
        {
            throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
        }
        catch (EvaluationException ex)
        {
            throw new ArgumentException($"Evaluation error: {ex.Message} at position {ex.Position}");
        }
    }

    /// <summary>
    /// Formats an expression in standard or LaTeX notation
    /// </summary>
    /// <param name="expression">The expression to format</param>
    /// <param name="format">The output format (Standard or LaTeX)</param>
    /// <returns>The formatted expression</returns>
    /// <exception cref="ArgumentException">Thrown when there is a parse error</exception>
    public string Format(string expression, OutputFormat format = OutputFormat.Standard)
    {
        try
        {
            // Parse the expression
            var parser = new ExpressionParser(expression); // Changed to var
            var root = parser.Parse(); // Changed to var

            // Format the expression
            var visitor = new FormattingVisitor(format); // Changed to var
            return visitor.Format(root);
        }
        catch (ParserException ex)
        {
            throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
        }
    }

    /// <summary>
    /// Validates if an expression can be parsed without errors
    /// </summary>
    /// <param name="expression">The expression to validate</param>
    /// <param name="error">The error message if validation fails</param>
    /// <returns>True if the expression is valid, otherwise false</returns>
    public bool Validate(string expression, out string error)
    {
        try
        {
            var parser = new ExpressionParser(expression); // Changed to var
            parser.Parse();

            error = null;
            return true;
        }
        catch (ParserException ex)
        {
            error = $"Parse error: {ex.Message} at position {ex.Position}";
            return false;
        }
    }

    /// <summary>
    /// Formats a number according to the specified arithmetic settings
    /// </summary>
    /// <param name="value">The value to format</param>
    /// <returns>The formatted value</returns>
    private decimal FormatNumber(decimal value)
    {
        switch (_arithmeticType)
        {
            case ArithmeticType.Normal:
                return value;

            case ArithmeticType.Truncate:
                if (_useSignificantDigits)
                {
                    return TruncateToSignificantDigits(value, _precision);
                }
                else
                {
                    return TruncateToDecimalPlaces(value, _precision);
                }

            case ArithmeticType.Round:
                if (_useSignificantDigits)
                {
                    return RoundToSignificantDigits(value, _precision);
                }
                else
                {
                    return Math.Round(value, _precision);
                }

            default:
                return value;
        }
    }

    /// <summary>
    /// Truncates a decimal value to the specified number of decimal places
    /// </summary>
    /// <param name="value">The value to truncate</param>
    /// <param name="decimalPlaces">The number of decimal places</param>
    /// <returns>The truncated value</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when decimalPlaces is negative</exception>
    private decimal TruncateToDecimalPlaces(decimal value, int decimalPlaces)
    {
        if (decimalPlaces < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(decimalPlaces), "Decimal places must be non-negative");
        }

        var multiplier = (decimal)Math.Pow(10, decimalPlaces); // Changed to var

        if (value >= 0)
        {
            return Math.Floor(value * multiplier) / multiplier;
        }
        else
        {
            return Math.Ceiling(value * multiplier) / multiplier;
        }
    }

    /// <summary>
    /// Truncates a decimal value to the specified number of significant digits
    /// </summary>
    /// <param name="value">The value to truncate</param>
    /// <param name="sigDigits">The number of significant digits</param>
    /// <returns>The truncated value</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive</exception>
    private decimal TruncateToSignificantDigits(decimal value, int sigDigits)
    {
        if (sigDigits <= 0)
        {
            throw new ArgumentOutOfRangeException(nameof(sigDigits), "Significant digits must be positive");
        }

        if (value == 0)
        {
            return 0;
        }

        // Get the exponent (power of 10) of the value
        var exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value))); // Changed to var

        // Calculate the number of decimal places needed
        var decimalPlaces = sigDigits - exponent - 1; // Changed to var

        // Adjust for small numbers (value < 1)
        if (value != 0 && Math.Abs(value) < 1)
        {
            decimalPlaces = sigDigits + Math.Abs(exponent + 1);
        }

        return TruncateToDecimalPlaces(value, Math.Max(0, decimalPlaces));
    }

    /// <summary>
    /// Rounds a decimal value to the specified number of significant digits
    /// </summary>
    /// <param name="value">The value to round</param>
    /// <param name="sigDigits">The number of significant digits</param>
    /// <returns>The rounded value</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive</exception>
    private decimal RoundToSignificantDigits(decimal value, int sigDigits)
    {
        if (sigDigits <= 0)
        {
            throw new ArgumentOutOfRangeException(nameof(sigDigits), "Significant digits must be positive");
        }

        if (value == 0)
        {
            return 0;
        }

        // Get the exponent (power of 10) of the value
        var exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value))); // Changed to var

        // Calculate the number of decimal places needed
        var decimalPlaces = sigDigits - exponent - 1; // Changed to var

        // Adjust for small numbers (value < 1)
        if (value != 0 && Math.Abs(value) < 1)
        {
            decimalPlaces = sigDigits + Math.Abs(exponent + 1);
        }

        return Math.Round(value, Math.Max(0, decimalPlaces));
    }
}

/// <summary>
/// Result of a calculation with steps
/// </summary>
public class CalculationResult
{
    /// <summary>
    /// The original expression
    /// </summary>
    public string OriginalExpression { get; set; }

    /// <summary>
    /// The arithmetic mode used
    /// </summary>
    public string ArithmeticMode { get; set; }

    /// <summary>
    /// Information about the precision used
    /// </summary>
    public string PrecisionInfo { get; set; }

    /// <summary>
    /// The calculation direction used
    /// </summary>
    public string Direction { get; set; }

    /// <summary>
    /// The actual unformatted result
    /// </summary>
    public decimal ActualResult { get; set; }

    /// <summary>
    /// The formatted result according to arithmetic settings
    /// </summary>
    public decimal FormattedResult { get; set; }

    /// <summary>
    /// The steps taken in the calculation
    /// </summary>
    public List<CalculationStep> Steps { get; set; }
}