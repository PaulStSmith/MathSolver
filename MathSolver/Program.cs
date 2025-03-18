using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StepByStepMathSolver
{
    /// <summary>
    /// Arithmetic modes available for calculation results
    /// </summary>
    public enum ArithmeticType
    {
        Normal,
        Truncate,
        Round
    }

    /// <summary>
    /// Direction for evaluating expressions
    /// </summary>
    public enum CalculationDirection
    {
        LeftToRight,
        RightToLeft
    }

    /// <summary>
    /// Token types for expression parsing
    /// </summary>
    public enum TokenType
    {
        Number,
        Operator,
        Function,
        OpenParenthesis,
        CloseParenthesis
    }

    /// <summary>
    /// A token in a mathematical expression
    /// </summary>
    public class ExpressionToken
    {
        public TokenType Type { get; set; }
        public string Value { get; set; }
        public int Precedence { get; set; }

        public ExpressionToken(TokenType type, string value, int precedence = 0)
        {
            Type = type;
            Value = value;
            Precedence = precedence;
        }
    }

    /// <summary>
    /// Represents a single step in a calculation
    /// </summary>
    public class CalculationStep
    {
        public string Expression { get; set; }
        public double Result { get; set; }

        public CalculationStep(string expression, double result)
        {
            Expression = expression;
            Result = result;
        }

        public override string ToString()
        {
            return $"{Expression} = {Result}";
        }
    }

    /// <summary>
    /// Contains detailed information about a calculation, including all steps
    /// </summary>
    public class CalculationResult
    {
        public string OriginalExpression { get; set; }
        public string ArithmeticMode { get; set; }
        public string PrecisionInfo { get; set; }
        public string Direction { get; set; }
        public List<CalculationStep> Steps { get; set; } = new List<CalculationStep>();
        public double ActualResult { get; set; }
        public double FormattedResult { get; set; }

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.AppendLine($"Solving: {OriginalExpression}");
            sb.AppendLine($"Mode: {ArithmeticMode}");
            sb.AppendLine($"Precision: {PrecisionInfo}");
            sb.AppendLine($"Direction: {Direction}");
            sb.AppendLine("Start Calculation");

            for (int i = 0; i < Steps.Count; i++)
            {
                sb.AppendLine($"Step {i + 1}: {Steps[i]}");
            }

            sb.AppendLine("End Calculation");
            sb.AppendLine();

            sb.AppendLine($"{ArithmeticMode}d Result: {FormattedResult}");
            sb.AppendLine($"Actual Result: {ActualResult}");

            // Calculate the error between the actual and formatted result
            double error = FormattedResult - ActualResult;
            sb.AppendLine($"Error: {error}");

            return sb.ToString();
        }
    }

    /// <summary>
    /// A solver for LaTeX mathematical expressions with configurable arithmetic and direction
    /// </summary>
    public class StepByStepMathSolver
    {
        private ArithmeticType _arithmeticType;
        private int _precision;
        private bool _useSignificantDigits;
        private CalculationDirection _direction;

        /// <summary>
        /// Creates a new LaTeX math solver with the specified arithmetic settings
        /// </summary>
        /// <param name="arithmeticType">Type of arithmetic to use</param>
        /// <param name="precision">Number of decimal places or significant digits</param>
        /// <param name="useSignificantDigits">Whether precision refers to significant digits</param>
        /// <param name="direction">Direction for evaluating expressions</param>
        public StepByStepMathSolver(
            ArithmeticType arithmeticType = ArithmeticType.Normal,
            int precision = 10,
            bool useSignificantDigits = false,
            CalculationDirection direction = CalculationDirection.LeftToRight)
        {
            SetArithmeticMode(arithmeticType, precision, useSignificantDigits);
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
            _arithmeticType = type;
            _precision = precision;
            _useSignificantDigits = useSignificantDigits;
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
        /// Tokenizes a mathematical expression for evaluation
        /// </summary>
        /// <param name="expression">The expression to tokenize</param>
        /// <returns>A list of tokens</returns>
        private List<ExpressionToken> Tokenize(string expression)
        {
            List<ExpressionToken> tokens = new List<ExpressionToken>();
            string pattern = @"(\d+\.\d+|\d+|\+|\-|\*|\/|\^|\(|\))";

            MatchCollection matches = Regex.Matches(expression, pattern);
            foreach (Match match in matches)
            {
                string value = match.Value;

                if (double.TryParse(value, out _))
                {
                    tokens.Add(new ExpressionToken(TokenType.Number, value));
                }
                else if (value == "(" || value == "[" || value == "{")
                {
                    tokens.Add(new ExpressionToken(TokenType.OpenParenthesis, "("));
                }
                else if (value == ")" || value == "]" || value == "}")
                {
                    tokens.Add(new ExpressionToken(TokenType.CloseParenthesis, ")"));
                }
                else
                {
                    int precedence = 0;
                    switch (value)
                    {
                        case "+":
                        case "-":
                            precedence = 1;
                            break;
                        case "*":
                        case "/":
                            precedence = 2;
                            break;
                        case "^":
                            precedence = 3;
                            break;
                    }

                    tokens.Add(new ExpressionToken(TokenType.Operator, value, precedence));
                }
            }

            return tokens;
        }

        /// <summary>
        /// Converts an infix expression to postfix (Reverse Polish Notation)
        /// </summary>
        /// <param name="infixTokens">Tokens in infix notation</param>
        /// <returns>Tokens in postfix notation</returns>
        private List<ExpressionToken> ConvertToPostfix(List<ExpressionToken> infixTokens)
        {
            List<ExpressionToken> postfix = new List<ExpressionToken>();
            Stack<ExpressionToken> operatorStack = new Stack<ExpressionToken>();

            foreach (var token in infixTokens)
            {
                switch (token.Type)
                {
                    case TokenType.Number:
                        postfix.Add(token);
                        break;

                    case TokenType.OpenParenthesis:
                        operatorStack.Push(token);
                        break;

                    case TokenType.CloseParenthesis:
                        while (operatorStack.Count > 0 && operatorStack.Peek().Type != TokenType.OpenParenthesis)
                        {
                            postfix.Add(operatorStack.Pop());
                        }

                        if (operatorStack.Count > 0 && operatorStack.Peek().Type == TokenType.OpenParenthesis)
                        {
                            operatorStack.Pop(); // Discard the open parenthesis
                        }
                        break;

                    case TokenType.Operator:
                        while (operatorStack.Count > 0 &&
                               operatorStack.Peek().Type != TokenType.OpenParenthesis &&
                               operatorStack.Peek().Precedence >= token.Precedence)
                        {
                            postfix.Add(operatorStack.Pop());
                        }
                        operatorStack.Push(token);
                        break;
                }
            }

            while (operatorStack.Count > 0)
            {
                if (operatorStack.Peek().Type == TokenType.OpenParenthesis)
                {
                    throw new Exception("Mismatched parentheses in expression");
                }
                postfix.Add(operatorStack.Pop());
            }

            return postfix;
        }

        /// <summary>
        /// Converts an infix expression to prefix (Polish Notation)
        /// Used for right-to-left evaluation
        /// </summary>
        /// <param name="infixTokens">Tokens in infix notation</param>
        /// <returns>Tokens in prefix notation</returns>
        private List<ExpressionToken> ConvertToPrefix(List<ExpressionToken> infixTokens)
        {
            // Reverse the infix tokens
            List<ExpressionToken> reversedTokens = new List<ExpressionToken>(infixTokens);
            reversedTokens.Reverse();

            // Swap parentheses
            for (int i = 0; i < reversedTokens.Count; i++)
            {
                if (reversedTokens[i].Type == TokenType.OpenParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.CloseParenthesis, ")");
                }
                else if (reversedTokens[i].Type == TokenType.CloseParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.OpenParenthesis, "(");
                }
            }

            // Convert to postfix
            List<ExpressionToken> postfix = ConvertToPostfix(reversedTokens);

            // Reverse to get prefix
            postfix.Reverse();

            return postfix;
        }

        /// <summary>
        /// Performs a binary operation with formatting at each step
        /// </summary>
        /// <param name="a">First operand</param>
        /// <param name="b">Second operand</param>
        /// <param name="op">Operator</param>
        /// <returns>Result of the operation</returns>
        private double PerformOperation(double a, double b, string op)
        {
            double result;

            switch (op)
            {
                case "+":
                    result = a + b;
                    break;
                case "-":
                    result = a - b;
                    break;
                case "*":
                    result = a * b;
                    break;
                case "/":
                    if (Math.Abs(b) < double.Epsilon)
                    {
                        throw new DivideByZeroException("Cannot divide by zero");
                    }
                    result = a / b;
                    break;
                case "^":
                    result = Math.Pow(a, b);
                    break;
                default:
                    throw new ArgumentException($"Unsupported operator: {op}");
            }

            // Format the result after each operation
            return FormatNumber(result);
        }

        /// <summary>
        /// Evaluates an expression in postfix notation (left-to-right) and collects steps
        /// </summary>
        /// <param name="postfixTokens">Tokens in postfix notation</param>
        /// <param name="result">The calculation result to fill with steps</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluatePostfix(List<ExpressionToken> postfixTokens, CalculationResult result)
        {
            Stack<double> operandStack = new Stack<double>();
            Stack<string> expressionStack = new Stack<string>(); // For tracking string representations

            foreach (var token in postfixTokens)
            {
                if (token.Type == TokenType.Number)
                {
                    double value = double.Parse(token.Value);
                    operandStack.Push(value);
                    expressionStack.Push(token.Value);
                }
                else if (token.Type == TokenType.Operator)
                {
                    if (operandStack.Count < 2 || expressionStack.Count < 2)
                    {
                        throw new Exception("Invalid expression format");
                    }

                    double b = operandStack.Pop();
                    double a = operandStack.Pop();

                    string bStr = expressionStack.Pop();
                    string aStr = expressionStack.Pop();

                    string stepExpr = $"{aStr} {token.Value} {bStr}";
                    double stepResult = PerformOperation(a, b, token.Value);

                    result.Steps.Add(new CalculationStep(stepExpr, stepResult));

                    operandStack.Push(stepResult);
                    expressionStack.Push(stepResult.ToString());
                }
            }

            if (operandStack.Count != 1)
            {
                throw new Exception("Invalid expression format");
            }

            return operandStack.Pop();
        }

        /// <summary>
        /// Evaluates an expression in prefix notation (right-to-left) and collects steps
        /// </summary>
        /// <param name="prefixTokens">Tokens in prefix notation</param>
        /// <param name="result">The calculation result to fill with steps</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluatePrefix(List<ExpressionToken> prefixTokens, CalculationResult result)
        {
            Stack<double> operandStack = new Stack<double>();
            Dictionary<int, string> valueMap = new Dictionary<int, string>(); // Maps stack positions to original values
            Stack<string> expressionStack = new Stack<string>(); // For tracking string representations

            // Process tokens in reverse order for prefix notation
            for (int i = prefixTokens.Count - 1; i >= 0; i--)
            {
                var token = prefixTokens[i];

                if (token.Type == TokenType.Number)
                {
                    double value = double.Parse(token.Value);
                    operandStack.Push(value);
                    expressionStack.Push(token.Value);
                }
                else if (token.Type == TokenType.Operator)
                {
                    if (operandStack.Count < 2 || expressionStack.Count < 2)
                    {
                        throw new Exception("Invalid expression format");
                    }

                    double a = operandStack.Pop();
                    double b = operandStack.Pop();

                    string aStr = expressionStack.Pop();
                    string bStr = expressionStack.Pop();

                    string stepExpr = $"{aStr} {token.Value} {bStr}";
                    double stepResult = PerformOperation(a, b, token.Value);

                    result.Steps.Add(new CalculationStep(stepExpr, stepResult));

                    operandStack.Push(stepResult);
                    expressionStack.Push(stepResult.ToString());
                }
            }

            if (operandStack.Count != 1)
            {
                throw new Exception("Invalid expression format");
            }

            return operandStack.Pop();
        }

        /// <summary>
        /// Truncates a number to a specific number of decimal places
        /// </summary>
        /// <param name="value">The number to truncate</param>
        /// <param name="decimalPlaces">Number of decimal places to keep</param>
        /// <returns>The truncated number</returns>
        private double TruncateToDecimalPlaces(double value, int decimalPlaces)
        {
            double multiplier = Math.Pow(10, decimalPlaces);
            return Math.Truncate(value * multiplier) / multiplier;
        }

        /// <summary>
        /// Truncates a number to a specific number of significant digits
        /// </summary>
        /// <param name="value">The number to truncate</param>
        /// <param name="sigDigits">Number of significant digits to keep</param>
        /// <returns>The truncated number</returns>
        private double TruncateToSignificantDigits(double value, int sigDigits)
        {
            if (Math.Abs(value) < double.Epsilon)
            {
                return 0;
            }

            double absValue = Math.Abs(value);
            int sign = value < 0 ? -1 : 1;

            // Find the magnitude of the number (position of the first significant digit)
            int magnitude = (int)Math.Floor(Math.Log10(absValue)) + 1;

            // Calculate the appropriate scaling factor
            double scale = Math.Pow(10, sigDigits - magnitude);

            // Scale, truncate, and scale back
            return sign * Math.Truncate(absValue * scale) / scale;
        }

        /// <summary>
        /// Rounds a number to a specific number of significant digits
        /// </summary>
        /// <param name="value">The number to round</param>
        /// <param name="sigDigits">Number of significant digits to keep</param>
        /// <returns>The rounded number</returns>
        private double RoundToSignificantDigits(double value, int sigDigits)
        {
            if (Math.Abs(value) < double.Epsilon)
            {
                return 0;
            }

            double absValue = Math.Abs(value);
            int sign = value < 0 ? -1 : 1;

            // Find the magnitude of the number
            int magnitude = (int)Math.Floor(Math.Log10(absValue)) + 1;

            // Calculate the appropriate scaling factor
            double scale = Math.Pow(10, sigDigits - magnitude);

            // Scale, round, and scale back
            return sign * Math.Round(absValue * scale, MidpointRounding.AwayFromZero) / scale;
        }

        /// <summary>
        /// Formats a number according to the configured arithmetic settings
        /// </summary>
        /// <param name="value">The number to format</param>
        /// <returns>The formatted number</returns>
        private double FormatNumber(double value)
        {
            if (double.IsNaN(value) || double.IsInfinity(value))
            {
                return value;
            }

            if (_arithmeticType == ArithmeticType.Normal)
            {
                return value;
            }

            if (_useSignificantDigits)
            {
                // Handle significant digits
                if (_arithmeticType == ArithmeticType.Truncate)
                {
                    return TruncateToSignificantDigits(value, _precision);
                }
                else // Round
                {
                    return RoundToSignificantDigits(value, _precision);
                }
            }
            else
            {
                // Handle decimal places
                if (_arithmeticType == ArithmeticType.Truncate)
                {
                    return TruncateToDecimalPlaces(value, _precision);
                }
                else // Round
                {
                    return Math.Round(value, _precision, MidpointRounding.AwayFromZero);
                }
            }
        }

        /// <summary>
        /// Converts a LaTeX string to a standard mathematical expression
        /// </summary>
        /// <param name="latexString">The LaTeX expression to convert</param>
        /// <returns>A standard mathematical expression</returns>
        private string ConvertLatexToStandard(string latexString)
        {
            // For simple numbers, return as is
            if (double.TryParse(latexString, out _))
            {
                return latexString;
            }

            // Handle simple LaTeX expressions 
            string expr = latexString.Trim();

            // Replace LaTeX commands with standard math equivalents
            expr = Regex.Replace(expr, @"\\frac\{([^{}]*)\}\{([^{}]*)\}", "($1)/($2)");
            expr = expr.Replace("\\cdot", "*");
            expr = expr.Replace("\\times", "*");
            expr = expr.Replace("\\div", "/");
            expr = Regex.Replace(expr, @"\\sqrt\{([^{}]*)\}", "sqrt($1)");
            expr = expr.Replace("\\pi", Math.PI.ToString());

            // Handle LaTeX exponents
            expr = Regex.Replace(expr, @"\^{([^{}]*)}", "^($1)");

            return expr;
        }

        /// <summary>
        /// Solves a LaTeX math expression with detailed step-by-step output
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>A calculation result with detailed steps</returns>
        public CalculationResult SolveWithSteps(string latexExpression)
        {
            CalculationResult result = new CalculationResult();
            result.OriginalExpression = latexExpression;
            result.ArithmeticMode = _arithmeticType.ToString();
            result.PrecisionInfo = _useSignificantDigits
                ? $"{_precision} significant digits"
                : $"{_precision} decimal places";
            result.Direction = _direction.ToString();

            try
            {
                // First, handle direct numbers (already in decimal form)
                if (double.TryParse(latexExpression, out double directValue))
                {
                    result.ActualResult = directValue;
                    result.FormattedResult = FormatNumber(directValue);
                    return result;
                }

                // Convert from LaTeX to standard mathematical notation
                string standardExpr = ConvertLatexToStandard(latexExpression);

                // For calculating the actual result without formatting
                var normalSolver = new StepByStepMathSolver(ArithmeticType.Normal, 10, false, _direction);
                double actualResult = normalSolver.Solve(latexExpression);
                result.ActualResult = actualResult;

                // Tokenize the expression
                List<ExpressionToken> tokens = Tokenize(standardExpr);

                // Evaluate based on the direction and collect steps
                double formattedResult;
                if (_direction == CalculationDirection.LeftToRight)
                {
                    List<ExpressionToken> postfixTokens = ConvertToPostfix(tokens);
                    formattedResult = EvaluatePostfix(postfixTokens, result);
                }
                else // RightToLeft
                {
                    List<ExpressionToken> prefixTokens = ConvertToPrefix(tokens);
                    formattedResult = EvaluatePrefix(prefixTokens, result);
                }

                result.FormattedResult = formattedResult;
                return result;
            }
            catch (Exception ex)
            {
                throw new Exception($"Error solving expression: {ex.Message}");
            }
        }

        /// <summary>
        /// Solves a LaTeX math expression
        /// </summary>
        /// <param name="latexExpression">The LaTeX expression to solve</param>
        /// <returns>The result of evaluating the expression</returns>
        public double Solve(string latexExpression)
        {
            try
            {
                // First, handle direct numbers (already in decimal form)
                if (double.TryParse(latexExpression, out double directValue))
                {
                    return FormatNumber(directValue);
                }

                // Convert from LaTeX to standard mathematical notation
                string standardExpr = ConvertLatexToStandard(latexExpression);

                // Tokenize the expression
                List<ExpressionToken> tokens = Tokenize(standardExpr);

                // Evaluate based on the direction
                if (_direction == CalculationDirection.LeftToRight)
                {
                    List<ExpressionToken> postfixTokens = ConvertToPostfix(tokens);
                    return EvaluatePostfix(postfixTokens, new CalculationResult());
                }
                else // RightToLeft
                {
                    List<ExpressionToken> prefixTokens = ConvertToPrefix(tokens);
                    return EvaluatePrefix(prefixTokens, new CalculationResult());
                }
            }
            catch (Exception ex)
            {
                throw new Exception($"Error solving expression: {ex.Message}");
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

                if (double.IsNaN(result))
                {
                    return "Error: Invalid expression or not a number";
                }

                if (double.IsInfinity(result))
                {
                    return result > 0 ? "Infinity" : "-Infinity";
                }

                // Format the string representation appropriately
                if (_useSignificantDigits)
                {
                    return result.ToString($"G{_precision}");
                }
                else
                {
                    string formatted = result.ToString($"F{_precision}");
                    // Trim trailing zeros and decimal point if appropriate
                    return formatted.TrimEnd('0').TrimEnd('.');
                }
            }
            catch (Exception ex)
            {
                return $"Error: {ex.Message}";
            }
        }
    }

    /// <summary>
    /// Example usage of the StepByStepMathSolver
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            // Test with the example from the requirements
            string exampleExpr = "1 + 2 + 3 * 4 + 5";
            Console.WriteLine("Example Expression Test:");

            // Create solver with right-to-left evaluation and truncate with 1 decimal place
            var solver = new StepByStepMathSolver(
                ArithmeticType.Truncate,
                1,
                false,
                CalculationDirection.RightToLeft);

            var result = solver.SolveWithSteps(exampleExpr);
            Console.WriteLine(result);
            Console.WriteLine("\n====================================\n");

            // Additional test expressions
            TestExpression("2.5 * 3.6 / 1.2 + 4.8", ArithmeticType.Truncate, 2, false);
            TestExpression("9.75 - 3.25 + 7.5 / 2.5", ArithmeticType.Round, 1, true);
            TestExpression("\\frac{2.5 + 3.7}{1.5}", ArithmeticType.Truncate, 3, false);

            Console.ReadLine();
        }

        static void TestExpression(
            string expression,
            ArithmeticType arithmeticType,
            int precision,
            bool useSignificantDigits)
        {
            Console.WriteLine($"Testing: {expression}");

            // Test left-to-right
            var leftToRightSolver = new StepByStepMathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.LeftToRight);

            var leftToRightResult = leftToRightSolver.SolveWithSteps(expression);
            Console.WriteLine("LEFT-TO-RIGHT EVALUATION:");
            Console.WriteLine(leftToRightResult);

            // Test right-to-left
            var rightToLeftSolver = new StepByStepMathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.RightToLeft);

            var rightToLeftResult = rightToLeftSolver.SolveWithSteps(expression);
            Console.WriteLine("\nRIGHT-TO-LEFT EVALUATION:");
            Console.WriteLine(rightToLeftResult);

            Console.WriteLine("\n====================================\n");
        }
    }
}