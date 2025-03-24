namespace MathSolver2;

/// <summary>
/// Base class for arithmetic visitors containing shared functionality.
/// </summary>
public abstract class BaseArithmeticVisitor<T> : ILatexExpressionVisitor<T>
{
    /// <summary>
    /// Dictionary of variable names and their values.
    /// </summary>
    protected readonly Dictionary<string, decimal> Variables;

    /// <summary>
    /// Type of arithmetic to use for calculations.
    /// </summary>
    protected readonly ArithmeticType ArithmeticType;

    /// <summary>
    /// Precision to use for rounding or truncation.
    /// </summary>
    protected readonly int Precision;

    /// <summary>
    /// Whether to use significant digits for formatting.
    /// </summary>
    protected readonly bool UseSignificantDigits;

    /// <summary>
    /// Initializes a new instance of the <see cref="BaseArithmeticVisitor{T}"/> class.
    /// </summary>
    /// <param name="variables">Dictionary of variable names and their values.</param>
    /// <param name="arithmeticType">Type of arithmetic to use for calculations.</param>
    /// <param name="precision">Precision to use for rounding or truncation.</param>
    /// <param name="useSignificantDigits">Whether to use significant digits for formatting.</param>
    protected BaseArithmeticVisitor(
        Dictionary<string, decimal> variables,
        ArithmeticType arithmeticType,
        int precision,
        bool useSignificantDigits)
    {
        Variables = variables ?? new Dictionary<string, decimal>();
        ArithmeticType = arithmeticType;
        Precision = precision;
        UseSignificantDigits = useSignificantDigits;
    }

    /// <summary>
    /// Sets a variable value.
    /// </summary>
    /// <param name="name">The name of the variable.</param>
    /// <param name="value">The value to set for the variable.</param>
    public void SetVariable(string name, decimal value)
    {
        Variables[name] = value;
    }

    /// <summary>
    /// Evaluates the expression step by step and records calculations.
    /// </summary>
    /// <param name="node">The expression node to evaluate.</param>
    /// <returns>The evaluated result.</returns>
    public T Evaluate(IExpressionNode node)
    {
        return node.Accept(this);
    }

    /// <summary>
    /// Gets a variable value, checking both math constants and user-defined variables.
    /// </summary>
    /// <param name="name">The name of the variable.</param>
    /// <param name="value">The value of the variable.</param>
    /// <returns>True if the variable exists, otherwise false.</returns>
    protected bool TryGetVariableValue(string name, out decimal value)
    {
        // First check if it's a mathematical constant
        if (MathConstants.TryGetValue(name, out value))
        {
            value = FormatNumber(value);
            return true;
        }

        // Then check user-defined variables
        return Variables.TryGetValue(name, out value);
    }

    /// <summary>
    /// Formats a number according to the specified arithmetic settings.
    /// </summary>
    /// <param name="value">The value to format.</param>
    /// <returns>The formatted value.</returns>
    protected decimal FormatNumber(decimal value)
    {
        switch (ArithmeticType)
        {
            case ArithmeticType.Normal:
                return value;

            case ArithmeticType.Truncate:
                if (UseSignificantDigits)
                {
                    return TruncateToSignificantDigits(value, Precision);
                }
                else
                {
                    return TruncateToDecimalPlaces(value, Precision);
                }

            case ArithmeticType.Round:
                if (UseSignificantDigits)
                {
                    return RoundToSignificantDigits(value, Precision);
                }
                else
                {
                    return Math.Round(value, Precision);
                }

            default:
                return value;
        }
    }

    /// <summary>
    /// Gets a description of the current formatting applied.
    /// </summary>
    /// <returns>A description of the current formatting applied.</returns>
    protected string GetFormatInfo()
    {
        if (ArithmeticType == ArithmeticType.Normal)
        {
            return "with no formatting";
        }

        string action = ArithmeticType == ArithmeticType.Truncate ? "truncating" : "rounding";
        string precision = UseSignificantDigits ?
            $"{Precision} significant digit{(Precision > 1 ? "s" : "")}" :
            $"{Precision} decimal place{(Precision > 1 ? "s" : "")}";

        return $"{action} to {precision}";
    }

    /// <summary>
    /// Checks if the function node has the expected number of arguments.
    /// </summary>
    /// <param name="node">The function node to check.</param>
    /// <param name="expectedCount">The expected number of arguments.</param>
    /// <exception cref="EvaluationException">Thrown when the argument count does not match the expected count.</exception>
    protected void CheckArgumentCount(FunctionNode node, int expectedCount)
    {
        if (node.Arguments.Count != expectedCount)
        {
            throw new EvaluationException(
                $"Function {node.Name} expects {expectedCount} argument(s), got {node.Arguments.Count}",
                node.Position);
        }
    }

    /// <summary>
    /// Truncates a decimal value to the specified number of decimal places.
    /// </summary>
    /// <param name="value">The value to truncate.</param>
    /// <param name="decimalPlaces">The number of decimal places to truncate to.</param>
    /// <returns>The truncated value.</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when decimalPlaces is negative.</exception>
    protected decimal TruncateToDecimalPlaces(decimal value, int decimalPlaces)
    {
        if (decimalPlaces < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(decimalPlaces), "Decimal places must be non-negative");
        }

        var multiplier = (decimal)Math.Pow(10, decimalPlaces);

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
    /// Truncates a decimal value to the specified number of significant digits.
    /// </summary>
    /// <param name="value">The value to truncate.</param>
    /// <param name="sigDigits">The number of significant digits to truncate to.</param>
    /// <returns>The truncated value.</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive.</exception>
    protected decimal TruncateToSignificantDigits(decimal value, int sigDigits)
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
        var exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value)));

        // Calculate the number of decimal places needed
        var decimalPlaces = sigDigits - exponent - 1;

        // Adjust for small numbers (value < 1)
        if (value != 0 && Math.Abs(value) < 1)
        {
            decimalPlaces = sigDigits + Math.Abs(exponent + 1);
        }

        return TruncateToDecimalPlaces(value, Math.Max(0, decimalPlaces));
    }

    /// <summary>
    /// Rounds a decimal value to the specified number of significant digits.
    /// </summary>
    /// <param name="value">The value to round.</param>
    /// <param name="sigDigits">The number of significant digits to round to.</param>
    /// <returns>The rounded value.</returns>
    /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive.</exception>
    protected decimal RoundToSignificantDigits(decimal value, int sigDigits)
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
        var exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value)));

        // Calculate the number of decimal places needed
        var decimalPlaces = sigDigits - exponent - 1;

        // Adjust for small numbers (value < 1)
        if (value != 0 && Math.Abs(value) < 1)
        {
            decimalPlaces = sigDigits + Math.Abs(exponent + 1);
        }

        return Math.Round(value, Math.Max(0, decimalPlaces));
    }

    /// <summary>
    /// Evaluates a function and returns both the result and a description of the operation.
    /// </summary>
    /// <param name="functionName">The name of the function.</param>
    /// <param name="args">The arguments to pass to the function.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>A tuple containing the result and a description of the operation.</returns>
    protected (decimal Result, string Description) EvaluateFunction(
        string functionName,
        decimal[] args,
        SourcePosition position)
    {
        // Get the raw result
        var rawResult = MathFunctions.Evaluate(functionName, args, position);

        // Format the result according to arithmetic settings
        var formattedResult = FormatNumber(rawResult);

        // Get the operation description
        var descriptionFormat = MathFunctions.GetDescriptionFormat(functionName);
        string description;

        if (args.Length == 1)
        {
            description = string.Format(descriptionFormat, args[0]);
        }
        else if (args.Length > 1)
        {
            description = string.Format(descriptionFormat, (object[])args.Select(a => (object)a).ToArray());
        }
        else
        {
            description = descriptionFormat;
        }

        // Add the formatting information to the description
        var formatInfo = GetFormatInfo();
        description = $"{description}, {formatInfo}";

        return (formattedResult, description);
    }

    public abstract T VisitNumber(NumberNode node);
    public abstract T VisitVariable(VariableNode node);
    public abstract T VisitAddition(AdditionNode node);
    public abstract T VisitSubtraction(SubtractionNode node);
    public abstract T VisitMultiplication(MultiplicationNode node);
    public abstract T VisitDivision(DivisionNode node);
    public abstract T VisitExponent(ExponentNode node);
    public abstract T VisitParenthesis(ParenthesisNode node);
    public abstract T VisitFunction(FunctionNode node);
    public abstract T VisitFactorial(FactorialNode node);
    public abstract T VisitSummation(SummationNode node);
    public abstract T VisitProduct(ProductNode node);
}