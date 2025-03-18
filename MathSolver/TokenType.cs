namespace MathSolver
{
    /// <summary>
    /// Token types for expression parsing.
    /// </summary>
    public enum TokenType
    {
        /// <summary>
        /// Represents a numeric value.
        /// </summary>
        Number,

        /// <summary>
        /// Represents an operator (e.g., +, -, *, /).
        /// </summary>
        Operator,

        /// <summary>
        /// Represents a function (e.g., sin, cos).
        /// </summary>
        Function,

        /// <summary>
        /// Represents an open parenthesis.
        /// </summary>
        OpenParenthesis,

        /// <summary>
        /// Represents a close parenthesis.
        /// </summary>
        CloseParenthesis,

        /// <summary>
        /// Represents a summation expression.
        /// </summary>
        Summation,

        /// <summary>
        /// Represents a factorial operation.
        /// </summary>
        Factorial,

        Variable,
    }
}