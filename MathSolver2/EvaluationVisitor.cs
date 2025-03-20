namespace MathSolver2
{

    /// <summary>
    /// Evaluates an expression tree and returns the numeric result
    /// </summary>
    [Obsolete("This class is obsolete and will be removed in a future version. Use the ArithmeticVisitor class instead.")]
    public class EvaluationVisitor : IExpressionVisitor<decimal>
    {
        private readonly Dictionary<string, decimal> _variables;

        public EvaluationVisitor()
        {
            _variables = new Dictionary<string, decimal>();
        }

        public EvaluationVisitor(Dictionary<string, decimal> variables)
        {
            _variables = variables ?? new Dictionary<string, decimal>();
        }

        /// <summary>
        /// Sets a variable value
        /// </summary>
        public void SetVariable(string name, decimal value)
        {
            _variables[name] = value;
        }

        /// <summary>
        /// Gets a variable value
        /// </summary>
        public decimal GetVariable(string name)
        {
            if (_variables.TryGetValue(name, out decimal value))
            {
                return value;
            }

            throw new ArgumentException($"Variable '{name}' is not defined");
        }

        /// <summary>
        /// Evaluates the expression and returns the result
        /// </summary>
        public decimal Evaluate(IExpressionNode node)
        {
            return node.Accept(this);
        }

        public decimal VisitNumber(NumberNode node)
        {
            return node.Value;
        }

        public decimal VisitVariable(VariableNode node)
        {
            if (_variables.TryGetValue(node.Name, out decimal value))
            {
                return value;
            }

            throw new EvaluationException($"Variable '{node.Name}' is not defined", node.Position);
        }

        public decimal VisitAddition(AdditionNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            return left + right;
        }

        public decimal VisitSubtraction(SubtractionNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            return left - right;
        }

        public decimal VisitMultiplication(MultiplicationNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            return left * right;
        }

        public decimal VisitDivision(DivisionNode node)
        {
            decimal numerator = node.Numerator.Accept(this);
            decimal denominator = node.Denominator.Accept(this);

            if (denominator == 0)
            {
                throw new EvaluationException("Division by zero", node.Position);
            }

            return numerator / denominator;
        }

        public decimal VisitExponent(ExponentNode node)
        {
            decimal @base = node.Base.Accept(this);
            decimal exponent = node.Exponent.Accept(this);

            // Handle some special cases
            if (exponent == 0)
            {
                return 1;
            }

            if (@base == 0)
            {
                return 0;
            }

            // Check if exponent is an integer
            if (Math.Abs(exponent - Math.Round(exponent)) < Precision.Epsilon)
            {
                int intExponent = (int)Math.Round(exponent);

                // Use standard power algorithm for integer exponents
                return (decimal)Math.Pow((double)@base, intExponent);
            }

            // For non-integer exponents, we need to use the Math.Pow function
            // which works with doubles, so we'll convert and then convert back
            return (decimal)Math.Pow((double)@base, (double)exponent);
        }

        public decimal VisitParenthesis(ParenthesisNode node)
        {
            return node.Expression.Accept(this);
        }

        public decimal VisitFunction(FunctionNode node)
        {
            string functionName = node.Name.ToLower();

            switch (functionName)
            {
                case "sin":
                    CheckArgumentCount(node, 1);
                    return (decimal)Math.Sin((double)node.Arguments[0].Accept(this));

                case "cos":
                    CheckArgumentCount(node, 1);
                    return (decimal)Math.Cos((double)node.Arguments[0].Accept(this));

                case "tan":
                    CheckArgumentCount(node, 1);
                    return (decimal)Math.Tan((double)node.Arguments[0].Accept(this));

                case "sqrt":
                    CheckArgumentCount(node, 1);
                    decimal arg = node.Arguments[0].Accept(this);
                    if (arg < 0)
                    {
                        throw new EvaluationException("Cannot take square root of a negative number", node.Position);
                    }
                    return (decimal)Math.Sqrt((double)arg);

                default:
                    throw new EvaluationException($"Unsupported function: {node.Name}", node.Position);
            }
        }

        public decimal VisitFactorial(FactorialNode node)
        {
            decimal value = node.Expression.Accept(this);

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

            return result;
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