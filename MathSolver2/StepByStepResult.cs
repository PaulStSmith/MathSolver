namespace MathSolver2;

/// <summary>
/// Represents the result of a step-by-step evaluation, including the final value and all calculation steps.
/// </summary>
public class StepByStepResult
{
    /// <summary>
    /// Gets the final calculated value.
    /// </summary>
    public decimal Value { get; }

    /// <summary>
    /// Gets the list of steps involved in the calculation.
    /// </summary>
    public List<CalculationStep> Steps { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="StepByStepResult"/> class.
    /// </summary>
    /// <param name="value">The final calculated value.</param>
    /// <param name="steps">The list of calculation steps. If null, an empty list is used.</param>
    public StepByStepResult(decimal value, List<CalculationStep> steps)
    {
        Value = value;
        Steps = steps ?? new List<CalculationStep>();
    }
}