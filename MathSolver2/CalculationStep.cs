namespace MathSolver2;

/// <summary>
/// Represents a single step in a calculation.
/// </summary>
public class CalculationStep
{
    /// <summary>
    /// Gets the expression before this step.
    /// </summary>
    public string Expression { get; }

    /// <summary>
    /// Gets the operation performed in this step.
    /// </summary>
    public string Operation { get; }

    /// <summary>
    /// Gets the expression after this step.
    /// </summary>
    public string Result { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="CalculationStep"/> class.
    /// </summary>
    /// <param name="expression">The expression before this step.</param>
    /// <param name="operation">The operation performed in this step.</param>
    /// <param name="result">The expression after this step.</param>
    public CalculationStep(string expression, string operation, string result)
    {
        Expression = expression;
        Operation = operation;
        Result = result;
    }

    /// <summary>
    /// Returns a string that represents the current object.
    /// </summary>
    /// <returns>A string that represents the current object.</returns>
    public override string ToString()
    {
        return $"{Expression} => {Operation} => {Result}";
    }
}