namespace MathSolver2;

/// <summary>
/// Exception thrown during evaluation
/// </summary>
public class EvaluationException : Exception
{
    /// <summary>
    /// Gets the position in the source where the exception occurred.
    /// </summary>
    public SourcePosition Position { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="EvaluationException"/> class with a specified error message and position.
    /// </summary>
    /// <param name="message">The message that describes the error.</param>
    /// <param name="position">The position in the source where the error occurred.</param>
    public EvaluationException(string message, SourcePosition position)
        : base(message)
    {
        Position = position;
    }

    /// <summary>
    /// Returns a string that represents the current object.
    /// </summary>
    /// <returns>A string that represents the current object.</returns>
    public override string ToString()
    {
        return $"{Message} at {Position}";
    }
}