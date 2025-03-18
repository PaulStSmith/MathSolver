using System.Text.RegularExpressions;

namespace MathSolver
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
            infixTokens.Reverse();

            // Swap parentheses
            for (int i = 0; i < infixTokens.Count; i++)
            {
                if (infixTokens[i].Type == TokenType.OpenParenthesis)
                {
                    infixTokens[i] = new ExpressionToken(TokenType.CloseParenthesis, ")");
                }
                else if (infixTokens[i].Type == TokenType.CloseParenthesis)
                {
                    infixTokens[i] = new ExpressionToken(TokenType.OpenParenthesis, "(");
                }
            }

            // Convert to postfix
            List<ExpressionToken> postfix = ConvertToPostfix(infixTokens);

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
        /// Evaluates an expression in postfix notation (left-to-right)
        /// </summary>
        /// <param name="postfixTokens">Tokens in postfix notation</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluatePostfix(List<ExpressionToken> postfixTokens)
        {
            Stack<double> operandStack = new Stack<double>();

            foreach (var token in postfixTokens)
            {
                if (token.Type == TokenType.Number)
                {
                    operandStack.Push(double.Parse(token.Value));
                }
                else if (token.Type == TokenType.Operator)
                {
                    if (operandStack.Count < 2)
                    {
                        throw new Exception("Invalid expression format");
                    }

                    double b = operandStack.Pop();
                    double a = operandStack.Pop();

                    double result = PerformOperation(a, b, token.Value);
                    operandStack.Push(result);
                }
            }

            if (operandStack.Count != 1)
            {
                throw new Exception("Invalid expression format");
            }

            return operandStack.Pop();
        }

        /// <summary>
        /// Evaluates an expression in prefix notation (right-to-left)
        /// </summary>
        /// <param name="prefixTokens">Tokens in prefix notation</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluatePrefix(List<ExpressionToken> prefixTokens)
        {
            Stack<double> operandStack = new Stack<double>();

            // Process tokens in reverse order for prefix notation
            for (int i = prefixTokens.Count - 1; i >= 0; i--)
            {
                var token = prefixTokens[i];

                if (token.Type == TokenType.Number)
                {
                    operandStack.Push(double.Parse(token.Value));
                }
                else if (token.Type == TokenType.Operator)
                {
                    if (operandStack.Count < 2)
                    {
                        throw new Exception("Invalid expression format");
                    }

                    double a = operandStack.Pop();
                    double b = operandStack.Pop();

                    double result = PerformOperation(a, b, token.Value);
                    operandStack.Push(result);
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

            // Handle simple LaTeX expressions (This is simplified for demonstration)
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
                    return EvaluatePostfix(postfixTokens);
                }
                else // RightToLeft
                {
                    List<ExpressionToken> prefixTokens = ConvertToPrefix(tokens);
                    return EvaluatePrefix(prefixTokens);
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
            // Test simple number
            string simpleNumber = "1.02500125";
            Console.WriteLine($"Expression: {simpleNumber}\n");

            TestExpressionWithAllModes(simpleNumber);

            // Test a complex expression
            string complexExpr = "2.5 * 3.6 / 1.2 + 4.8";
            Console.WriteLine($"\nExpression: {complexExpr}\n");

            Console.WriteLine("=== Left-to-Right Evaluation ===");
            TestExpressionWithAllModes(complexExpr, CalculationDirection.LeftToRight);

            Console.WriteLine("\n=== Right-to-Left Evaluation ===");
            TestExpressionWithAllModes(complexExpr, CalculationDirection.RightToLeft);

            // Test a LaTeX expression
            string latexExpr = "\\frac{2.5 + 3.7}{1.5}";
            Console.WriteLine($"\nLaTeX Expression: {latexExpr}\n");

            Console.WriteLine("=== Left-to-Right Evaluation ===");
            TestExpressionWithAllModes(latexExpr, CalculationDirection.LeftToRight);

            Console.WriteLine("\n=== Right-to-Left Evaluation ===");
            TestExpressionWithAllModes(latexExpr, CalculationDirection.RightToLeft);

            Console.ReadLine();
        }

        static void TestExpressionWithAllModes(string expression, CalculationDirection direction = CalculationDirection.LeftToRight)
        {
            // Normal arithmetic (full precision)
            var normalSolver = new StepByStepMathSolver(ArithmeticType.Normal, 10, false, direction);
            Console.WriteLine($"Normal Arithmetic: {normalSolver.SolveToString(expression)}");

            // Truncate to 3 decimal places
            var truncateDecimalSolver = new StepByStepMathSolver(ArithmeticType.Truncate, 3, false, direction);
            Console.WriteLine($"Truncate Arithmetic, 3 decimal places: {truncateDecimalSolver.SolveToString(expression)}");

            // Truncate to 3 significant digits
            var truncateSigDigitsSolver = new StepByStepMathSolver(ArithmeticType.Truncate, 3, true, direction);
            Console.WriteLine($"Truncate Arithmetic, 3 significant digits: {truncateSigDigitsSolver.SolveToString(expression)}");

            // Round to 3 decimal places
            var roundDecimalSolver = new StepByStepMathSolver(ArithmeticType.Round, 3, false, direction);
            Console.WriteLine($"Rounded Arithmetic, 3 decimal places: {roundDecimalSolver.SolveToString(expression)}");

            // Round to 3 significant digits
            var roundSigDigitsSolver = new StepByStepMathSolver(ArithmeticType.Round, 3, true, direction);
            Console.WriteLine($"Rounded Arithmetic, 3 significant digits: {roundSigDigitsSolver.SolveToString(expression)}");
        }
    }
}