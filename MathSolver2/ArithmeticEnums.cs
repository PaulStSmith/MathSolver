namespace MathSolver2;

/// <summary>
/// Type of arithmetic to use for calculations.
/// </summary>
public enum ArithmeticType
{
    /// <summary>
    /// No rounding or truncation.
    /// </summary>
    Normal,

    /// <summary>
    /// Truncate to specified precision.
    /// </summary>
    Truncate,

    /// <summary>
    /// Round to specified precision.
    /// </summary>
    Round
}

/// <summary>
/// Direction for evaluating expressions.
/// </summary>
public enum CalculationDirection
{
    /// <summary>
    /// Evaluate from left to right.
    /// </summary>
    LeftToRight,

    /// <summary>
    /// Evaluate from right to left.
    /// </summary>
    RightToLeft
}