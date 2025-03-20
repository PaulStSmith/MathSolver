namespace MathSolver2;

/// <summary>
/// Exception thrown during parsing when an error is encountered.
/// </summary>
public class ParserException : Exception
{
    /// <summary>
    /// Gets the position in the source where the error occurred.
    /// </summary>
    public SourcePosition Position { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="ParserException"/> class with a specified error message and source position.
    /// </summary>
    /// <param name="message">The error message that explains the reason for the exception.</param>
    /// <param name="position">The position in the source where the error occurred.</param>
    public ParserException(string message, SourcePosition position)
        : base(message)
    {
        Position = position;
    }

    /// <summary>
    /// Returns a string representation of the exception, including the error message and source position.
    /// </summary>
    /// <returns>A string that represents the exception.</returns>
    public override string ToString()
    {
        return $"{Message} at {Position}";
    }
}