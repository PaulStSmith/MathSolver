using System.Text;

namespace MathSolver2;

/// <summary>
/// Defines the types of tokens that can be recognized by the tokenizer.
/// </summary>
public enum TokenType
{
    None,
    /// <summary>Represents a numeric value.</summary>
    Number,
    /// <summary>Represents a variable or identifier.</summary>
    Variable,
    /// <summary>Represents the '+' operator.</summary>
    Plus,
    /// <summary>Represents the '-' operator.</summary>
    Minus,
    /// <summary>Represents the '*' operator.</summary>
    Multiply,
    /// <summary>Represents the '/' operator.</summary>
    Divide,
    /// <summary>Represents the '^' operator for exponentiation.</summary>
    Power,
    /// <summary>Represents a left parenthesis '('.</summary>
    LeftParenthesis,
    /// <summary>Represents a right parenthesis ')'.</summary>
    RightParenthesis,
    /// <summary>Represents a left brace '{'.</summary>
    LeftBrace,
    /// <summary>Represents a right brace '}'.</summary>
    RightBrace,
    /// <summary>Represents a left bracket '[' used in LaTeX-style square root notation.</summary>
    LeftBracket,
    /// <summary>Represents a right bracket ']' used in LaTeX-style square root notation.</summary>
    RightBracket,
    /// <summary>Represents a comma ','.</summary>
    Comma,
    /// <summary>Represents a LaTeX command starting with '\'.</summary>
    LatexCommand,
    /// <summary>Represents the factorial operator '!'.</summary>
    Factorial,
    /// <summary>Represents an underscore '_' used in LaTeX summation.</summary>
    Underscore,
    /// <summary>Represents an equals sign '=' used in LaTeX summation.</summary>
    Equals,
    /// <summary>Represents the end of the expression.</summary>
    EndOfExpression,
    /// <summary>Represents an unrecognized or invalid token.</summary>
    Error
}

/// <summary>
/// Represents a token from the input expression.
/// </summary>
public class Token
{
    /// <summary>Gets the type of the token.</summary>
    public TokenType Type { get; }

    /// <summary>Gets the value of the token.</summary>
    public string Value { get; }

    /// <summary>Gets the position of the token in the input.</summary>
    public SourcePosition Position { get; }

    public readonly static Token None = new Token(TokenType.None, "", new SourcePosition(0, 0, 0, 0));

    /// <summary>
    /// Initializes a new instance of the <see cref="Token"/> class.
    /// </summary>
    /// <param name="type">The type of the token.</param>
    /// <param name="value">The value of the token.</param>
    /// <param name="position">The position of the token in the input.</param>
    public Token(TokenType type, string value, SourcePosition position)
    {
        Type = type;
        Value = value;
        Position = position;
        Console.WriteLine($"Token: {Type} {Value} at {Position}");
    }

    /// <summary>
    /// Returns a string representation of the token for debugging purposes.
    /// </summary>
    public override string ToString()
    {
        return $"{Type}: {Value} at {Position}";
    }
}

/// <summary>
/// Converts an input string into a sequence of tokens.
/// </summary>
public class Tokenizer
{
    private readonly string _input;
    private int _position;
    private int _line;
    private int _column;

    /// <summary>
    /// Initializes a new instance of the <see cref="Tokenizer"/> class.
    /// </summary>
    /// <param name="input">The input string to tokenize.</param>
    public Tokenizer(string input)
    {
        _input = input ?? "";
        _position = 0;
        _line = 1;
        _column = 1;
    }

    /// <summary>
    /// Returns the next token from the input without advancing the position.
    /// </summary>
    public Token Peek()
    {
        var savedPosition = _position;
        var savedLine = _line;
        var savedColumn = _column;

        var token = GetNextToken();

        _position = savedPosition;
        _line = savedLine;
        _column = savedColumn;

        return token;
    }

    /// <summary>
    /// Returns the next token from the input and advances the position.
    /// </summary>
    public Token GetNextToken()
    {
        // Skip whitespace
        SkipWhitespace();

        if (_position >= _input.Length)
        {
            return new Token(TokenType.EndOfExpression, "",
                new SourcePosition(_position, _position, _line, _column));
        }

        var current = _input[_position];
        var startPosition = _position;
        var startLine = _line;
        var startColumn = _column;

        // Check for numbers
        if (char.IsDigit(current) || current == '.')
        {
            return ScanNumber();
        }

        // Check for LaTeX commands
        if (current == '\\')
        {
            return ScanLatexCommand();
        }

        // Check for variables and function names
        if (char.IsLetter(current))
        {
            return ScanIdentifier();
        }

        // Check for operators and other symbols
        switch (current)
        {
            case '+':
                MoveNext();
                return new Token(TokenType.Plus, "+",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '-':
                MoveNext();
                return new Token(TokenType.Minus, "-",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '*':
                MoveNext();
                return new Token(TokenType.Multiply, "*",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '/':
                MoveNext();
                return new Token(TokenType.Divide, "/",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '^':
                MoveNext();
                return new Token(TokenType.Power, "^",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '(':
                MoveNext();
                return new Token(TokenType.LeftParenthesis, "(",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case ')':
                MoveNext();
                return new Token(TokenType.RightParenthesis, ")",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '[':
                MoveNext();
                return new Token(TokenType.LeftBracket, "[",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case ']':
                MoveNext();
                return new Token(TokenType.RightBracket, "]",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '{':
                MoveNext();
                return new Token(TokenType.LeftBrace, "{",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '}':
                MoveNext();
                return new Token(TokenType.RightBrace, "}",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case ',':
                MoveNext();
                return new Token(TokenType.Comma, ",",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '!':
                MoveNext();
                return new Token(TokenType.Factorial, "!",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '_':
                MoveNext();
                return new Token(TokenType.Underscore, "_",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            case '=':
                MoveNext();
                return new Token(TokenType.Equals, "=",
                    new SourcePosition(startPosition, _position, startLine, startColumn));
            default:
                MoveNext(); // Consume the character even if it's not recognized
                return new Token(TokenType.Error, current.ToString(),
                    new SourcePosition(startPosition, _position, startLine, startColumn));
        }
    }

    /// <summary>
    /// Scans a number token from the input.
    /// </summary>
    private Token ScanNumber()
    {
        var startPosition = _position;
        var startLine = _line;
        var startColumn = _column;

        var sb = new StringBuilder();
        var hasDecimalPoint = false;

        while (_position < _input.Length)
        {
            var current = _input[_position];

            if (char.IsDigit(current))
            {
                sb.Append(current);
                MoveNext();
            }
            else if (current == '.' && !hasDecimalPoint)
            {
                sb.Append(current);
                hasDecimalPoint = true;
                MoveNext();
            }
            else
            {
                break;
            }
        }

        var numberStr = sb.ToString();

        // If it's just a decimal point with no digits, treat it as an error
        if (numberStr == ".")
        {
            return new Token(TokenType.Error, numberStr,
                new SourcePosition(startPosition, _position, startLine, startColumn));
        }

        return new Token(TokenType.Number, numberStr,
            new SourcePosition(startPosition, _position - 1, startLine, startColumn));
    }

    /// <summary>
    /// Scans an identifier (variable or function name) from the input.
    /// </summary>
    private Token ScanIdentifier()
    {
        var startPosition = _position;
        var startLine = _line;
        var startColumn = _column;

        var sb = new StringBuilder();

        while (_position < _input.Length && (char.IsLetterOrDigit(_input[_position]) || _input[_position] == '_'))
        {
            sb.Append(_input[_position]);
            MoveNext();
        }

        return new Token(TokenType.Variable, sb.ToString(),
            new SourcePosition(startPosition, _position - 1, startLine, startColumn));
    }

    /// <summary>
    /// Scans a LaTeX command from the input.
    /// </summary>
    private Token ScanLatexCommand()
    {
        var startPosition = _position;
        var startLine = _line;
        var startColumn = _column;

        // Skip the backslash
        MoveNext();

        var sb = new StringBuilder();

        // LaTeX command names can only contain letters
        while (_position < _input.Length && char.IsLetter(_input[_position]))
        {
            sb.Append(_input[_position]);
            MoveNext();
        }

        if (sb.Length == 0)
        {
            // Just a backslash with no command name
            return new Token(TokenType.Error, "\\",
                new SourcePosition(startPosition, _position - 1, startLine, startColumn));
        }

        return new Token(TokenType.LatexCommand, sb.ToString(),
            new SourcePosition(startPosition, _position - 1, startLine, startColumn));
    }

    /// <summary>
    /// Skips whitespace characters in the input.
    /// </summary>
    private void SkipWhitespace()
    {
        while (_position < _input.Length && char.IsWhiteSpace(_input[_position]))
        {
            MoveNext();
        }
    }

    /// <summary>
    /// Moves to the next character in the input and updates line and column information.
    /// </summary>
    private void MoveNext()
    {
        if (_position < _input.Length)
        {
            if (_input[_position] == '\n')
            {
                _line++;
                _column = 1;
            }
            else
            {
                _column++;
            }

            _position++;
        }
    }
}