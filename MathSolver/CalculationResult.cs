using System.Text;

namespace MathSolver
{
    /// <summary>
    /// Contains detailed information about a calculation, including all steps.
    /// </summary>
    public class CalculationResult
    {
        /// <summary>
        /// Gets or sets the original expression that was calculated.
        /// </summary>
        public string? OriginalExpression { get; set; }

        /// <summary>
        /// Gets or sets the arithmetic mode used for the calculation.
        /// </summary>
        public string? ArithmeticMode { get; set; }

        /// <summary>
        /// Gets or sets the precision information for the calculation.
        /// </summary>
        public string? PrecisionInfo { get; set; }

        /// <summary>
        /// Gets or sets the direction in which the calculation was performed.
        /// </summary>
        public string? Direction { get; set; }

        /// <summary>
        /// Gets or sets the list of steps involved in the calculation.
        /// </summary>
        public List<CalculationStep> Steps { get; set; } = new List<CalculationStep>();

        /// <summary>
        /// Gets or sets the actual result of the calculation.
        /// </summary>
        public double ActualResult { get; set; }

        /// <summary>
        /// Gets or sets the formatted result of the calculation.
        /// </summary>
        public double FormattedResult { get; set; }

        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.AppendLine($"Solving   : {OriginalExpression}");
            sb.AppendLine($"Mode      : {ArithmeticMode}");
            sb.AppendLine($"Precision : {PrecisionInfo}");
            sb.AppendLine($"Direction : {Direction}");
            sb.AppendLine("Start Calculation");

            for (int i = 0; i < Steps.Count; i++)
            {
                sb.AppendLine($"* Step {i + 1}: {Steps[i]}");
            }

            sb.AppendLine("End Calculation");
            sb.AppendLine();

            sb.AppendLine($"{ArithmeticMode}d Result: {FormattedResult}");
            sb.AppendLine($"Actual Result: {ActualResult}");

            // Calculate the error between the actual and formatted result
            double error = Math.Abs(FormattedResult - ActualResult);
            sb.AppendLine($"Error: {error}");

            return sb.ToString();
        }
    }
}