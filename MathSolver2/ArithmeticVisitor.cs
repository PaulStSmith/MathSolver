namespace MathSolver2
{
    /// <summary>
    /// Evaluates an expression tree with arithmetic formatting applied at each calculation step.
    /// </summary>
    public class ArithmeticVisitor : ILatexExpressionVisitor<decimal>
    {
        private readonly Dictionary<string, decimal> _variables;
        private readonly ArithmeticType _arithmeticType;
        private readonly int _precision;
        private readonly bool _useSignificantDigits;

        /// <summary>
        /// Creates a new arithmetic visitor with the specified settings.
        /// </summary>
        /// <param name="variables">Dictionary of variable names and their values.</param>
        /// <param name="arithmeticType">Type of arithmetic to use for calculations.</param>
        /// <param name="precision">Precision to use for rounding or truncation.</param>
        /// <param name="useSignificantDigits">Whether to use significant digits for formatting.</param>
        public ArithmeticVisitor(
            Dictionary<string, decimal> variables,
            ArithmeticType arithmeticType,
            int precision,
            bool useSignificantDigits)
        {
            _variables = variables ?? new Dictionary<string, decimal>();
            _arithmeticType = arithmeticType;
            _precision = precision;
            _useSignificantDigits = useSignificantDigits;
        }

        /// <summary>
        /// Sets a variable value.
        /// </summary>
        /// <param name="name">The name of the variable.</param>
        /// <param name="value">The value to set for the variable.</param>
        public void SetVariable(string name, decimal value)
        {
            _variables[name] = value;
        }

        /// <summary>
        /// Evaluates the expression and returns the result with arithmetic formatting.
        /// </summary>
        /// <param name="node">The expression node to evaluate.</param>
        /// <returns>The evaluated result with formatting applied.</returns>
        public decimal Evaluate(IExpressionNode node)
        {
            return node.Accept(this);
        }

        /// <summary>
        /// Formats a number according to the specified arithmetic settings.
        /// </summary>
        /// <param name="value">The value to format.</param>
        /// <returns>The formatted value.</returns>
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
        /// Truncates a decimal value to the specified number of decimal places.
        /// </summary>
        /// <param name="value">The value to truncate.</param>
        /// <param name="decimalPlaces">The number of decimal places to truncate to.</param>
        /// <returns>The truncated value.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown when decimalPlaces is negative.</exception>
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
        /// Truncates a decimal value to the specified number of significant digits.
        /// </summary>
        /// <param name="value">The value to truncate.</param>
        /// <param name="sigDigits">The number of significant digits to truncate to.</param>
        /// <returns>The truncated value.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive.</exception>
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
        /// Rounds a decimal value to the specified number of significant digits.
        /// </summary>
        /// <param name="value">The value to round.</param>
        /// <param name="sigDigits">The number of significant digits to round to.</param>
        /// <returns>The rounded value.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown when sigDigits is not positive.</exception>
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

        /// <summary>
        /// Visits a number node and returns its value.
        /// </summary>
        /// <param name="node">The number node to visit.</param>
        /// <returns>The value of the number node.</returns>
        public decimal VisitNumber(NumberNode node)
        {
            return node.Value;
        }

        /// <summary>
        /// Visits a variable node and returns its value.
        /// </summary>
        /// <param name="node">The variable node to visit.</param>
        /// <returns>The value of the variable node.</returns>
        /// <exception cref="EvaluationException">Thrown when the variable is not defined.</exception>
        public decimal VisitVariable(VariableNode node)
        {
            if (_variables.TryGetValue(node.Name, out decimal value))
            {
                return value;
            }

            throw new EvaluationException($"Variable '{node.Name}' is not defined", node.Position);
        }

        /// <summary>
        /// Visits an addition node and returns the result of the addition.
        /// </summary>
        /// <param name="node">The addition node to visit.</param>
        /// <returns>The result of the addition.</returns>
        public decimal VisitAddition(AdditionNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            decimal result = left + right;
            return FormatNumber(result);
        }

        /// <summary>
        /// Visits a subtraction node and returns the result of the subtraction.
        /// </summary>
        /// <param name="node">The subtraction node to visit.</param>
        /// <returns>The result of the subtraction.</returns>
        public decimal VisitSubtraction(SubtractionNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            decimal result = left - right;
            return FormatNumber(result);
        }

        /// <summary>
        /// Visits a multiplication node and returns the result of the multiplication.
        /// </summary>
        /// <param name="node">The multiplication node to visit.</param>
        /// <returns>The result of the multiplication.</returns>
        public decimal VisitMultiplication(MultiplicationNode node)
        {
            decimal left = node.Left.Accept(this);
            decimal right = node.Right.Accept(this);

            decimal result = left * right;
            return FormatNumber(result);
        }

        /// <summary>
        /// Visits a division node and returns the result of the division.
        /// </summary>
        /// <param name="node">The division node to visit.</param>
        /// <returns>The result of the division.</returns>
        /// <exception cref="EvaluationException">Thrown when division by zero occurs.</exception>
        public decimal VisitDivision(DivisionNode node)
        {
            decimal numerator = node.Numerator.Accept(this);
            decimal denominator = node.Denominator.Accept(this);

            if (denominator == 0)
            {
                throw new EvaluationException("Division by zero", node.Position);
            }

            decimal result = numerator / denominator;
            return FormatNumber(result);
        }

        /// <summary>
        /// Visits an exponent node and returns the result of the exponentiation.
        /// </summary>
        /// <param name="node">The exponent node to visit.</param>
        /// <returns>The result of the exponentiation.</returns>
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

            decimal result;

            // Check if exponent is an integer
            if (Math.Abs(exponent - Math.Round(exponent)) < Precision.Epsilon)
            {
                int intExponent = (int)Math.Round(exponent);

                // Use standard power algorithm for integer exponents
                result = (decimal)Math.Pow((double)@base, intExponent);
            }
            else
            {
                // For non-integer exponents, we need to use the Math.Pow function
                // which works with doubles, so we'll convert and then convert back
                result = (decimal)Math.Pow((double)@base, (double)exponent);
            }

            return FormatNumber(result);
        }

        /// <summary>
        /// Visits a parenthesis node and returns the result of the enclosed expression.
        /// </summary>
        /// <param name="node">The parenthesis node to visit.</param>
        /// <returns>The result of the enclosed expression.</returns>
        public decimal VisitParenthesis(ParenthesisNode node)
        {
            return node.Expression.Accept(this);
        }

        /// <summary>
        /// Visits a function node and returns the result of the function.
        /// </summary>
        /// <param name="node">The function node to visit.</param>
        /// <returns>The result of the function.</returns>
        /// <exception cref="EvaluationException">Thrown when the function is unsupported or has incorrect arguments.</exception>
        public decimal VisitFunction(FunctionNode node)
        {
            string functionName = node.Name.ToLower();

            switch (functionName)
            {
                case "sin":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        decimal result = (decimal)Math.Sin((double)arg);
                        return FormatNumber(result);
                    }

                case "cos":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        decimal result = (decimal)Math.Cos((double)arg);
                        return FormatNumber(result);
                    }

                case "tan":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        decimal result = (decimal)Math.Tan((double)arg);
                        return FormatNumber(result);
                    }

                case "sqrt":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        if (arg < 0)
                        {
                            throw new EvaluationException("Cannot take square root of a negative number", node.Position);
                        }
                        decimal result = (decimal)Math.Sqrt((double)arg);
                        return FormatNumber(result);
                    }

                case "log":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        if (arg <= 0)
                        {
                            throw new EvaluationException("Cannot take logarithm of a non-positive number", node.Position);
                        }
                        decimal result = (decimal)Math.Log10((double)arg);
                        return FormatNumber(result);
                    }

                case "ln":
                    CheckArgumentCount(node, 1);
                    {
                        decimal arg = node.Arguments[0].Accept(this);
                        if (arg <= 0)
                        {
                            throw new EvaluationException("Cannot take natural logarithm of a non-positive number", node.Position);
                        }
                        decimal result = (decimal)Math.Log((double)arg);
                        return FormatNumber(result);
                    }

                default:
                    throw new EvaluationException($"Unsupported function: {node.Name}", node.Position);
            }
        }

        /// <summary>
        /// Visits a factorial node and returns the result of the factorial.
        /// </summary>
        /// <param name="node">The factorial node to visit.</param>
        /// <returns>The result of the factorial.</returns>
        /// <exception cref="EvaluationException">Thrown when the value is not a non-negative integer.</exception>
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

            return FormatNumber(result);
        }

        #endregion

        public decimal VisitSummation(SummationNode node)
        {
            // Get the start value
            decimal start = node.Start.Accept(this);

            // Get the end value
            decimal end = node.End.Accept(this);

            // We need to handle non-integer bounds as an error
            if (Math.Abs(start - Math.Round(start)) > Precision.Epsilon ||
                Math.Abs(end - Math.Round(end)) > Precision.Epsilon)
            {
                throw new EvaluationException("Summation bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(start);
            int endInt = (int)Math.Round(end);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = _variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Initialize the result
                decimal result = 0;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    _variables[node.Variable] = i;

                    // Evaluate the expression and add to the result
                    decimal value = node.Expression.Accept(this);
                    result += value;

                    // Apply formatting after each step if needed
                    result = FormatNumber(result);
                }

                return result;
            }
            finally
            {
                // Restore the original value of the iteration variable
                if (hasOriginalValue)
                {
                    _variables[node.Variable] = originalValue;
                }
                else
                {
                    _variables.Remove(node.Variable);
                }
            }
        }

        public decimal VisitProduct(ProductNode node)
        {
            // Get the start value
            decimal start = node.Start.Accept(this);

            // Get the end value
            decimal end = node.End.Accept(this);

            // We need to handle non-integer bounds as an error
            if (Math.Abs(start - Math.Round(start)) > Precision.Epsilon ||
                Math.Abs(end - Math.Round(end)) > Precision.Epsilon)
            {
                throw new EvaluationException("Product bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(start);
            int endInt = (int)Math.Round(end);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = _variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Initialize the result
                decimal result = 1;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    _variables[node.Variable] = i;

                    // Evaluate the expression and multiply to the result
                    decimal value = node.Expression.Accept(this);
                    result *= value;

                    // Apply formatting after each step if needed
                    result = FormatNumber(result);
                }

                return result;
            }
            finally
            {
                // Restore the original value of the iteration variable
                if (hasOriginalValue)
                {
                    _variables[node.Variable] = originalValue;
                }
                else
                {
                    _variables.Remove(node.Variable);
                }
            }
        }

        /// <summary>
        /// Checks if the function node has the expected number of arguments.
        /// </summary>
        /// <param name="node">The function node to check.</param>
        /// <param name="expectedCount">The expected number of arguments.</param>
        /// <exception cref="EvaluationException">Thrown when the argument count does not match the expected count.</exception>
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