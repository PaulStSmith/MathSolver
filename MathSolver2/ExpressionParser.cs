using System.Globalization;

namespace MathSolver2;

/// <summary>
/// Parses tokens into an Abstract Syntax Tree (AST) for mathematical expressions.
/// </summary>
public class ExpressionParser
{
    private readonly Tokenizer _tokenizer;
    private readonly Dictionary<string, Func<SourcePosition, IExpressionNode>> _latexCommandHandlers;
    private Token? _currentToken;

    /// <summary>
    /// Initializes a new instance of the <see cref="ExpressionParser"/> class with the given input string.
    /// </summary>
    /// <param name="input">The input string to parse.</param>
    public ExpressionParser(string input)
    {
        _tokenizer = new Tokenizer(input);
        // Initialize the dictionary of LaTeX command handlers
        _latexCommandHandlers = new Dictionary<string, Func<SourcePosition, IExpressionNode>>(StringComparer.OrdinalIgnoreCase)
        {
            { "frac", ParseFracCommand },
            { "sum", ParseSumCommand },
            { "prod", ParseProductCommand },
            { "sqrt", ParseSqrtCommand },
            { "sin", pos => ParseSimpleFunctionCommand("sin", pos) },
            { "cos", pos => ParseSimpleFunctionCommand("cos", pos) },
            { "tan", pos => ParseSimpleFunctionCommand("tan", pos) },
            { "log", pos => ParseSimpleFunctionCommand("log", pos) },
            { "ln", pos => ParseSimpleFunctionCommand("ln", pos) },
            { "cdot", pos => ParseMultiplicationCommand(pos) },
            { "times", pos => ParseMultiplicationCommand(pos) }
        };
        NextToken(); // Initialize current token
    }

    /// <summary>
    /// Registers a new LaTeX command handler for parsing custom commands.
    /// </summary>
    /// <param name="command">The LaTeX command name (e.g., "frac").</param>
    /// <param name="handler">The handler function to process the command.</param>
    /// <exception cref="ArgumentNullException">Thrown if the handler is null.</exception>
    public void RegisterCommandHandler(string command, Func<SourcePosition, IExpressionNode> handler)
    {
        _latexCommandHandlers[command.ToLower()] = handler ?? throw new ArgumentNullException(nameof(handler));
    }

    /// <summary>
    /// Parses the input string and returns the root node of the Abstract Syntax Tree (AST).
    /// </summary>
    /// <returns>The root node of the parsed AST.</returns>
    /// <exception cref="ParserException">Thrown if there are unexpected tokens in the input.</exception>
    public IExpressionNode Parse()
    {
        IExpressionNode node = ParseExpression();

        if (_currentToken.Type != TokenType.EndOfExpression)
        {
            throw new ParserException($"Unexpected token: {_currentToken.Value}", _currentToken.Position);
        }

        return node;
    }

    /// <summary>
    /// Parses an expression, handling addition and subtraction.
    /// </summary>
    /// <returns>The parsed expression node.</returns>
    private IExpressionNode ParseExpression()
    {
        IExpressionNode left = ParseTerm();

        while (_currentToken.Type == TokenType.Plus || _currentToken.Type == TokenType.Minus)
        {
            Token operatorToken = _currentToken;
            NextToken();
            IExpressionNode right = ParseTerm();

            if (operatorToken.Type == TokenType.Plus)
            {
                left = new AdditionNode(left, right);
            }
            else
            {
                left = new SubtractionNode(left, right);
            }

            ((ExpressionNode)left).Position = new SourcePosition(
                ((ExpressionNode)left).Position.StartIndex,
                ((ExpressionNode)right).Position.EndIndex,
                operatorToken.Position.Line,
                operatorToken.Position.Column);
        }

        return left;
    }

    /// <summary>
    /// Parses a term, handling multiplication and division.
    /// </summary>
    /// <returns>The parsed term node.</returns>
    private IExpressionNode ParseTerm()
    {
        IExpressionNode left = ParseFactor();

        while (_currentToken.Type == TokenType.Multiply || _currentToken.Type == TokenType.Divide)
        {
            Token operatorToken = _currentToken;
            NextToken();
            IExpressionNode right = ParseFactor();

            if (operatorToken.Type == TokenType.Multiply)
            {
                left = new MultiplicationNode(left, right);
            }
            else
            {
                left = new DivisionNode(left, right);
            }

            ((ExpressionNode)left).Position = new SourcePosition(
                ((ExpressionNode)left).Position.StartIndex,
                ((ExpressionNode)right).Position.EndIndex,
                operatorToken.Position.Line,
                operatorToken.Position.Column);
        }

        return left;
    }

    /// <summary>
    /// Parses a factor, handling exponentiation.
    /// </summary>
    /// <returns>The parsed factor node.</returns>
    private IExpressionNode ParseFactor()
    {
        IExpressionNode left = ParsePrimary();

        if (_currentToken.Type == TokenType.Power)
        {
            Token operatorToken = _currentToken;
            NextToken();
            IExpressionNode right = ParseFactor(); // Note: right-associative

            left = new ExponentNode(left, right);

            ((ExpressionNode)left).Position = new SourcePosition(
                ((ExpressionNode)left).Position.StartIndex,
                ((ExpressionNode)right).Position.EndIndex,
                operatorToken.Position.Line,
                operatorToken.Position.Column);
        }

        return left;
    }

    /// <summary>
    /// Parses a primary expression, such as numbers, variables, or parenthesized expressions.
    /// </summary>
    /// <returns>The parsed primary expression node.</returns>
    private IExpressionNode ParsePrimary()
    {
        Token token = _currentToken;

        switch (token.Type)
        {
            case TokenType.Number:
                NextToken();
                var numberNode = new NumberNode(decimal.Parse(token.Value, CultureInfo.InvariantCulture));
                numberNode.Position = token.Position;
                return numberNode;

            case TokenType.Variable:
                NextToken();
                var variableNode = new VariableNode(token.Value);
                variableNode.Position = token.Position;
                return variableNode;

            case TokenType.LeftParenthesis:
                NextToken();
                IExpressionNode expr = ParseExpression();

                if (_currentToken.Type != TokenType.RightParenthesis)
                {
                    throw new ParserException("Expected closing parenthesis", _currentToken.Position);
                }

                NextToken();
                var parenthesisNode = new ParenthesisNode(expr);
                parenthesisNode.Position = token.Position;
                return parenthesisNode;

            case TokenType.LatexCommand:
                return ParseLatexCommand();

            case TokenType.Minus: // Unary minus
                NextToken();
                IExpressionNode factor = ParseFactor();
                // Create a subtraction with 0 as the left operand
                var zeroNode = new NumberNode(0);
                zeroNode.Position = token.Position;
                var subtractionNode = new SubtractionNode(zeroNode, factor);
                subtractionNode.Position = new SourcePosition(
                    token.Position.StartIndex,
                    ((ExpressionNode)factor).Position.EndIndex,
                    token.Position.Line,
                    token.Position.Column);
                return subtractionNode;

            default:
                throw new ParserException($"Unexpected token: {token.Value}", token.Position);
        }
    }

    /// <summary>
    /// Parses a LaTeX command and returns the corresponding expression node.
    /// </summary>
    /// <returns>The parsed LaTeX command node.</returns>
    /// <exception cref="ParserException">Thrown if the command is unsupported.</exception>
    private IExpressionNode ParseLatexCommand()
    {
        Token token = _currentToken;
        string command = token.Value;

        NextToken(); // Consume the command token

        // Check if we have a handler for this command
        if (_latexCommandHandlers.TryGetValue(command.ToLower(), out var handler))
        {
            return handler(token.Position);
        }

        throw new ParserException($"Unsupported LaTeX command: {command}", token.Position);
    }

    /// <summary>
    /// Parses a LaTeX \frac command, representing a fraction.
    /// </summary>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed fraction node.</returns>
    private IExpressionNode ParseFracCommand(SourcePosition commandPosition)
    {
        // Expect opening brace for numerator
        if (_currentToken.Type != TokenType.LeftBrace)
        {
            throw new ParserException("Expected '{' after \\frac", _currentToken.Position);
        }

        NextToken(); // Consume {

        // Parse numerator expression
        IExpressionNode numerator = ParseExpression();

        // Expect closing brace for numerator
        if (_currentToken.Type != TokenType.RightBrace)
        {
            throw new ParserException("Expected '}' after numerator in \\frac", _currentToken.Position);
        }

        NextToken(); // Consume }

        // Expect opening brace for denominator
        if (_currentToken.Type != TokenType.LeftBrace)
        {
            throw new ParserException("Expected '{' for denominator in \\frac", _currentToken.Position);
        }

        NextToken(); // Consume {

        // Parse denominator expression
        IExpressionNode denominator = ParseExpression();

        // Expect closing brace for denominator
        if (_currentToken.Type != TokenType.RightBrace)
        {
            throw new ParserException("Expected '}' after denominator in \\frac", _currentToken.Position);
        }

        NextToken(); // Consume }

        // Create division node
        var divisionNode = new DivisionNode(numerator, denominator);
        divisionNode.Position = new SourcePosition(
            commandPosition.StartIndex,
            _currentToken.Position.EndIndex,
            commandPosition.Line,
            commandPosition.Column);

        return divisionNode;
    }

    /// <summary>
    /// Parses a LaTeX \sum command, representing summation.
    /// </summary>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed summation node.</returns>
    private IExpressionNode ParseSumCommand(SourcePosition commandPosition)
    {
        // Check for subscript (_) indicating the lower bound
        if (_currentToken.Type != TokenType.Underscore)
        {
            throw new ParserException("Expected '_' after \\sum", _currentToken.Position);
        }

        NextToken(); // Consume _

        // Parse the lower bound information (variable and start value)
        IterationRange range = ParseIterationRange();

        // Check for superscript (^) indicating the upper bound
        if (_currentToken.Type != TokenType.Power)
        {
            throw new ParserException("Expected '^' after lower bound in \\sum", _currentToken.Position);
        }

        NextToken(); // Consume ^

        // Parse the upper bound expression
        IExpressionNode upperBound = ParseBracedExpression();

        // Parse the expression to sum
        IExpressionNode expression = ParseBracedExpressionOrPrimary();

        // Create summation node
        var summationNode = new SummationNode(range.Variable, range.Start, upperBound, expression);
        summationNode.Position = new SourcePosition(
            commandPosition.StartIndex,
            ((ExpressionNode)expression).Position.EndIndex,
            commandPosition.Line,
            commandPosition.Column);

        return summationNode;
    }

    /// <summary>
    /// Parses a LaTeX \prod command, representing product notation.
    /// </summary>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed product node.</returns>
    private IExpressionNode ParseProductCommand(SourcePosition commandPosition)
    {
        // Check for subscript (_) indicating the lower bound
        if (_currentToken.Type != TokenType.Underscore)
        {
            throw new ParserException("Expected '_' after \\prod", _currentToken.Position);
        }

        NextToken(); // Consume _

        // Parse the lower bound information (variable and start value)
        IterationRange range = ParseIterationRange();

        // Check for superscript (^) indicating the upper bound
        if (_currentToken.Type != TokenType.Power)
        {
            throw new ParserException("Expected '^' after lower bound in \\prod", _currentToken.Position);
        }

        NextToken(); // Consume ^

        // Parse the upper bound expression
        IExpressionNode upperBound = ParseBracedExpression();

        // Parse the expression to multiply
        IExpressionNode expression = ParseBracedExpressionOrPrimary();

        // Create product node
        var productNode = new ProductNode(range.Variable, range.Start, upperBound, expression);
        productNode.Position = new SourcePosition(
            commandPosition.StartIndex,
            ((ExpressionNode)expression).Position.EndIndex,
            commandPosition.Line,
            commandPosition.Column);

        return productNode;
    }

    /// <summary>
    /// Parses a LaTeX \sqrt command, representing a square root or general root.
    /// </summary>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed square root node.</returns>
    private IExpressionNode ParseSqrtCommand(SourcePosition commandPosition)
    {
        // Check if this is a general root with an optional order (e.g., \sqrt[3]{x})
        IExpressionNode order = null;

        if (_currentToken.Type == TokenType.LeftBracket)
        {
            NextToken(); // Consume [

            // Parse the root order
            order = ParseExpression();

            // Expect closing bracket
            if (_currentToken.Type != TokenType.RightBracket)
            {
                throw new ParserException("Expected ']' after root order in \\sqrt", _currentToken.Position);
            }

            NextToken(); // Consume ]
        }

        // Parse the expression under the square root
        IExpressionNode expression = ParseBracedExpression();

        // If order is specified, create a power with 1/order exponent
        if (order != null)
        {
            // Create 1/order for the exponent
            var oneNode = new NumberNode(1);
            oneNode.Position = commandPosition;

            var reciprocalNode = new DivisionNode(oneNode, order);
            reciprocalNode.Position = order.Position;

            // Create the expression^(1/order) node
            var powerNode = new ExponentNode(expression, reciprocalNode);
            powerNode.Position = new SourcePosition(
                commandPosition.StartIndex,
                ((ExpressionNode)expression).Position.EndIndex,
                commandPosition.Line,
                commandPosition.Column);

            return powerNode;
        }
        else
        {
            // Create sqrt function node
            var arguments = new List<IExpressionNode> { expression };
            var sqrtNode = new StandardFunctionNode("sqrt", arguments);
            sqrtNode.Position = new SourcePosition(
                commandPosition.StartIndex,
                ((ExpressionNode)expression).Position.EndIndex,
                commandPosition.Line,
                commandPosition.Column);

            return sqrtNode;
        }
    }

    /// <summary>
    /// Parses a simple function LaTeX command, such as \sin or \cos.
    /// </summary>
    /// <param name="functionName">The name of the function (e.g., "sin").</param>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed function node.</returns>
    private IExpressionNode ParseSimpleFunctionCommand(string functionName, SourcePosition commandPosition)
    {
        // Parse the argument to the function
        IExpressionNode argument = ParseBracedExpressionOrPrimary();

        // Create function node
        var arguments = new List<IExpressionNode> { argument };
        var functionNode = new StandardFunctionNode(functionName, arguments);
        functionNode.Position = new SourcePosition(
            commandPosition.StartIndex,
            ((ExpressionNode)argument).Position.EndIndex,
            commandPosition.Line,
            commandPosition.Column);

        return functionNode;
    }

    /// <summary>
    /// Parses a multiplication LaTeX command, such as \cdot or \times.
    /// </summary>
    /// <param name="commandPosition">The position of the command in the input.</param>
    /// <returns>The parsed multiplication node.</returns>
    private IExpressionNode ParseMultiplicationCommand(SourcePosition commandPosition)
    {
        // In this implementation, we're treating \cdot and \times as regular multiplication operators
        // The left operand should have been parsed already, so we just return a marker that will be
        // handled by ParseTerm

        // Create a dummy node that will be replaced with a proper MultiplicationNode in ParseTerm
        var leftOperand = new NumberNode(1); // This will be discarded
        var rightOperand = new NumberNode(1); // This will be discarded
        var multiplicationNode = new MultiplicationNode(leftOperand, rightOperand);
        multiplicationNode.Position = commandPosition;

        return multiplicationNode;
    }

    /// <summary>
    /// Parses an expression enclosed in braces (e.g., {expression}).
    /// </summary>
    /// <returns>The parsed braced expression node.</returns>
    /// <exception cref="ParserException">Thrown if the braces are mismatched.</exception>
    private IExpressionNode ParseBracedExpression()
    {
        // Expect opening brace
        if (_currentToken.Type != TokenType.LeftBrace)
        {
            throw new ParserException("Expected '{'", _currentToken.Position);
        }

        NextToken(); // Consume {

        // Parse the expression
        IExpressionNode expression = ParseExpression();

        // Expect closing brace
        if (_currentToken.Type != TokenType.RightBrace)
        {
            throw new ParserException("Expected '}'", _currentToken.Position);
        }

        NextToken(); // Consume }

        return expression;
    }

    /// <summary>
    /// Parses either a braced expression or a primary expression.
    /// </summary>
    /// <returns>The parsed expression node.</returns>
    private IExpressionNode ParseBracedExpressionOrPrimary()
    {
        if (_currentToken.Type == TokenType.LeftBrace)
        {
            return ParseBracedExpression();
        }
        else
        {
            return ParsePrimary();
        }
    }

    /// <summary>
    /// Parses an iteration range, such as a variable with a start value.
    /// </summary>
    /// <returns>The parsed iteration range.</returns>
    private IterationRange ParseIterationRange()
    {
        // Parse the range either with braces or without
        if (_currentToken.Type == TokenType.LeftBrace)
        {
            NextToken(); // Consume {

            IterationRange range = ParseIterationRangeContent();

            // Expect closing brace
            if (_currentToken.Type != TokenType.RightBrace)
            {
                throw new ParserException("Expected '}' after iteration range", _currentToken.Position);
            }

            NextToken(); // Consume }

            return range;
        }
        else
        {
            return ParseIterationRangeContent();
        }
    }

    /// <summary>
    /// Parses the content of an iteration range, such as variable = start.
    /// </summary>
    /// <returns>The parsed iteration range content.</returns>
    private IterationRange ParseIterationRangeContent()
    {
        // Expect a variable name
        if (_currentToken.Type != TokenType.Variable)
        {
            throw new ParserException("Expected variable name in iteration range", _currentToken.Position);
        }

        string variable = _currentToken.Value;
        NextToken(); // Consume variable

        // Expect equals sign
        if (_currentToken.Type != TokenType.Equals)
        {
            throw new ParserException("Expected '=' after variable in iteration range", _currentToken.Position);
        }

        NextToken(); // Consume =

        // Parse start value
        if (_currentToken.Type != TokenType.Number && _currentToken.Type != TokenType.Variable)
        {
            throw new ParserException("Expected number or variable for start value in iteration range", _currentToken.Position);
        }

        IExpressionNode start;

        if (_currentToken.Type == TokenType.Number)
        {
            start = new NumberNode(decimal.Parse(_currentToken.Value, CultureInfo.InvariantCulture));
            ((ExpressionNode)start).Position = _currentToken.Position;
        }
        else // Variable
        {
            start = new VariableNode(_currentToken.Value);
            ((ExpressionNode)start).Position = _currentToken.Position;
        }

        NextToken(); // Consume start value

        return new IterationRange(variable, start);
    }

    /// <summary>
    /// Advances to the next token in the input.
    /// </summary>
    private void NextToken()
    {
        _currentToken = _tokenizer.GetNextToken();
    }

    /// <summary>
    /// Ensures the current token matches the expected type and advances to the next token.
    /// </summary>
    /// <param name="type">The expected token type.</param>
    /// <exception cref="ParserException">Thrown if the token type does not match.</exception>
    private void Expect(TokenType type)
    {
        if (_currentToken.Type != type)
        {
            throw new ParserException($"Expected {type}, got {_currentToken.Type}", _currentToken.Position);
        }

        NextToken();
    }

    /// <summary>
    /// Represents an iteration range with a variable and start value.
    /// </summary>
    private class IterationRange
    {
        /// <summary>
        /// Gets the variable name in the iteration range.
        /// </summary>
        public string Variable { get; }

        /// <summary>
        /// Gets the start value of the iteration range.
        /// </summary>
        public IExpressionNode Start { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="IterationRange"/> class.
        /// </summary>
        /// <param name="variable">The variable name.</param>
        /// <param name="start">The start value.</param>
        public IterationRange(string variable, IExpressionNode start)
        {
            Variable = variable;
            Start = start;
        }
    }
}