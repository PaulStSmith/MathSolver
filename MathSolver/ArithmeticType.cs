namespace MathSolver
{
    /// <summary>
    /// Represents the different arithmetic modes available for calculation results.
    /// </summary>
    public enum ArithmeticType
    {
        /// <summary>
        /// Normal arithmetic mode without any modifications.
        /// </summary>
        Normal,

        /// <summary>
        /// Arithmetic mode that truncates the result, removing any fractional part.
        /// </summary>
        Truncate,

        /// <summary>
        /// Arithmetic mode that rounds the result to the nearest whole number.
        /// </summary>
        Round
    }
}