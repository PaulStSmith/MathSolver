using System;
using System.Collections.Generic;

namespace MathSolver
{
    /// <summary>
    /// Main facade class that coordinates the process of solving mathematical expressions
    /// with configurable arithmetic and direction settings
    /// </summary>
    public class MathSolver
    {
        private ArithmeticHandler _arithmeticHandler;
        private FunctionEvaluator _functionEvaluator;
        private ExpressionParser _expressionParser;
        private TokenProcessor _tokenProcessor;
        private ExpressionEvaluator _expressionEvaluator;
        private CalculationDirection _direction;

        /// <summary>
        /// Creates a new mathematical expression solver with specified settings
        /// </summary>
        /// <param name="arithmeticType">Type of arithmetic to use</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        /// <param name="direction">Direction for evaluating expressions</param>
        public MathSolver(
            ArithmeticType arithmeticType = ArithmeticType.Normal,
            int precision = 10,
            bool useSignificantDigits = false,
            CalculationDirection direction = CalculationDirection.LeftToRight)
        {
            // Initialize components with dependency injection
            _arithmeticHandler = new ArithmeticHandler(arithmeticType, precision, useSignificantDigits);
            _functionEvaluator = new FunctionEvaluator(_arithmeticHandler);
            _expressionParser = new ExpressionParser();
            _tokenProcessor = new TokenProcessor();
            _expressionEvaluator = new ExpressionEvaluator(_arithmeticHandler, _functionEvaluator);
            _direction = direction;
        }

        /// <summary>
        /// Sets the arithmetic mode for the solver
        /// </summary>
        /// <param name="type">Type of arithmetic to use</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        public void SetArithmeticMode(ArithmeticType type, int precision, bool useSignificantDigits)
        {
            // Create a new arithmetic handler with the updated settings
            var newHandler = new ArithmeticHandler(type, precision, useSignificantDigits);

            // Update the components that depend on arithmetic settings
            _arithmeticHandler = newHandler;
            _functionEvaluator = new FunctionEvaluator(_arithmeticHandler);
            _expressionEvaluator = new ExpressionEvaluator(_arithmeticHandler, _functionEvaluator);
        }

        /// <summary>
        /// Sets the calculation direction
        /// </summary>
        /// <param name="direction">Direction to use for calculations</param>
        public void SetCalculationDirection(CalculationDirection direction)
        {
            _direction = direction;
        }

        /// <summary>
        /// Registers a custom function with the solver
        /// </summary>
        /// <param name="name">Name of the function</param>
        /// <param name="implementation">Function implementation</param>
        public void RegisterFunction(string name, Func<double[], double> implementation)
        {
            _functionEvaluator.RegisterFunction(name, implementation);
            _expressionParser.RegisterFunction(name);
        }

        /// <summary>
        /// Sets a variable value for use in expressions
        /// </summary>
        /// <param name="name">Variable name</param>
        /// <param name="value">Variable value</param>
        public void SetVariable(string name, double value)
        {
            _expressionEvaluator.SetVariable(name, value);
        }

        /// <summary>
        /// Clears all variables
        /// </summary>
        public void ClearVariables()
        {
            _expressionEvaluator.ClearVariables();
        }

        /// <summary>
        /// Solves a LaTeX math expression with detailed step-by-step output
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>A calculation result with detailed steps</returns>
        public CalculationResult SolveWithSteps(string latexExpression)
        {
            var result = new CalculationResult();
            result.OriginalExpression = latexExpression;
            result.ArithmeticMode = _arithmeticHandler.ArithmeticType.ToString();

            // Get precision info from the arithmetic handler's properties
            result.PrecisionInfo = _arithmeticHandler.PrecisionDescription;
            result.Direction = _direction.ToString();

            try
            {
                // First, handle direct numbers (already in decimal form)
                if (double.TryParse(latexExpression, out double directValue))
                {
                    result.ActualResult = directValue;
                    result.FormattedResult = _arithmeticHandler.FormatNumber(directValue);
                    return result;
                }

                // Create a calculator with normal arithmetic for the actual result
                var normalSolver = new MathSolver(ArithmeticType.Normal, 15, false, _direction);
                result.ActualResult = normalSolver.Solve(latexExpression);

                // Tokenize the expression (the parser will handle LaTeX conversion internally)
                var tokens = _expressionParser.Tokenize(latexExpression);

                // Evaluate based on the direction and collect steps
                List<CalculationStep> steps = new List<CalculationStep>();
                double formattedResult;

                if (_direction == CalculationDirection.LeftToRight)
                {
                    var postfixTokens = _tokenProcessor.ConvertToPostfix(tokens);
                    formattedResult = _expressionEvaluator.Evaluate(postfixTokens, steps, false);
                }
                else // RightToLeft
                {
                    var prefixTokens = _tokenProcessor.ConvertToPrefix(tokens);
                    formattedResult = _expressionEvaluator.Evaluate(prefixTokens, steps, true);
                }

                result.FormattedResult = formattedResult;
                result.Steps.AddRange(steps);
                return result;
            }
            catch (Exception ex)
            {
                // Instead of throwing, add an error step and return a result
                result.Steps.Add(new CalculationStep($"Error: {ex.Message}", double.NaN));
                result.FormattedResult = double.NaN;
                result.ActualResult = double.NaN;
                return result;
            }
        }



        /// <summary>
        /// Solves a LaTeX math expression
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <param name="useNormalArithmetic">Whether to use normal arithmetic regardless of settings</param>
        /// <returns>The result of evaluating the expression</returns>
        public double Solve(string latexExpression, bool useNormalArithmetic = false)
        {
            try
            {
                // First, handle direct numbers (already in decimal form)
                if (double.TryParse(latexExpression, out double directValue))
                {
                    return useNormalArithmetic ? directValue : _arithmeticHandler.FormatNumber(directValue);
                }

                // Use a temporary arithmetic handler if we need normal arithmetic
                ArithmeticHandler tempHandler = useNormalArithmetic ?
                    new ArithmeticHandler(ArithmeticType.Normal, 15, false) :
                    _arithmeticHandler;

                // Tokenize the expression directly (the parser will handle LaTeX conversion)
                var tokens = _expressionParser.Tokenize(latexExpression);

                // Create a temporary evaluator if needed
                ExpressionEvaluator evaluator = useNormalArithmetic ?
                    new ExpressionEvaluator(tempHandler, _functionEvaluator) :
                    _expressionEvaluator;

                // Evaluate based on the direction
                if (_direction == CalculationDirection.LeftToRight)
                {
                    var postfixTokens = _tokenProcessor.ConvertToPostfix(tokens);
                    return evaluator.Evaluate(postfixTokens, null, false);
                }
                else // RightToLeft
                {
                    var prefixTokens = _tokenProcessor.ConvertToPrefix(tokens);
                    return evaluator.Evaluate(prefixTokens, null, true);
                }
            }
            catch (Exception ex)
            {
                throw new Exception($"Error solving expression: {ex.Message}", ex);
            }
        }

        /// <summary>
        /// Solves a LaTeX math expression and returns the result as a string
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>The result of evaluating the expression as a string</returns>
        public string SolveToString(string latexExpression)
        {
            try
            {
                double result = Solve(latexExpression);
                return _arithmeticHandler.FormatNumberAsString(result);
            }
            catch (Exception ex)
            {
                return $"Error: {ex.Message}";
            }
        }

        /// <summary>
        /// Calculates a Taylor series approximation for a given expression
        /// </summary>
        /// <param name="expression">The expression to approximate</param>
        /// <param name="variable">The variable to expand around</param>
        /// <param name="expandAroundPoint">The point to expand around</param>
        /// <param name="evaluateAtPoint">The point to evaluate the series at</param>
        /// <param name="terms">Number of terms to include</param>
        /// <returns>The Taylor series approximation</returns>
        public double CalculateTaylorSeries(string expression, string variable, double expandAroundPoint, double evaluateAtPoint, int terms)
        {
            if (terms < 1)
            {
                throw new ArgumentException("Number of terms must be at least 1", nameof(terms));
            }

            // Create a function that evaluates the expression for different values of the variable
            Func<double, double> function = x =>
            {
                this.SetVariable(variable, x);
                return this.Solve(expression, true); // Use normal arithmetic for derivatives
            };

            // Calculate the Taylor series
            double result = _functionEvaluator.TaylorSeries(function, expandAroundPoint, evaluateAtPoint, terms);

            // Format the result according to arithmetic settings
            return _arithmeticHandler.FormatNumber(result);
        }
    }
}