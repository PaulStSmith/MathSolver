namespace MathSolver2
{
    /// <summary>
    /// Evaluates an expression tree with arithmetic formatting applied at each calculation step.
    /// </summary>
    public class ArithmeticVisitor : BaseArithmeticVisitor<decimal>
    {
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
            bool useSignificantDigits) : base(variables, arithmeticType, precision, useSignificantDigits) { }

        #region Visitor Methods

        /// <summary>
        /// Visits a number node and returns its value.
        /// </summary>
        /// <param name="node">The number node to visit.</param>
        /// <returns>The value of the number node.</returns>
        public override decimal VisitNumber(NumberNode node)
        {
            return node.Value;
        }

        /// <summary>
        /// Visits a variable node and returns its value.
        /// </summary>
        /// <param name="node">The variable node to visit.</param>
        /// <returns>The value of the variable node.</returns>
        /// <exception cref="EvaluationException">Thrown when the variable is not defined.</exception>
        public override decimal VisitVariable(VariableNode node)
        {
            if (MathConstants.TryGetValue(node.Name, out decimal constValue))
            {
                // Apply the existing formatting logic to the constant
                return FormatNumber(constValue);
            }

            if (Variables.TryGetValue(node.Name, out decimal value))
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
        public override decimal VisitAddition(AdditionNode node)
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
        public override decimal VisitSubtraction(SubtractionNode node)
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
        public override decimal VisitMultiplication(MultiplicationNode node)
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
        public override decimal VisitDivision(DivisionNode node)
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
        public override decimal VisitExponent(ExponentNode node)
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
            if (Math.Abs(exponent - Math.Round(exponent)) < MathConstants.Epsilon)
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
        public override decimal VisitParenthesis(ParenthesisNode node)
        {
            return node.Expression.Accept(this);
        }

        /// <summary>
        /// Visits a function node and returns the result of the function.
        /// </summary>
        /// <param name="node">The function node to visit.</param>
        /// <returns>The result of the function.</returns>
        /// <exception cref="EvaluationException">Thrown when the function is unsupported or has incorrect arguments.</exception>
        public override decimal VisitFunction(FunctionNode node)
        {
            // Evaluate the arguments
            decimal[] args = node.Arguments.Select(arg => arg.Accept(this)).ToArray();

            // Evaluate the function - getting both result and description
            var (result, _) = EvaluateFunction(node.Name, args, node.Position);

            // Just return the result (ignore the description)
            return result;
        }

        /// <summary>
        /// Visits a factorial node and returns the result of the factorial.
        /// </summary>
        /// <param name="node">The factorial node to visit.</param>
        /// <returns>The result of the factorial.</returns>
        /// <exception cref="EvaluationException">Thrown when the value is not a non-negative integer.</exception>
        public override decimal VisitFactorial(FactorialNode node)
        {
            decimal value = node.Expression.Accept(this);

            // Check if value is a non-negative integer
            if (value < 0 || Math.Abs(value - Math.Round(value)) > MathConstants.Epsilon)
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

        /// <summary>
        /// Visits a summation node and returns the result of the summation.
        /// </summary>
        /// <param name="node">The summation node to visit.</param>
        /// <returns>The result of the summation.</returns>
        /// <exception cref="EvaluationException">Thrown when the summation bounds are not integers.</exception>
        public override decimal VisitSummation(SummationNode node)
        {
            // Get the start value
            decimal start = node.Start.Accept(this);

            // Get the end value
            decimal end = node.End.Accept(this);

            // We need to handle non-integer bounds as an error
            if (Math.Abs(start - Math.Round(start)) > MathConstants.Epsilon ||
                Math.Abs(end - Math.Round(end)) > MathConstants.Epsilon)
            {
                throw new EvaluationException("Summation bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(start);
            int endInt = (int)Math.Round(end);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = Variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Initialize the result
                decimal result = 0;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    Variables[node.Variable] = i;

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
                    Variables[node.Variable] = originalValue;
                }
                else
                {
                    Variables.Remove(node.Variable);
                }
            }
        }

        /// <summary>
        /// Visits a product node and returns the result of the product.
        /// </summary>
        /// <param name="node">The product node to visit.</param>
        /// <returns>The result of the product.</returns>
        /// <exception cref="EvaluationException">Thrown when the product bounds are not integers.</exception>
        public override decimal VisitProduct(ProductNode node)
        {
            // Get the start value
            decimal start = node.Start.Accept(this);

            // Get the end value
            decimal end = node.End.Accept(this);

            // We need to handle non-integer bounds as an error
            if (Math.Abs(start - Math.Round(start)) > MathConstants.Epsilon ||
                Math.Abs(end - Math.Round(end)) > MathConstants.Epsilon)
            {
                throw new EvaluationException("Product bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(start);
            int endInt = (int)Math.Round(end);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = Variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Initialize the result
                decimal result = 1;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    Variables[node.Variable] = i;

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
                    Variables[node.Variable] = originalValue;
                }
                else
                {
                    Variables.Remove(node.Variable);
                }
            }
        }
    }
}