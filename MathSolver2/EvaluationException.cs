namespace MathSolver2
{
    /// <summary>
    /// Exception thrown during evaluation
    /// </summary>
    public class EvaluationException : Exception
    {
        public SourcePosition Position { get; }

        public EvaluationException(string message, SourcePosition position)
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