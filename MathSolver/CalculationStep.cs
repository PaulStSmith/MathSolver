namespace MathSolver
{
    /// <summary>
    /// Represents a single step in a calculation.
    /// </summary>
    public class CalculationStep
    {
        /// <summary>
        /// Gets or sets the expression for this calculation step.
        /// </summary>
        public string Expression { get; set; }

        /// <summary>
        /// Gets or sets the result of this calculation step.
        /// </summary>
        public double Result { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="CalculationStep"/> class with the specified expression and result.
        /// </summary>
        /// <param name="expression">The expression for this calculation step.</param>
        /// <param name="result">The result of this calculation step.</param>
        public CalculationStep(string expression, double result)
        {
            Expression = expression;
            Result = result;
        }

        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{Expression} = {Result}";
        }
    }
}