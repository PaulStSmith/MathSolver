using System.Reflection;

namespace MathSolver2;

/// <summary>
/// Attribute to provide a description format for a math function.
/// </summary>
[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public class FunctionDescriptionAttribute : Attribute
{
    /// <summary>
    /// Gets the description format for the math function.
    /// </summary>
    public string DescriptionFormat { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="FunctionDescriptionAttribute"/> class.
    /// </summary>
    /// <param name="descriptionFormat">The description format for the function.</param>
    public FunctionDescriptionAttribute(string descriptionFormat)
    {
        DescriptionFormat = descriptionFormat;
    }
}

/// <summary>
/// Central repository of mathematical functions.
/// </summary>
public static class MathFunctions
{
    // Dictionary of function handlers
    private static readonly Dictionary<string, Func<decimal[], SourcePosition, decimal>> _functions
        = new Dictionary<string, Func<decimal[], SourcePosition, decimal>>(StringComparer.OrdinalIgnoreCase);

    // Dictionary of function descriptions
    private static readonly Dictionary<string, string> _descriptions
        = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

    // Static constructor to auto-register all functions using reflection
    static MathFunctions()
    {
        RegisterAllFunctions();
    }

    /// <summary>
    /// Registers all methods decorated with the <see cref="FunctionDescriptionAttribute"/>.
    /// </summary>
    private static void RegisterAllFunctions()
    {
        Type type = typeof(MathFunctions);
        MethodInfo[] methods = type.GetMethods(BindingFlags.Public | BindingFlags.Static);

        foreach (var method in methods)
        {
            // Look for the FunctionDescription attribute
            var attr = method.GetCustomAttribute<FunctionDescriptionAttribute>();
            if (attr == null) continue;

            // Verify the method has the correct signature
            var parameters = method.GetParameters();
            if (parameters.Length != 2 ||
                parameters[0].ParameterType != typeof(decimal[]) ||
                parameters[1].ParameterType != typeof(SourcePosition) ||
                method.ReturnType != typeof(decimal))
            {
                throw new InvalidOperationException(
                    $"Function method {method.Name} must have signature: decimal Method(decimal[] args, SourcePosition position)");
            }

            // Create a delegate and register the function
            var function = (Func<decimal[], SourcePosition, decimal>)method.CreateDelegate(
                typeof(Func<decimal[], SourcePosition, decimal>));

            string functionName = method.Name.ToLowerInvariant();
            _functions[functionName] = function;
            _descriptions[functionName] = attr.DescriptionFormat;
        }
    }

    /// <summary>
    /// Checks if a function with the specified name exists.
    /// </summary>
    /// <param name="functionName">The name of the function to check.</param>
    /// <returns><c>true</c> if the function exists; otherwise, <c>false</c>.</returns>
    public static bool Exists(string functionName)
    {
        return _functions.ContainsKey(functionName);
    }

    /// <summary>
    /// Evaluates a function by its name.
    /// </summary>
    /// <param name="functionName">The name of the function to evaluate.</param>
    /// <param name="args">The arguments to pass to the function.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The result of the function evaluation.</returns>
    /// <exception cref="EvaluationException">Thrown if the function is not supported.</exception>
    public static decimal Evaluate(string functionName, decimal[] args, SourcePosition position)
    {
        if (_functions.TryGetValue(functionName, out var function))
        {
            return function(args, position);
        }

        throw new EvaluationException($"Unsupported function: {functionName}", position);
    }

    /// <summary>
    /// Gets the description format for a function by its name.
    /// </summary>
    /// <param name="functionName">The name of the function.</param>
    /// <returns>The description format of the function.</returns>
    public static string GetDescriptionFormat(string functionName)
    {
        if (_descriptions.TryGetValue(functionName, out var description))
        {
            return description;
        }

        return $"Calculate {functionName} function";
    }

    /// <summary>
    /// Calculates the sine of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value in radians.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The sine of the specified value.</returns>
    [FunctionDescription("Calculate sine of {0}")]
    public static decimal Sin(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("sin", args.Length, 1, position);
        return (decimal)Math.Sin((double)args[0]);
    }

    /// <summary>
    /// Calculates the cosine of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value in radians.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The cosine of the specified value.</returns>
    [FunctionDescription("Calculate cosine of {0}")]
    public static decimal Cos(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("cos", args.Length, 1, position);
        return (decimal)Math.Cos((double)args[0]);
    }

    /// <summary>
    /// Calculates the tangent of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value in radians.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The tangent of the specified value.</returns>
    [FunctionDescription("Calculate tangent of {0}")]
    public static decimal Tan(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("tan", args.Length, 1, position);
        return (decimal)Math.Tan((double)args[0]);
    }

    /// <summary>
    /// Calculates the square root of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The square root of the specified value.</returns>
    /// <exception cref="EvaluationException">Thrown if the value is negative.</exception>
    [FunctionDescription("Calculate square root of {0}")]
    public static decimal Sqrt(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("sqrt", args.Length, 1, position);
        if (args[0] < 0)
        {
            throw new EvaluationException("Cannot take square root of a negative number", position);
        }
        return (decimal)Math.Sqrt((double)args[0]);
    }

    /// <summary>
    /// Calculates the base-10 logarithm of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The base-10 logarithm of the specified value.</returns>
    /// <exception cref="EvaluationException">Thrown if the value is non-positive.</exception>
    [FunctionDescription("Calculate base-10 logarithm of {0}")]
    public static decimal Log(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("log", args.Length, 1, position);
        if (args[0] <= 0)
        {
            throw new EvaluationException("Cannot take logarithm of a non-positive number", position);
        }
        return (decimal)Math.Log10((double)args[0]);
    }

    /// <summary>
    /// Calculates the natural logarithm of the specified value.
    /// </summary>
    /// <param name="args">An array containing one argument: the value.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <returns>The natural logarithm of the specified value.</returns>
    /// <exception cref="EvaluationException">Thrown if the value is non-positive.</exception>
    [FunctionDescription("Calculate natural logarithm of {0}")]
    public static decimal Ln(decimal[] args, SourcePosition position)
    {
        CheckArgumentCount("ln", args.Length, 1, position);
        if (args[0] <= 0)
        {
            throw new EvaluationException("Cannot take natural logarithm of a non-positive number", position);
        }
        return (decimal)Math.Log((double)args[0]);
    }

    /// <summary>
    /// Validates the argument count for a function.
    /// </summary>
    /// <param name="functionName">The name of the function.</param>
    /// <param name="actualCount">The actual number of arguments provided.</param>
    /// <param name="expectedCount">The expected number of arguments.</param>
    /// <param name="position">The source position for error reporting.</param>
    /// <exception cref="EvaluationException">Thrown if the argument count does not match the expected count.</exception>
    private static void CheckArgumentCount(string functionName, int actualCount, int expectedCount, SourcePosition position)
    {
        if (actualCount != expectedCount)
        {
            throw new EvaluationException(
                $"Function {functionName} expects {expectedCount} argument(s), got {actualCount}",
                position);
        }
    }
}
