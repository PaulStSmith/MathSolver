using System.Globalization;
using System.Text;

namespace MathSolver2;

/// <summary>
/// Output format for the expression.
/// </summary>
public enum OutputFormat
{
    /// <summary>
    /// Standard mathematical notation.
    /// </summary>
    Standard,

    /// <summary>
    /// LaTeX mathematical notation.
    /// </summary>
    LaTeX
}

/// <summary>
/// Formats an expression tree as a string.
/// </summary>
public class FormattingVisitor : IExpressionVisitor<string>
{
    private readonly OutputFormat _format;

    /// <summary>
    /// Initializes a new instance of the <see cref="FormattingVisitor"/> class.
    /// </summary>
    /// <param name="format">The output format for the expression.</param>
    public FormattingVisitor(OutputFormat format = OutputFormat.Standard)
    {
        _format = format;
    }

    /// <summary>
    /// Formats the given expression node as a string.
    /// </summary>
    /// <param name="node">The expression node to format.</param>
    /// <returns>A string representation of the expression.</returns>
    public string Format(IExpressionNode node)
    {
        return node.Accept(this);
    }

    /// <summary>
    /// Visits a number node and formats it as a string.
    /// </summary>
    /// <param name="node">The number node to visit.</param>
    /// <returns>The formatted number as a string.</returns>
    public string VisitNumber(NumberNode node)
    {
        // Format the number using the invariant culture to ensure consistent output
        var value = node.Value;
        return value.ToString(CultureInfo.InvariantCulture);
    }

    /// <summary>
    /// Visits a variable node and formats it as a string.
    /// </summary>
    /// <param name="node">The variable node to visit.</param>
    /// <returns>The variable name as a string.</returns>
    public string VisitVariable(VariableNode node)
    {
        return node.Name;
    }

    /// <summary>
    /// Visits an addition node and formats it as a string.
    /// </summary>
    /// <param name="node">The addition node to visit.</param>
    /// <returns>The formatted addition expression as a string.</returns>
    public string VisitAddition(AdditionNode node)
    {
        string left = node.Left.Accept(this);
        string right = node.Right.Accept(this);

        // Check if right operand is a negative number and adjust formatting
        if (node.Right is NumberNode rightNumber && rightNumber.Value < 0)
        {
            // If right operand is negative, change to subtraction of positive
            return $"{left} - {(-rightNumber.Value).ToString(CultureInfo.InvariantCulture)}";
        }

        return $"{left} + {right}";
    }

    /// <summary>
    /// Visits a subtraction node and formats it as a string.
    /// </summary>
    /// <param name="node">The subtraction node to visit.</param>
    /// <returns>The formatted subtraction expression as a string.</returns>
    public string VisitSubtraction(SubtractionNode node)
    {
        string left = node.Left.Accept(this);
        string right = node.Right.Accept(this);

        return $"{left} - {right}";
    }

    /// <summary>
    /// Visits a multiplication node and formats it as a string.
    /// </summary>
    /// <param name="node">The multiplication node to visit.</param>
    /// <returns>The formatted multiplication expression as a string.</returns>
    public string VisitMultiplication(MultiplicationNode node)
    {
        string left = FormatWithParenthesesIfNeeded(node.Left, IsLowerPrecedence(node.Left, node));
        string right = FormatWithParenthesesIfNeeded(node.Right, IsLowerPrecedence(node.Right, node));

        if (_format == OutputFormat.LaTeX)
        {
            return $"{left} \\cdot {right}";
        }

        return $"{left} * {right}";
    }

    /// <summary>
    /// Visits a division node and formats it as a string.
    /// </summary>
    /// <param name="node">The division node to visit.</param>
    /// <returns>The formatted division expression as a string.</returns>
    public string VisitDivision(DivisionNode node)
    {
        if (_format == OutputFormat.LaTeX)
        {
            string numerator = node.Numerator.Accept(this);
            string denominator = node.Denominator.Accept(this);

            return $"\\frac{{{numerator}}}{{{denominator}}}";
        }
        else
        {
            string numerator = FormatWithParenthesesIfNeeded(node.Numerator, IsLowerPrecedence(node.Numerator, node));
            string denominator = FormatWithParenthesesIfNeeded(node.Denominator, IsLowerPrecedence(node.Denominator, node));

            return $"{numerator} / {denominator}";
        }
    }

    /// <summary>
    /// Visits an exponent node and formats it as a string.
    /// </summary>
    /// <param name="node">The exponent node to visit.</param>
    /// <returns>The formatted exponent expression as a string.</returns>
    public string VisitExponent(ExponentNode node)
    {
        string baseStr = FormatWithParenthesesIfNeeded(node.Base,
            IsLowerPrecedence(node.Base, node) || node.Base is ExponentNode);
        string exponentStr = FormatWithParenthesesIfNeeded(node.Exponent,
            IsLowerPrecedence(node.Exponent, node));

        if (_format == OutputFormat.LaTeX)
        {
            return $"{baseStr}^{{{exponentStr}}}";
        }

        return $"{baseStr}^{exponentStr}";
    }

    /// <summary>
    /// Visits a parenthesis node and formats it as a string.
    /// </summary>
    /// <param name="node">The parenthesis node to visit.</param>
    /// <returns>The formatted parenthesis expression as a string.</returns>
    public string VisitParenthesis(ParenthesisNode node)
    {
        string inner = node.Expression.Accept(this);

        return $"({inner})";
    }

    /// <summary>
    /// Visits a function node and formats it as a string.
    /// </summary>
    /// <param name="node">The function node to visit.</param>
    /// <returns>The formatted function expression as a string.</returns>
    public string VisitFunction(FunctionNode node)
    {
        StringBuilder sb = new StringBuilder();

        sb.Append(node.Name);
        sb.Append('(');

        for (int i = 0; i < node.Arguments.Count; i++)
        {
            if (i > 0)
            {
                sb.Append(", ");
            }

            sb.Append(node.Arguments[i].Accept(this));
        }

        sb.Append(')');

        return sb.ToString();
    }

    /// <summary>
    /// Visits a factorial node and formats it as a string.
    /// </summary>
    /// <param name="node">The factorial node to visit.</param>
    /// <returns>The formatted factorial expression as a string.</returns>
    public string VisitFactorial(FactorialNode node)
    {
        string expr = FormatWithParenthesesIfNeeded(node.Expression,
            !(node.Expression is NumberNode || node.Expression is VariableNode || node.Expression is ParenthesisNode));

        return $"{expr}!";
    }

    /// <summary>
    /// Formats a node with parentheses if needed.
    /// </summary>
    /// <param name="node">The node to format.</param>
    /// <param name="needsParentheses">Whether parentheses are needed.</param>
    /// <returns>The formatted node as a string.</returns>
    private string FormatWithParenthesesIfNeeded(IExpressionNode node, bool needsParentheses)
    {
        string formatted = node.Accept(this);

        if (needsParentheses)
        {
            return $"({formatted})";
        }

        return formatted;
    }

    /// <summary>
    /// Determines if a node has lower precedence than its parent.
    /// </summary>
    /// <param name="node">The child node.</param>
    /// <param name="parent">The parent node.</param>
    /// <returns><c>true</c> if the child node has lower precedence; otherwise, <c>false</c>.</returns>
    private bool IsLowerPrecedence(IExpressionNode node, IExpressionNode parent)
    {
        int nodePrecedence = GetPrecedence(node);
        int parentPrecedence = GetPrecedence(parent);

        return nodePrecedence < parentPrecedence;
    }

    /// <summary>
    /// Gets the precedence level of a node.
    /// </summary>
    /// <param name="node">The node to evaluate.</param>
    /// <returns>The precedence level of the node.</returns>
    private int GetPrecedence(IExpressionNode node)
    {
        if (node is AdditionNode || node is SubtractionNode)
        {
            return 1; // Lowest precedence
        }
        else if (node is MultiplicationNode || node is DivisionNode)
        {
            return 2;
        }
        else if (node is ExponentNode)
        {
            return 3;
        }
        else if (node is NumberNode || node is VariableNode || node is ParenthesisNode ||
                 node is FunctionNode || node is FactorialNode)
        {
            return 4; // Highest precedence
        }

        return 0;
    }
}