using System.Text;

namespace MathSolver2
{
    /// <summary>
    /// Defines the types of tokens that can be recognized by the tokenizer
    /// </summary>
    public enum TokenType
    {
        Number,
        Variable,
        Plus,
        Minus,
        Multiply,
        Divide,
        Power,
        LeftParenthesis,
        RightParenthesis,
        LeftBrace,
        RightBrace,
        LeftBracket,    // [ - for \sqrt[n]{x}
        RightBracket,   // ] - for \sqrt[n]{x}
        Comma,
        LatexCommand,
        Factorial,
        Underscore,     // _ - for \sum_{i=1}
        Equals,         // = - for i=1 in \sum_{i=1}
        EndOfExpression,
        Error
    }

    /// <summary>
    /// Represents a token from the input expression
    /// </summary>
    public class Token
    {
        public TokenType Type { get; }
        public string Value { get; }
        public SourcePosition Position { get; }

        public Token(TokenType type, string value, SourcePosition position)
        {
            Type = type;
            Value = value;
            Position = position;
        }

        public override string ToString()
        {
            return $"{Type}: {Value} at {Position}";
        }
    }

    /// <summary>
    /// Converts an input string into a sequence of tokens
    /// </summary>
    public class Tokenizer
    {
        private readonly string _input;
        private int _position;
        private int _line;
        private int _column;

        public Tokenizer(string input)
        {
            _input = input ?? "";
            _position = 0;
            _line = 1;
            _column = 1;
        }

        /// <summary>
        /// Returns the next token from the input without advancing the position
        /// </summary>
        public Token Peek()
        {
            int savedPosition = _position;
            int savedLine = _line;
            int savedColumn = _column;

            Token token = GetNextToken();

            _position = savedPosition;
            _line = savedLine;
            _column = savedColumn;

            return token;
        }

        /// <summary>
        /// Returns the next token from the input and advances the position
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

            char current = _input[_position];
            int startPosition = _position;
            int startLine = _line;
            int startColumn = _column;

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
                default:
                    MoveNext(); // Consume the character even if it's not recognized
                    return new Token(TokenType.Error, current.ToString(),
                        new SourcePosition(startPosition, _position, startLine, startColumn));
            }
        }

        /// <summary>
        /// Scans a number token
        /// </summary>
        private Token ScanNumber()
        {
            int startPosition = _position;
            int startLine = _line;
            int startColumn = _column;

            StringBuilder sb = new StringBuilder();
            bool hasDecimalPoint = false;

            while (_position < _input.Length)
            {
                char current = _input[_position];

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

            string numberStr = sb.ToString();

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
        /// Scans an identifier (variable or function name)
        /// </summary>
        private Token ScanIdentifier()
        {
            int startPosition = _position;
            int startLine = _line;
            int startColumn = _column;

            StringBuilder sb = new StringBuilder();

            while (_position < _input.Length && (char.IsLetterOrDigit(_input[_position]) || _input[_position] == '_'))
            {
                sb.Append(_input[_position]);
                MoveNext();
            }

            return new Token(TokenType.Variable, sb.ToString(),
                new SourcePosition(startPosition, _position - 1, startLine, startColumn));
        }

        /// <summary>
        /// Scans a LaTeX command
        /// </summary>
        private Token ScanLatexCommand()
        {
            int startPosition = _position;
            int startLine = _line;
            int startColumn = _column;

            // Skip the backslash
            MoveNext();

            StringBuilder sb = new StringBuilder();

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
        /// Skips whitespace characters
        /// </summary>
        private void SkipWhitespace()
        {
            while (_position < _input.Length && char.IsWhiteSpace(_input[_position]))
            {
                MoveNext();
            }
        }

        /// <summary>
        /// Moves to the next character in the input and updates line and column information
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
}