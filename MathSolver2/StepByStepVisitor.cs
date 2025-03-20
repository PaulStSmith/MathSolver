namespace MathSolver2
{

    /// <summary>
    /// Evaluates an expression tree step by step and records each calculation
    /// </summary>
    [Obsolete("This class is obsolete and will be removed in a future version. Use the StepByStepArithmeticVisitor class instead.")]
    public class StepByStepVisitor : IExpressionVisitor<StepByStepResult>
    {
        private readonly Dictionary<string, decimal> _variables;
        private readonly FormattingVisitor _formatter;

        public StepByStepVisitor()
        {
            _variables = new Dictionary<string, decimal>();
            _formatter = new FormattingVisitor();
        }

        public StepByStepVisitor(Dictionary<string, decimal> variables)
        {
            _variables = variables ?? new Dictionary<string, decimal>();
            _formatter = new FormattingVisitor();
        }

        /// <summary>
        /// Sets a variable value
        /// </summary>
        public void SetVariable(string name, decimal value)
        {
            _variables[name] = value;
        }

        /// <summary>
        /// Evaluates the expression step by step and records calculations
        /// </summary>
        public StepByStepResult Evaluate(IExpressionNode node)
        {
            return node.Accept(this);
        }

        public StepByStepResult VisitNumber(NumberNode node)
        {
            return new StepByStepResult(node.Value, new List<CalculationStep>());
        }

        public StepByStepResult VisitVariable(VariableNode node)
        {
            if (_variables.TryGetValue(node.Name, out decimal value))
            {
                var steps = new List<CalculationStep>
                {
                    new CalculationStep(
                        node.Name,
                        $"Substitute variable {node.Name}",
                        value.ToString())
                };

                return new StepByStepResult(value, steps);
            }

            throw new EvaluationException($"Variable '{node.Name}' is not defined", node.Position);
        }

        public StepByStepResult VisitAddition(AdditionNode node)
        {
            var leftResult = node.Left.Accept(this);
            var rightResult = node.Right.Accept(this);

            var allSteps = new List<CalculationStep>(leftResult.Steps);
            allSteps.AddRange(rightResult.Steps);

            decimal result = leftResult.Value + rightResult.Value;

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Add {leftResult.Value} and {rightResult.Value}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitSubtraction(SubtractionNode node)
        {
            var leftResult = node.Left.Accept(this);
            var rightResult = node.Right.Accept(this);

            var allSteps = new List<CalculationStep>(leftResult.Steps);
            allSteps.AddRange(rightResult.Steps);

            decimal result = leftResult.Value - rightResult.Value;

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Subtract {rightResult.Value} from {leftResult.Value}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitMultiplication(MultiplicationNode node)
        {
            var leftResult = node.Left.Accept(this);
            var rightResult = node.Right.Accept(this);

            var allSteps = new List<CalculationStep>(leftResult.Steps);
            allSteps.AddRange(rightResult.Steps);

            decimal result = leftResult.Value * rightResult.Value;

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Multiply {leftResult.Value} by {rightResult.Value}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitDivision(DivisionNode node)
        {
            var numeratorResult = node.Numerator.Accept(this);
            var denominatorResult = node.Denominator.Accept(this);

            var allSteps = new List<CalculationStep>(numeratorResult.Steps);
            allSteps.AddRange(denominatorResult.Steps);

            if (denominatorResult.Value == 0)
            {
                throw new EvaluationException("Division by zero", node.Position);
            }

            decimal result = numeratorResult.Value / denominatorResult.Value;

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Divide {numeratorResult.Value} by {denominatorResult.Value}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitExponent(ExponentNode node)
        {
            var baseResult = node.Base.Accept(this);
            var exponentResult = node.Exponent.Accept(this);

            var allSteps = new List<CalculationStep>(baseResult.Steps);
            allSteps.AddRange(exponentResult.Steps);

            decimal result;
            string operation;

            // Handle special cases
            if (exponentResult.Value == 0)
            {
                result = 1;
                operation = $"Any number raised to power 0 is 1";
            }
            else if (baseResult.Value == 0)
            {
                result = 0;
                operation = $"0 raised to any non-zero power is 0";
            }
            else if (exponentResult.Value == 1)
            {
                result = baseResult.Value;
                operation = $"Any number raised to power 1 is the number itself";
            }
            else
            {
                // Check if exponent is an integer
                if (Math.Abs(exponentResult.Value - Math.Round(exponentResult.Value)) < Precision.Epsilon)
                {
                    int intExponent = (int)Math.Round(exponentResult.Value);
                    result = (decimal)Math.Pow((double)baseResult.Value, intExponent);
                }
                else
                {
                    // For non-integer exponents, use Math.Pow
                    result = (decimal)Math.Pow((double)baseResult.Value, (double)exponentResult.Value);
                }

                operation = $"Raise {baseResult.Value} to the power of {exponentResult.Value}";
            }

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(expression, operation, result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitParenthesis(ParenthesisNode node)
        {
            var innerResult = node.Expression.Accept(this);

            // Add a step showing the parentheses are being evaluated
            string expression = _formatter.Format(node);
            var allSteps = new List<CalculationStep>(innerResult.Steps);

            if (innerResult.Steps.Count > 0)
            {
                allSteps.Add(new CalculationStep(
                    expression,
                    "Evaluate parentheses",
                    innerResult.Value.ToString()));
            }

            return new StepByStepResult(innerResult.Value, allSteps);
        }

        public StepByStepResult VisitFunction(FunctionNode node)
        {
            // Evaluate all arguments
            var argResults = node.Arguments.Select(arg => arg.Accept(this)).ToList();

            // Collect all steps from arguments
            var allSteps = new List<CalculationStep>();
            foreach (var argResult in argResults)
            {
                allSteps.AddRange(argResult.Steps);
            }

            string functionName = node.Name.ToLower();
            decimal result;
            string operation;

            switch (functionName)
            {
                case "sin":
                    CheckArgumentCount(node, 1);
                    result = (decimal)Math.Sin((double)argResults[0].Value);
                    operation = $"Calculate sine of {argResults[0].Value}";
                    break;

                case "cos":
                    CheckArgumentCount(node, 1);
                    result = (decimal)Math.Cos((double)argResults[0].Value);
                    operation = $"Calculate cosine of {argResults[0].Value}";
                    break;

                case "tan":
                    CheckArgumentCount(node, 1);
                    result = (decimal)Math.Tan((double)argResults[0].Value);
                    operation = $"Calculate tangent of {argResults[0].Value}";
                    break;

                case "sqrt":
                    CheckArgumentCount(node, 1);
                    decimal arg = argResults[0].Value;
                    if (arg < 0)
                    {
                        throw new EvaluationException("Cannot take square root of a negative number", node.Position);
                    }
                    result = (decimal)Math.Sqrt((double)arg);
                    operation = $"Calculate square root of {arg}";
                    break;

                default:
                    throw new EvaluationException($"Unsupported function: {node.Name}", node.Position);
            }

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(expression, operation, result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        public StepByStepResult VisitFactorial(FactorialNode node)
        {
            var innerResult = node.Expression.Accept(this);
            var allSteps = new List<CalculationStep>(innerResult.Steps);

            decimal value = innerResult.Value;

            // Check if value is a non-negative integer
            if (value < 0 || Math.Abs(value - Math.Round(value)) > Precision.Epsilon)
            {
                throw new EvaluationException("Factorial is only defined for non-negative integers", node.Position);
            }

            int n = (int)Math.Round(value);

            // Calculate factorial
            decimal result = 1;
            for (int i = 2; i <= n; i++)
            {
                result *= i;
            }

            string expression = _formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Calculate factorial of {n}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }

        private void CheckArgumentCount(FunctionNode node, int expectedCount)
        {
            if (node.Arguments.Count != expectedCount)
            {
                throw new EvaluationException(
                    $"Function {node.Name} expects {expectedCount} argument(s), got {node.Arguments.Count}",
                    node.Position);
            }
        }
    }
}