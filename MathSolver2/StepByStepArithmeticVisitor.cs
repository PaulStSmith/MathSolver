namespace MathSolver2
{
    /// <summary>
    /// Evaluates an expression tree step by step with arithmetic formatting and records each calculation
    /// </summary>
    public class StepByStepArithmeticVisitor : IExpressionVisitor<StepByStepResult>
    {
        private readonly Dictionary<string, decimal> _variables;
        private readonly FormattingVisitor _formatter;
        private readonly ArithmeticType _arithmeticType;
        private readonly int _precision;
        private readonly bool _useSignificantDigits;

        /// <summary>
        /// Creates a new step-by-step arithmetic visitor with the specified settings
        /// </summary>
        public StepByStepArithmeticVisitor(
            Dictionary<string, decimal> variables,
            ArithmeticType arithmeticType,
            int precision,
            bool useSignificantDigits)
        {
            _variables = variables ?? new Dictionary<string, decimal>();
            _formatter = new FormattingVisitor();
            _arithmeticType = arithmeticType;
            _precision = precision;
            _useSignificantDigits = useSignificantDigits;
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

        /// <summary>
        /// Formats a number according to the specified arithmetic settings
        /// </summary>
        private decimal FormatNumber(decimal value)
        {
            switch (_arithmeticType)
            {
                case ArithmeticType.Normal:
                    return value;

                case ArithmeticType.Truncate:
                    if (_useSignificantDigits)
                    {
                        return TruncateToSignificantDigits(value, _precision);
                    }
                    else
                    {
                        return TruncateToDecimalPlaces(value, _precision);
                    }

                case ArithmeticType.Round:
                    if (_useSignificantDigits)
                    {
                        return RoundToSignificantDigits(value, _precision);
                    }
                    else
                    {
                        return Math.Round(value, _precision);
                    }

                default:
                    return value;
            }
        }

        /// <summary>
        /// Truncates a decimal value to the specified number of decimal places
        /// </summary>
        private decimal TruncateToDecimalPlaces(decimal value, int decimalPlaces)
        {
            if (decimalPlaces < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(decimalPlaces), "Decimal places must be non-negative");
            }

            decimal multiplier = (decimal)Math.Pow(10, decimalPlaces);

            if (value >= 0)
            {
                return Math.Floor(value * multiplier) / multiplier;
            }
            else
            {
                return Math.Ceiling(value * multiplier) / multiplier;
            }
        }

        /// <summary>
        /// Truncates a decimal value to the specified number of significant digits
        /// </summary>
        private decimal TruncateToSignificantDigits(decimal value, int sigDigits)
        {
            if (sigDigits <= 0)
            {
                throw new ArgumentOutOfRangeException(nameof(sigDigits), "Significant digits must be positive");
            }

            if (value == 0)
            {
                return 0;
            }

            // Get the exponent (power of 10) of the value
            int exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value)));

            // Calculate the number of decimal places needed
            int decimalPlaces = sigDigits - exponent - 1;

            // Adjust for small numbers (value < 1)
            if (value != 0 && Math.Abs(value) < 1)
            {
                decimalPlaces = sigDigits + Math.Abs(exponent + 1);
            }

            return TruncateToDecimalPlaces(value, Math.Max(0, decimalPlaces));
        }

        /// <summary>
        /// Rounds a decimal value to the specified number of significant digits
        /// </summary>
        private decimal RoundToSignificantDigits(decimal value, int sigDigits)
        {
            if (sigDigits <= 0)
            {
                throw new ArgumentOutOfRangeException(nameof(sigDigits), "Significant digits must be positive");
            }

            if (value == 0)
            {
                return 0;
            }

            // Get the exponent (power of 10) of the value
            int exponent = (int)Math.Floor(Math.Log10((double)Math.Abs(value)));

            // Calculate the number of decimal places needed
            int decimalPlaces = sigDigits - exponent - 1;

            // Adjust for small numbers (value < 1)
            if (value != 0 && Math.Abs(value) < 1)
            {
                decimalPlaces = sigDigits + Math.Abs(exponent + 1);
            }

            return Math.Round(value, Math.Max(0, decimalPlaces));
        }

        #region Visitor Methods

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

            decimal rawResult = leftResult.Value + rightResult.Value;
            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"Add {leftResult.Value} and {rightResult.Value}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
        }

        public StepByStepResult VisitSubtraction(SubtractionNode node)
        {
            var leftResult = node.Left.Accept(this);
            var rightResult = node.Right.Accept(this);

            var allSteps = new List<CalculationStep>(leftResult.Steps);
            allSteps.AddRange(rightResult.Steps);

            decimal rawResult = leftResult.Value - rightResult.Value;
            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"Subtract {rightResult.Value} from {leftResult.Value}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
        }

        public StepByStepResult VisitMultiplication(MultiplicationNode node)
        {
            var leftResult = node.Left.Accept(this);
            var rightResult = node.Right.Accept(this);

            var allSteps = new List<CalculationStep>(leftResult.Steps);
            allSteps.AddRange(rightResult.Steps);

            decimal rawResult = leftResult.Value * rightResult.Value;
            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"Multiply {leftResult.Value} by {rightResult.Value}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
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

            decimal rawResult = numeratorResult.Value / denominatorResult.Value;
            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"Divide {numeratorResult.Value} by {denominatorResult.Value}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
        }

        public StepByStepResult VisitExponent(ExponentNode node)
        {
            var baseResult = node.Base.Accept(this);
            var exponentResult = node.Exponent.Accept(this);

            var allSteps = new List<CalculationStep>(baseResult.Steps);
            allSteps.AddRange(exponentResult.Steps);

            decimal rawResult;
            string operation;

            // Handle special cases
            if (exponentResult.Value == 0)
            {
                rawResult = 1;
                operation = $"Any number raised to power 0 is 1";
            }
            else if (baseResult.Value == 0)
            {
                rawResult = 0;
                operation = $"0 raised to any non-zero power is 0";
            }
            else if (exponentResult.Value == 1)
            {
                rawResult = baseResult.Value;
                operation = $"Any number raised to power 1 is the number itself";
            }
            else
            {
                // Check if exponent is an integer
                if (Math.Abs(exponentResult.Value - Math.Round(exponentResult.Value)) < Precision.Epsilon)
                {
                    int intExponent = (int)Math.Round(exponentResult.Value);
                    rawResult = (decimal)Math.Pow((double)baseResult.Value, intExponent);
                }
                else
                {
                    // For non-integer exponents, use Math.Pow
                    rawResult = (decimal)Math.Pow((double)baseResult.Value, (double)exponentResult.Value);
                }

                operation = $"Raise {baseResult.Value} to the power of {exponentResult.Value}";
            }

            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"{operation}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
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
            decimal rawResult;
            string operation;

            switch (functionName)
            {
                case "sin":
                    CheckArgumentCount(node, 1);
                    rawResult = (decimal)Math.Sin((double)argResults[0].Value);
                    operation = $"Calculate sine of {argResults[0].Value}";
                    break;

                case "cos":
                    CheckArgumentCount(node, 1);
                    rawResult = (decimal)Math.Cos((double)argResults[0].Value);
                    operation = $"Calculate cosine of {argResults[0].Value}";
                    break;

                case "tan":
                    CheckArgumentCount(node, 1);
                    rawResult = (decimal)Math.Tan((double)argResults[0].Value);
                    operation = $"Calculate tangent of {argResults[0].Value}";
                    break;

                case "sqrt":
                    CheckArgumentCount(node, 1);
                    decimal arg = argResults[0].Value;
                    if (arg < 0)
                    {
                        throw new EvaluationException("Cannot take square root of a negative number", node.Position);
                    }
                    rawResult = (decimal)Math.Sqrt((double)arg);
                    operation = $"Calculate square root of {arg}";
                    break;

                case "log":
                    CheckArgumentCount(node, 1);
                    if (argResults[0].Value <= 0)
                    {
                        throw new EvaluationException("Cannot take logarithm of a non-positive number", node.Position);
                    }
                    rawResult = (decimal)Math.Log10((double)argResults[0].Value);
                    operation = $"Calculate base-10 logarithm of {argResults[0].Value}";
                    break;

                case "ln":
                    CheckArgumentCount(node, 1);
                    if (argResults[0].Value <= 0)
                    {
                        throw new EvaluationException("Cannot take natural logarithm of a non-positive number", node.Position);
                    }
                    rawResult = (decimal)Math.Log((double)argResults[0].Value);
                    operation = $"Calculate natural logarithm of {argResults[0].Value}";
                    break;

                default:
                    throw new EvaluationException($"Unsupported function: {node.Name}", node.Position);
            }

            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"{operation}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
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
            decimal rawResult = 1;
            for (int i = 2; i <= n; i++)
            {
                rawResult *= i;
            }

            decimal formattedResult = FormatNumber(rawResult);

            string formatInfo = GetFormatInfo();
            string expression = _formatter.Format(node);

            allSteps.Add(new CalculationStep(
                expression,
                $"Calculate factorial of {n}, {formatInfo}",
                formattedResult.ToString()));

            return new StepByStepResult(formattedResult, allSteps);
        }

        #endregion

        public StepByStepResult VisitSummation(SummationNode node)
        {
            return IteratorNodeVisitors.VisitSummation(
                this,
                node,
                _variables,
                _formatter,
                FormatNumber,
                GetFormatInfo);
        }

        public StepByStepResult VisitProduct(ProductNode node)
        {
            return IteratorNodeVisitors.VisitProduct(
                this,
                node,
                _variables,
                _formatter,
                FormatNumber,
                GetFormatInfo);
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

        /// <summary>
        /// Gets a description of the current formatting applied
        /// </summary>
        private string GetFormatInfo()
        {
            if (_arithmeticType == ArithmeticType.Normal)
            {
                return "with no formatting";
            }

            string action = _arithmeticType == ArithmeticType.Truncate ? "truncating" : "rounding";
            string precision = _useSignificantDigits ?
                $"{_precision} significant digit{(_precision > 1 ? "s" : "")}" :
                $"{_precision} decimal place{(_precision > 1 ? "s" : "")}";

            return $"{action} to {precision}";
        }
    }
}