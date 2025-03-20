namespace MathSolver2
{
    /// <summary>
    /// Result of a step-by-step evaluation containing the final value and all steps
    /// </summary>
    public class StepByStepResult
    {
        public decimal Value { get; }
        public List<CalculationStep> Steps { get; }

        public StepByStepResult(decimal value, List<CalculationStep> steps)
        {
            Value = value;
            Steps = steps ?? new List<CalculationStep>();
        }
    }
}