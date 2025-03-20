namespace MathSolver2
{
    /// <summary>
    /// Exception thrown during parsing when an error is encountered
    /// </summary>
    public class ParserException : Exception
    {
        public SourcePosition Position { get; }

        public ParserException(string message, SourcePosition position)
            : base(message)
        {
            Position = position;
        }

        public override string ToString()
        {
            return $"{Message} at {Position}";
        }
    }
}