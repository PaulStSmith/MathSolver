using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Linq;

namespace MathSolver
{
    /// <summary>
    /// Parses mathematical expressions in LaTeX and standard formats into tokens
    /// </summary>
    public class ExpressionParser
    {
        private readonly HashSet<string> _knownFunctions;
        private readonly TokenProcessor _tokenProcessor;

        /// <summary>
        /// Creates a new expression parser with default function recognition
        /// </summary>
        public ExpressionParser()
        {
            _knownFunctions = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                // Basic functions
                "abs", "sqrt", "cbrt", "pow", "exp", "ln", "log",
                
                // Trigonometric functions
                "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
                
                // Hyperbolic functions
                "sinh", "cosh", "tanh",
                
                // Rounding functions
                "floor", "ceil", "round", "trunc",
                
                // Statistics functions
                "min", "max", "sum", "avg",
                
                // Special functions
                "factorial", "gcd", "lcm"
            };

            _tokenProcessor = new TokenProcessor();
        }

        /// <summary>
        /// Registers a custom function name for recognition during parsing
        /// </summary>
        /// <param name="functionName">Name of the function to register</param>
        public void RegisterFunction(string functionName)
        {
            if (string.IsNullOrWhiteSpace(functionName))
            {
                throw new ArgumentException("Function name cannot be empty", nameof(functionName));
            }

            _knownFunctions.Add(functionName);
        }

        /// <summary>
        /// Converts a LaTeX string to a standard mathematical expression
        /// </summary>
        /// <param name="latexString">The LaTeX expression to convert</param>
        /// <returns>A standard mathematical expression</returns>
        public string ConvertLatexToStandard(string latexString)
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

            // Handle various LaTeX expressions
            expr = Regex.Replace(expr, @"\\frac\{([^{}]*)\}\{([^{}]*)\}", "($1)/($2)");
            expr = expr.Replace("\\cdot", "*");
            expr = expr.Replace("\\times", "*");
            expr = expr.Replace("\\div", "/");

            // Handle square roots and nth roots
            expr = Regex.Replace(expr, @"\\sqrt\{([^{}]*)\}", "sqrt($1)");
            expr = Regex.Replace(expr, @"\\sqrt\[(\d+)\]\{([^{}]*)\}", (match) =>
            {
                string rootPower = match.Groups[1].Value;
                string rootArgument = match.Groups[2].Value;
                return $"pow($2, 1/{rootPower})";
            });

            // Replace constants
            expr = expr.Replace("\\pi", Math.PI.ToString());
            expr = expr.Replace("\\e", Math.E.ToString());

            // Handle LaTeX exponents
            expr = Regex.Replace(expr, @"\^{([^{}]*)}", "^($1)");
            expr = Regex.Replace(expr, @"([a-zA-Z0-9)]})\^(\d+)", "$1^$2");

            // Handle \factorial{n} notation
            expr = Regex.Replace(expr, @"\\factorial\{([^{}]*)\}", "factorial($1)");

            // Handle standard functions
            foreach (var function in _knownFunctions)
            {
                // Convert \sin{x} to sin(x)
                expr = Regex.Replace(expr, $@"\\{function}\{{([^{{}}]*)\}}", $"{function}($1)");

                // Convert \sin x to sin(x) - for cases without braces
                expr = Regex.Replace(expr, $@"\\{function} ([a-zA-Z0-9]+)", $"{function}($1)");
            }

            // Handle functions with multiple arguments like \min{a, b, c}
            expr = Regex.Replace(expr, @"\\min\{([^{}]*)\}", "min($1)");
            expr = Regex.Replace(expr, @"\\max\{([^{}]*)\}", "max($1)");

            // Handle subscripts for variables (ignore them for now - could be enhanced)
            expr = Regex.Replace(expr, @"([a-zA-Z])_(\d+)", "$1$2");

            return expr;
        }

        /// <summary>
        /// Tokenizes a mathematical expression for evaluation
        /// </summary>
        /// <param name="expression">The expression to tokenize</param>
        /// <returns>A list of tokens</returns>
        public List<ExpressionToken> Tokenize(string expression)
        {
            // First, we need to check if the expression is in LaTeX format 
            // and convert it to standard notation
            bool isLaTeX = expression.Contains("\\") ||
                           expression.Contains("\\frac") ||
                           expression.Contains("\\sum") ||
                           expression.Contains("\\factorial");

            if (isLaTeX)
            {
                expression = ConvertLatexToStandard(expression);
            }

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
                    // First extract function calls to avoid confusion with parentheses
                    var functionCalls = ExtractFunctionCalls(part);
                    var processedPart = part;

                    // Replace function calls with placeholders
                    foreach (var kvp in functionCalls)
                    {
                        processedPart = processedPart.Replace(kvp.Key, kvp.Value.Placeholder);
                    }

                    // Tokenize the expression
                    TokenizeExpression(processedPart, tokens);

                    // Now replace the function placeholders with actual function tokens
                    ReplaceDigestFunctionCalls(tokens, functionCalls);
                }
            }

            return tokens;
        }

        /// <summary>
        /// Represents a detected function call in the expression
        /// </summary>
        private class FunctionCall
        {
            public string FunctionName { get; }
            public string Arguments { get; }
            public string Placeholder { get; }

            public FunctionCall(string functionName, string arguments, string placeholder)
            {
                FunctionName = functionName;
                Arguments = arguments;
                Placeholder = placeholder;
            }
        }

        /// <summary>
        /// Extracts function calls from an expression to avoid confusion with parentheses
        /// </summary>
        /// <param name="expression">The expression to analyze</param>
        /// <returns>Dictionary mapping original function calls to function information</returns>
        private Dictionary<string, FunctionCall> ExtractFunctionCalls(string expression)
        {
            var result = new Dictionary<string, FunctionCall>();
            var functionRegex = new Regex(@"([a-zA-Z][a-zA-Z0-9_]*)\(([^()]*(?:\([^()]*\)[^()]*)*)\)");
            var matches = functionRegex.Matches(expression);

            for (int i = 0; i < matches.Count; i++)
            {
                var match = matches[i];
                string functionName = match.Groups[1].Value;
                string arguments = match.Groups[2].Value;
                string originalCall = match.Value;

                if (_knownFunctions.Contains(functionName))
                {
                    string placeholder = $"#FUNC_{i}#";
                    result[originalCall] = new FunctionCall(functionName, arguments, placeholder);
                }
            }

            return result;
        }

        /// <summary>
        /// Replaces function placeholders with actual function tokens
        /// </summary>
        /// <param name="tokens">The token list being built</param>
        /// <param name="functionCalls">Dictionary of function calls</param>
        private void ReplaceDigestFunctionCalls(List<ExpressionToken> tokens, Dictionary<string, FunctionCall> functionCalls)
        {
            // We need to find placeholders in the token list and replace them
            for (int i = 0; i < tokens.Count; i++)
            {
                if (tokens[i].Type == TokenType.Number && tokens[i].Value.StartsWith("#FUNC_"))
                {
                    var placeholder = tokens[i].Value;
                    var functionCall = functionCalls.Values.FirstOrDefault(f => f.Placeholder == placeholder);

                    if (functionCall != null)
                    {
                        // Replace the placeholder token with a function token
                        tokens[i] = _tokenProcessor.CreateFunctionToken(functionCall.FunctionName);

                        // Parse the arguments
                        var argTokens = TokenizeArgumentList(functionCall.Arguments);

                        // Insert the argument tokens after the function token
                        tokens.InsertRange(i + 1, argTokens);
                        i += argTokens.Count; // Skip over the newly inserted tokens
                    }
                }
            }
        }

        /// <summary>
        /// Tokenizes a function argument list
        /// </summary>
        /// <param name="arguments">Comma-separated argument list</param>
        /// <returns>List of tokens for the arguments</returns>
        private List<ExpressionToken> TokenizeArgumentList(string arguments)
        {
            var result = new List<ExpressionToken>();

            // Split by commas, respecting parentheses
            var argList = SplitArguments(arguments);

            foreach (var arg in argList)
            {
                // Tokenize each argument and add it to the list
                var argTokens = Tokenize(arg.Trim());
                result.AddRange(argTokens);

                // Add a comma token between arguments (represented as a special operator)
                if (arg != argList.Last())
                {
                    result.Add(new ExpressionToken(TokenType.Operator, ",", 0));
                }
            }

            return result;
        }

        /// <summary>
        /// Splits a function argument list by commas, respecting parentheses
        /// </summary>
        /// <param name="arguments">The comma-separated argument list</param>
        /// <returns>Array of individual arguments</returns>
        private string[] SplitArguments(string arguments)
        {
            var result = new List<string>();
            int parenLevel = 0;
            int startIndex = 0;

            for (int i = 0; i < arguments.Length; i++)
            {
                char c = arguments[i];

                if (c == '(')
                {
                    parenLevel++;
                }
                else if (c == ')')
                {
                    parenLevel--;
                }
                else if (c == ',' && parenLevel == 0)
                {
                    // Found an argument separator
                    result.Add(arguments.Substring(startIndex, i - startIndex));
                    startIndex = i + 1;
                }
            }

            // Add the last argument
            if (startIndex < arguments.Length)
            {
                result.Add(arguments.Substring(startIndex));
            }

            return result.ToArray();
        }

        /// <summary>
        /// Tokenizes a standard mathematical expression
        /// </summary>
        /// <param name="expression">The expression to tokenize</param>
        /// <param name="tokens">The token list to populate</param>
        private void TokenizeExpression(string expression, List<ExpressionToken> tokens)
        {
            string pattern = @"(\d+\.\d+|\d+|\+|\-|\*|\/|\^|\(|\)|!|[a-zA-Z][a-zA-Z0-9_]*|#FUNC_\d+#|,)";
            var matches = Regex.Matches(expression, pattern);

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
                else if (value.StartsWith("#FUNC_"))
                {
                    // Function placeholder, add as is for now
                    tokens.Add(new ExpressionToken(TokenType.Number, value));
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
                else if (value == "+" || value == "-" || value == "*" || value == "/" || value == "^" || value == ",")
                {
                    // Handle unary minus specially
                    if (value == "-" && (i == 0 || tokens.Count == 0 ||
                        (tokens[tokens.Count - 1].Type != TokenType.Number &&
                         tokens[tokens.Count - 1].Type != TokenType.CloseParenthesis)))
                    {
                        // This is a unary minus, add a zero before it
                        tokens.Add(new ExpressionToken(TokenType.Number, "0"));
                    }

                    tokens.Add(_tokenProcessor.CreateOperatorToken(value));
                }
                else if (Regex.IsMatch(value, @"^[a-zA-Z][a-zA-Z0-9_]*$"))
                {
                    // This is a variable or constant
                    if (value.Equals("pi", StringComparison.OrdinalIgnoreCase))
                    {
                        tokens.Add(new ExpressionToken(TokenType.Number, Math.PI.ToString()));
                    }
                    else if (value.Equals("e", StringComparison.OrdinalIgnoreCase))
                    {
                        tokens.Add(new ExpressionToken(TokenType.Number, Math.E.ToString()));
                    }
                    else
                    {
                        tokens.Add(new ExpressionToken(TokenType.Variable, value));
                    }
                }
            }
        }
    }
}