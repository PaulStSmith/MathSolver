using System.Globalization;
using System.Text;

namespace MathSolver2
{
    /// <summary>
    /// Output format for the expression
    /// </summary>
    public enum OutputFormat
    {
        Standard,
        LaTeX
    }

    /// <summary>
    /// Formats an expression tree as a string
    /// </summary>
    public class FormattingVisitor : IExpressionVisitor<string>
    {
        private readonly OutputFormat _format;

        public FormattingVisitor(OutputFormat format = OutputFormat.Standard)
        {
            _format = format;
        }

        /// <summary>
        /// Formats the expression as a string
        /// </summary>
        public string Format(IExpressionNode node)
        {
            return node.Accept(this);
        }

        public string VisitNumber(NumberNode node)
        {
            // Format the number using the invariant culture to ensure consistent output
            return node.Value.ToString(CultureInfo.InvariantCulture);
        }

        public string VisitVariable(VariableNode node)
        {
            return node.Name;
        }

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

        public string VisitSubtraction(SubtractionNode node)
        {
            string left = node.Left.Accept(this);
            string right = node.Right.Accept(this);

            return $"{left} - {right}";
        }

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

        public string VisitParenthesis(ParenthesisNode node)
        {
            string inner = node.Expression.Accept(this);

            return $"({inner})";
        }

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

        public string VisitFactorial(FactorialNode node)
        {
            string expr = FormatWithParenthesesIfNeeded(node.Expression,
                !(node.Expression is NumberNode || node.Expression is VariableNode || node.Expression is ParenthesisNode));

            return $"{expr}!";
        }

        /// <summary>
        /// Formats a node with parentheses if needed
        /// </summary>
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
        /// Determines if a node has lower precedence than its parent
        /// </summary>
        private bool IsLowerPrecedence(IExpressionNode node, IExpressionNode parent)
        {
            int nodePrecedence = GetPrecedence(node);
            int parentPrecedence = GetPrecedence(parent);

            return nodePrecedence < parentPrecedence;
        }

        /// <summary>
        /// Gets the precedence level of a node
        /// </summary>
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
}