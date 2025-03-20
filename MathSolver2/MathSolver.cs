namespace MathSolver2
{
    /// <summary>
    /// Facade class for parsing and evaluating mathematical expressions
    /// </summary>
    [Obsolete("This class is obsolete and will be removed in a future version. Use the EnhancedMathSolver class instead.")]
    public class MathSolver
    {
        private readonly Dictionary<string, decimal> _variables;

        public MathSolver()
        {
            _variables = new Dictionary<string, decimal>();
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
        /// Evaluates an expression and returns the result
        /// </summary>
        public decimal Evaluate(string expression)
        {
            try
            {
                // Parse the expression
                ExpressionParser parser = new ExpressionParser(expression);
                IExpressionNode root = parser.Parse();

                // Evaluate the expression
                EvaluationVisitor visitor = new EvaluationVisitor(_variables);
                return visitor.Evaluate(root);
            }
            catch (ParserException ex)
            {
                throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
            }
            catch (EvaluationException ex)
            {
                throw new ArgumentException($"Evaluation error: {ex.Message} at position {ex.Position}");
            }
        }

        /// <summary>
        /// Evaluates an expression step by step and returns all calculation steps
        /// </summary>
        public StepByStepResult EvaluateStepByStep(string expression)
        {
            try
            {
                // Parse the expression
                ExpressionParser parser = new ExpressionParser(expression);
                IExpressionNode root = parser.Parse();

                // Evaluate the expression step by step
                StepByStepVisitor visitor = new StepByStepVisitor(_variables);
                return visitor.Evaluate(root);
            }
            catch (ParserException ex)
            {
                throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
            }
            catch (EvaluationException ex)
            {
                throw new ArgumentException($"Evaluation error: {ex.Message} at position {ex.Position}");
            }
        }

        /// <summary>
        /// Formats an expression in standard or LaTeX notation
        /// </summary>
        public string Format(string expression, OutputFormat format = OutputFormat.Standard)
        {
            try
            {
                // Parse the expression
                ExpressionParser parser = new ExpressionParser(expression);
                IExpressionNode root = parser.Parse();

                // Format the expression
                FormattingVisitor visitor = new FormattingVisitor(format);
                return visitor.Format(root);
            }
            catch (ParserException ex)
            {
                throw new ArgumentException($"Parse error: {ex.Message} at position {ex.Position}");
            }
        }

        /// <summary>
        /// Validates if an expression can be parsed without errors
        /// </summary>
        public bool Validate(string expression, out string error)
        {
            try
            {
                ExpressionParser parser = new ExpressionParser(expression);
                parser.Parse();

                error = null;
                return true;
            }
            catch (ParserException ex)
            {
                error = $"Parse error: {ex.Message} at position {ex.Position}";
                return false;
            }
        }
    }
}