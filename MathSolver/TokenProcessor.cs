using System;
using System.Collections.Generic;

namespace MathSolver
{
    /// <summary>
    /// Processes tokens for mathematical expressions by converting between infix, postfix, and prefix notations
    /// </summary>
    public class TokenProcessor
    {
        /// <summary>
        /// Converts an infix expression to postfix (Reverse Polish Notation)
        /// Used for left-to-right evaluation
        /// </summary>
        /// <param name="infixTokens">Tokens in infix notation</param>
        /// <returns>Tokens in postfix notation</returns>
        public List<ExpressionToken> ConvertToPostfix(List<ExpressionToken> infixTokens)
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
                    case TokenType.Variable:
                        postfix.Add(token);
                        break;

                    case TokenType.Function:
                        operatorStack.Push(token);
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
                        else
                        {
                            throw new InvalidOperationException("Mismatched parentheses in expression");
                        }

                        // If the top of the stack is a function token, pop it to the output queue
                        if (operatorStack.Count > 0 && operatorStack.Peek().Type == TokenType.Function)
                        {
                            postfix.Add(operatorStack.Pop());
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
                    throw new InvalidOperationException("Mismatched parentheses in expression");
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
        public List<ExpressionToken> ConvertToPrefix(List<ExpressionToken> infixTokens)
        {
            // Create a copy of the infix tokens and reverse them
            var reversedTokens = new List<ExpressionToken>(infixTokens);
            reversedTokens.Reverse();

            // Swap parentheses
            for (var i = 0; i < reversedTokens.Count; i++)
            {
                var token = reversedTokens[i];
                if (token.Type == TokenType.OpenParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.CloseParenthesis, ")", token.Precedence);
                }
                else if (token.Type == TokenType.CloseParenthesis)
                {
                    reversedTokens[i] = new ExpressionToken(TokenType.OpenParenthesis, "(", token.Precedence);
                }
            }

            // Convert to postfix
            var postfix = ConvertToPostfix(reversedTokens);

            // Reverse to get prefix
            postfix.Reverse();

            return postfix;
        }

        /// <summary>
        /// Creates a token with the appropriate type and precedence based on the operator string
        /// </summary>
        /// <param name="op">The operator string</param>
        /// <returns>An ExpressionToken with the correct type and precedence</returns>
        public ExpressionToken CreateOperatorToken(string op)
        {
            int precedence = GetOperatorPrecedence(op);
            return new ExpressionToken(TokenType.Operator, op, precedence);
        }

        /// <summary>
        /// Creates a function token with the appropriate precedence
        /// </summary>
        /// <param name="functionName">The function name</param>
        /// <returns>An ExpressionToken representing the function</returns>
        public ExpressionToken CreateFunctionToken(string functionName)
        {
            // Functions have high precedence, typically higher than operators
            return new ExpressionToken(TokenType.Function, functionName, 4);
        }

        /// <summary>
        /// Determines the precedence of an operator
        /// </summary>
        /// <param name="op">The operator string</param>
        /// <returns>The precedence level</returns>
        private int GetOperatorPrecedence(string op)
        {
            return op switch
            {
                "+" or "-" => 1,
                "*" or "/" => 2,
                "^" => 3,
                _ => 0
            };
        }
    }
}