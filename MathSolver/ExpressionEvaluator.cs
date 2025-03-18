using System;
using System.Collections.Generic;

namespace MathSolver
{
    /// <summary>
    /// Evaluates mathematical expressions in postfix or prefix notation
    /// </summary>
    public class ExpressionEvaluator
    {
        private readonly ArithmeticHandler _arithmeticHandler;
        private readonly FunctionEvaluator _functionEvaluator;
        private readonly Dictionary<string, double> _variables;

        /// <summary>
        /// Creates a new expression evaluator
        /// </summary>
        /// <param name="arithmeticHandler">Handler for arithmetic operations and formatting</param>
        /// <param name="functionEvaluator">Evaluator for mathematical functions</param>
        public ExpressionEvaluator(ArithmeticHandler arithmeticHandler, FunctionEvaluator functionEvaluator)
        {
            _arithmeticHandler = arithmeticHandler ?? throw new ArgumentNullException(nameof(arithmeticHandler));
            _functionEvaluator = functionEvaluator ?? throw new ArgumentNullException(nameof(functionEvaluator));
            _variables = new Dictionary<string, double>();
        }

        /// <summary>
        /// Sets a variable value for use in expressions
        /// </summary>
        /// <param name="name">Variable name</param>
        /// <param name="value">Variable value</param>
        public void SetVariable(string name, double value)
        {
            _variables[name] = value;
        }

        /// <summary>
        /// Clears all variables
        /// </summary>
        public void ClearVariables()
        {
            _variables.Clear();
        }

        /// <summary>
        /// Evaluates an expression in either postfix or prefix notation and collects calculation steps
        /// </summary>
        /// <param name="tokens">Tokens in either postfix or prefix notation</param>
        /// <param name="steps">Collection to store calculation steps</param>
        /// <param name="isPrefix">Whether tokens are in prefix (true) or postfix (false) notation</param>
        /// <returns>Result of the evaluation</returns>
        public double Evaluate(List<ExpressionToken> tokens, List<CalculationStep> steps, bool isPrefix)
        {
            // Handle empty expression
            if (tokens == null || tokens.Count == 0)
            {
                return 0;
            }

            // Handle single summation token
            if (tokens.Count == 1 && tokens[0].Type == TokenType.Summation)
            {
                return EvaluateSummation(tokens[0], steps,
                    isPrefix ? CalculationDirection.RightToLeft : CalculationDirection.LeftToRight);
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

                switch (token.Type)
                {
                    case TokenType.Number:
                        var value = double.Parse(token.Value);
                        operandStack.Push(value);
                        expressionStack.Push(token.Value);
                        break;

                    case TokenType.Variable:
                        if (_variables.TryGetValue(token.Value, out double varValue))
                        {
                            operandStack.Push(varValue);
                            expressionStack.Push(token.Value);
                        }
                        else
                        {
                            throw new InvalidOperationException($"Variable '{token.Value}' is not defined");
                        }
                        break;

                    case TokenType.Factorial:
                        // Parse the value to calculate factorial
                        double n = double.Parse(token.Value);

                        // Factorial only works on integers
                        if (n != Math.Floor(n) || n < 0)
                        {
                            throw new ArgumentException("Factorial is only defined for non-negative integers");
                        }

                        var stepExpr = $"{n}!";
                        var factorialResult = _arithmeticHandler.Factorial((int)n);

                        steps?.Add(new CalculationStep(stepExpr, factorialResult));

                        operandStack.Push(factorialResult);
                        expressionStack.Push(factorialResult.ToString());
                        break;

                    case TokenType.Summation:
                        // Evaluate the summation and push the result onto the stack
                        var summationResult = EvaluateSummation(token, steps,
                            isPrefix ? CalculationDirection.RightToLeft : CalculationDirection.LeftToRight);
                        operandStack.Push(summationResult);
                        expressionStack.Push($"({summationResult})"); // Wrap in parentheses for clarity
                        break;

                    case TokenType.Function:
                        // Get the number of arguments - this would need to be predefined or parsed
                        int argCount = 1; // Default to 1 for most functions

                        if (operandStack.Count < argCount || expressionStack.Count < argCount)
                        {
                            throw new InvalidOperationException($"Not enough arguments for function {token.Value}");
                        }

                        // Get the arguments
                        double[] args = new double[argCount];
                        string[] argStrings = new string[argCount];

                        for (int j = 0; j < argCount; j++)
                        {
                            args[argCount - j - 1] = operandStack.Pop();
                            argStrings[argCount - j - 1] = expressionStack.Pop();
                        }

                        // Evaluate the function
                        double functionResult = _functionEvaluator.Evaluate(token.Value, args);
                        functionResult = _arithmeticHandler.FormatNumber(functionResult);

                        // Create step expression
                        string functionExpr = $"{token.Value}({string.Join(", ", argStrings)})";

                        steps?.Add(new CalculationStep(functionExpr, functionResult));

                        operandStack.Push(functionResult);
                        expressionStack.Push(functionResult.ToString());
                        break;

                    case TokenType.Operator:
                        if (operandStack.Count < 2 || expressionStack.Count < 2)
                        {
                            throw new InvalidOperationException("Invalid expression format - not enough operands");
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

                        var stepResult = _arithmeticHandler.PerformOperation(a, b, token.Value);

                        steps?.Add(new CalculationStep($"{aStr} {token.Value} {bStr}", stepResult));

                        operandStack.Push(stepResult);
                        expressionStack.Push(stepResult.ToString());
                        break;
                }
            }

            if (operandStack.Count != 1)
            {
                throw new InvalidOperationException("Invalid expression - final result stack should contain exactly one value");
            }

            return operandStack.Pop();
        }

        /// <summary>
        /// Evaluates a summation token and collects calculation steps
        /// </summary>
        /// <param name="token">The summation token to evaluate</param>
        /// <param name="steps">Collection to store calculation steps</param>
        /// <param name="direction">Direction for evaluating the summation</param>
        /// <returns>Result of the summation</returns>
        private double EvaluateSummation(ExpressionToken token, List<CalculationStep> steps, CalculationDirection direction)
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

            // Iterate through each term in the summation
            for (int i = start; leftToRight ? i <= end : i >= end; i += step)
            {
                // Save the original variable value if it exists
                bool hasVariable = _variables.TryGetValue("i", out double originalValue);

                // Set the counter variable
                _variables["i"] = i;

                // Create a temporary evaluation context
                var tempEvaluator = new ExpressionParser(); // You'd need this class to parse the term
                var termTokens = tempEvaluator.Tokenize(sumExpr);
                var tokenProcessor = new TokenProcessor();
                var postfixTokens = tokenProcessor.ConvertToPostfix(termTokens);

                // Evaluate the term
                double termValue = Evaluate(postfixTokens, null, false);

                // Format according to our arithmetic settings
                double formattedTermValue = _arithmeticHandler.FormatNumber(termValue);

                // Keep previous sum for display
                double previousSum = totalSum;

                // Add to our running sum
                totalSum += formattedTermValue;

                // Format the running sum
                totalSum = _arithmeticHandler.FormatNumber(totalSum);

                // Add this step to our result with expanded format
                string stepDescription = leftToRight
                    ? $"Sum_{{{lowerBound}}}^{{{i}}} {sumExpr} = {previousSum} + {formattedTermValue} = {totalSum}"
                    : $"Sum_{{{i}}}^{{{upperBound}}} {sumExpr} = {previousSum} + {formattedTermValue} = {totalSum}";

                steps?.Add(new CalculationStep(stepDescription, totalSum));

                // Restore the original variable value if it existed
                if (hasVariable)
                {
                    _variables["i"] = originalValue;
                }
                else
                {
                    _variables.Remove("i");
                }
            }

            return totalSum;
        }
    }
}