using System.Text.RegularExpressions;

namespace MathSolver
{
    /// <summary>
    /// A token in a mathematical expression
    /// </summary>
    /// <param name="Type"></param>
    /// <param name="Value"></param>
    /// <param name="Precedence"></param>
    public record ExpressionToken(TokenType Type, string Value, int Precedence = 0);

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
            var tokens = new List<ExpressionToken>();

            // Split the expression by the SUM keyword to identify summations
            var parts = Regex.Split(expression, @"(SUM\(\d+,\d+,[^)]+\))");

            foreach (var part in parts)
            {
                if (string.IsNullOrWhiteSpace(part))
                    continue;

                // Check if this part is a summation
                var sumRegex = new Regex(@"SUM\((\d+),(\d+),(.+)\)");
                var match = sumRegex.Match(part);

                if (match.Success)
                {
                    // This is a summation expression
                    int lowerBound = int.Parse(match.Groups[1].Value);
                    int upperBound = int.Parse(match.Groups[2].Value);
                    string sumExpr = match.Groups[3].Value;

                    // Create a special summation token
                    tokens.Add(new ExpressionToken(TokenType.Summation, $"{lowerBound},{upperBound},{sumExpr}"));
                }
                else
                {
                    // Tokenize regular mathematical expression
                    string pattern = @"(\d+\.\d+|\d+|\+|\-|\*|\/|\^|\(|\)|!)";
                    var matches = Regex.Matches(part, pattern);

                    for (int i = 0; i < matches.Count; i++)
                    {
                        var value = matches[i].Value;

                        if (value == "!" && i > 0 && double.TryParse(matches[i - 1].Value, out double factorialNum))
                        {
                            // Remove the previously added number token
                            tokens.RemoveAt(tokens.Count - 1);

                            // Add as a factorial token
                            tokens.Add(new ExpressionToken(TokenType.Factorial, factorialNum.ToString(), 4));
                        }
                        else if (double.TryParse(value, out _))
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
                }

                // Also check for \factorial{n} notation
                var factorialRegex = new Regex(@"\\factorial\{([^{}]*)\}");
                var factMatches = factorialRegex.Matches(part);

                foreach (Match m in factMatches)
                {
                    if (m.Success)
                    {
                        string factArg = m.Groups[1].Value;
                        // For \factorial{n}, we'll first evaluate n
                        tokens.Add(new ExpressionToken(TokenType.Factorial, factArg));
                    }
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
            var postfix = new List<ExpressionToken>();
            var operatorStack = new Stack<ExpressionToken>();

            foreach (var token in infixTokens)
            {
                switch (token.Type)
                {
                    case TokenType.Number:
                    case TokenType.Factorial:
                    case TokenType.Summation:
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
            var reversedTokens = new List<ExpressionToken>(infixTokens);
            reversedTokens.Reverse();

            // Swap parentheses
            for (var i = 0; i < reversedTokens.Count; i++)
            {
                if (reversedTokens[i].Type == TokenType.OpenParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.CloseParenthesis, ")");
                }
                else if (reversedTokens[i].Type == TokenType.CloseParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.OpenParenthesis, "(");
                }
                // Keep summation tokens as they are
            }

            // Convert to postfix
            var postfix = ConvertToPostfix(reversedTokens);

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
        /// Evaluates an expression in either postfix or prefix notation and collects steps
        /// </summary>
        /// <param name="tokens">Tokens in either postfix or prefix notation</param>
        /// <param name="result">The calculation result to fill with steps</param>
        /// <param name="isPrefix">Whether tokens are in prefix (true) or postfix (false) notation</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluateTokens(List<ExpressionToken> tokens, CalculationResult result, bool isPrefix)
        {
            // Handle single summation token
            if (tokens.Count == 1 && tokens[0].Type == TokenType.Summation)
            {
                return EvaluateSummation(tokens[0],
                    isPrefix ? CalculationDirection.RightToLeft : CalculationDirection.LeftToRight,
                    result);
            }

            var operandStack = new Stack<double>();
            var expressionStack = new Stack<string>(); // For tracking string representations

            // For prefix, we need to process tokens in reverse order
            int start = isPrefix ? tokens.Count - 1 : 0;
            int end = isPrefix ? -1 : tokens.Count;
            int step = isPrefix ? -1 : 1;

            for (int i = start; i != end; i += step)
            {
                var token = tokens[i];

                // Token type processing stays the same
                if (token.Type == TokenType.Factorial)
                {
                    // The factorial token already contains the value in its Value property
                    double n = double.Parse(token.Value);

                    // Factorial only works on integers
                    if (n != Math.Floor(n) || n < 0)
                    {
                        throw new ArgumentException("Factorial is only defined for non-negative integers");
                    }

                    var stepExpr = $"{n}!";
                    var factorialResult = Factorial((int)n);

                    // Format the factorial result
                    var stepResult = FormatNumber(factorialResult);

                    result.Steps.Add(new CalculationStep(stepExpr, stepResult));

                    operandStack.Push(stepResult);
                    expressionStack.Push(stepResult.ToString());
                }
                else if (token.Type == TokenType.Number)
                {
                    var value = double.Parse(token.Value);
                    operandStack.Push(value);
                    expressionStack.Push(token.Value);
                }
                else if (token.Type == TokenType.Summation)
                {
                    // Evaluate the summation and push the result onto the stack
                    var summationResult = EvaluateSummation(token, CalculationDirection.LeftToRight, result);
                    operandStack.Push(summationResult);
                    expressionStack.Push($"({summationResult})"); // Wrap in parentheses for clarity
                }
                else if (token.Type == TokenType.Operator)
                {
                    if (operandStack.Count < 2 || expressionStack.Count < 2)
                    {
                        throw new Exception("Invalid expression format");
                    }

                    // Get operands - note the order is important
                    var b = operandStack.Pop();
                    var a = operandStack.Pop();

                    var bStr = expressionStack.Pop();
                    var aStr = expressionStack.Pop();

                    // For prefix notation with right-to-left calculation, we need to swap operands for 
                    // non-commutative operations
                    if (isPrefix && (token.Value == "-" || token.Value == "/"))
                    {
                        var temp = a;
                        a = b;
                        b = temp;

                        var tempStr = aStr;
                        aStr = bStr;
                        bStr = tempStr;
                    }

                    var stepExpr = $"{aStr} {token.Value} {bStr}";
                    var stepResult = PerformOperation(a, b, token.Value);

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
                 /// Evaluates an expression in postfix notation (left-to-right) and collects steps
                 /// </summary>
                 /// <param name="postfixTokens">Tokens in postfix notation</param>
                 /// <param name="result">The calculation result to fill with steps</param>
                 /// <returns>Result of the evaluation</returns>
        private double EvaluatePostfix(List<ExpressionToken> postfixTokens, CalculationResult result)
        {
            return EvaluateTokens(postfixTokens, result, false);
        }

        /// <summary>
        /// Evaluates an expression in prefix notation (right-to-left) and collects steps
        /// </summary>
        /// <param name="prefixTokens">Tokens in prefix notation</param>
        /// <param name="result">The calculation result to fill with steps</param>
        /// <returns>Result of the evaluation</returns>
        private double EvaluatePrefix(List<ExpressionToken> prefixTokens, CalculationResult result)
        {
            return EvaluateTokens(prefixTokens, result, true);
        }

        private double EvaluateSummation(ExpressionToken token, CalculationDirection direction, CalculationResult result)
        {
            var parts = token.Value.Split(',', 3);
            int lowerBound = int.Parse(parts[0]);
            int upperBound = int.Parse(parts[1]);
            string sumExpr = parts[2];

            // Total result of the summation
            double totalSum = 0;

            // Determine the iteration order based on direction
            bool leftToRight = direction == CalculationDirection.LeftToRight;
            int start = leftToRight ? lowerBound : upperBound;
            int end = leftToRight ? upperBound : lowerBound;
            int step = leftToRight ? 1 : -1;

            // Create a temporary solver for evaluating each term
            var tempSolver = new StepByStepMathSolver(_arithmeticType, _precision, _useSignificantDigits, direction);

            // Iterate through each term in the summation
            for (int i = start; leftToRight ? i <= end : i >= end; i += step)
            {
                // Replace i in the expression
                string termExpr = sumExpr.Replace("i", i.ToString());

                // Evaluate the term
                double termValue = tempSolver.Solve(termExpr);

                // Format according to our arithmetic settings
                double formattedTermValue = FormatNumber(termValue);

                // Keep previous sum for display
                double previousSum = totalSum;

                // Add to our running sum
                totalSum += formattedTermValue;

                // Format the running sum according to our arithmetic settings
                totalSum = FormatNumber(totalSum);

                // Add this step to our result with expanded format
                string stepDescription = leftToRight
                    ? $"Sum_{{{lowerBound}}}^{{{i}}} {sumExpr} = {previousSum} + {formattedTermValue} = {totalSum}"
                    : $"Sum_{{{i}}}^{{{upperBound}}} {sumExpr} = {previousSum} + {formattedTermValue} = {totalSum}";

                result.Steps.Add(new CalculationStep(stepDescription, totalSum));
            }

            return totalSum;
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

            var absValue = Math.Abs(value);
            var sign = value < 0 ? -1 : 1;

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

            var absValue = Math.Abs(value);
            var sign = value < 0 ? -1 : 1;

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
            var expr = latexString.Trim();

            // Extract summation expressions first to handle them separately
            var summationRegex = new Regex(@"\\sum_\{i=(\d+)\}\^\{(\d+)\} (.+)");
            var match = summationRegex.Match(expr);

            if (match.Success)
            {
                // Extract the lower bound, upper bound, and expression to sum
                int lowerBound = int.Parse(match.Groups[1].Value);
                int upperBound = int.Parse(match.Groups[2].Value);
                string sumExpr = match.Groups[3].Value;

                // Validate that the summation bounds are valid
                if (lowerBound > upperBound)
                {
                    throw new ArgumentException("Invalid summation bounds: lower bound must be less than or equal to upper bound");
                }

                // Return a special format that we'll handle in our tokenizer
                return $"SUM({lowerBound},{upperBound},{sumExpr})";
            }

            // Handle other LaTeX expressions as before
            expr = Regex.Replace(expr, @"\\frac\{([^{}]*)\}\{([^{}]*)\}", "($1)/($2)");
            expr = expr.Replace("\\cdot", "*");
            expr = expr.Replace("\\times", "*");
            expr = expr.Replace("\\div", "/");
            expr = Regex.Replace(expr, @"\\sqrt\{([^{}]*)\}", "sqrt($1)");
            expr = expr.Replace("\\pi", Math.PI.ToString());

            // Handle LaTeX exponents
            expr = Regex.Replace(expr, @"\^{([^{}]*)}", "^($1)");

            // Handle \factorial{n} notation
            expr = Regex.Replace(expr, @"\\factorial\{([^{}]*)\}", "($1)!");

            return expr;
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
                var actualResult = normalSolver.Solve(latexExpression);
                result.ActualResult = actualResult;

                // Tokenize the expression
                var tokens = Tokenize(standardExpr);

                // Evaluate based on the direction and collect steps
                double formattedResult;
                if (_direction == CalculationDirection.LeftToRight)
                {
                    var postfixTokens = ConvertToPostfix(tokens);
                    formattedResult = EvaluatePostfix(postfixTokens, result);
                }
                else // RightToLeft
                {
                    var prefixTokens = ConvertToPrefix(tokens);
                    formattedResult = EvaluatePrefix(prefixTokens, result);
                }

                result.FormattedResult = formattedResult;
                return result;
            }
            catch (Exception ex)
            {
                throw new Exception($"Error solving expression: {ex.Message}", ex);
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
                var tokens = Tokenize(standardExpr);

                // Evaluate based on the direction
                if (_direction == CalculationDirection.LeftToRight)
                {
                    var postfixTokens = ConvertToPostfix(tokens);
                    return EvaluatePostfix(postfixTokens, new CalculationResult());
                }
                else // RightToLeft
                {
                    var prefixTokens = ConvertToPrefix(tokens);
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

        /// <summary>
        /// Calculates the factorial of an integer
        /// </summary>
        /// <param name="n">The number to calculate factorial for</param>
        /// <returns>The factorial result</returns>
        private double Factorial(int n)
        {
            if (n < 0)
            {
                throw new ArgumentException("Factorial is not defined for negative numbers");
            }

            if (n == 0 || n == 1)
            {
                return 1;
            }

            double result = 1;
            for (int i = 2; i <= n; i++)
            {
                result *= i;
            }

            return result;
        }
    }
}