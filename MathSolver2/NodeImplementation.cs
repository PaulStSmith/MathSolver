namespace MathSolver2
{
    /// <summary>
    /// Represents a numeric literal in the expression.
    /// </summary>
    public class NumberNode : ExpressionNode
    {
        /// <summary>
        /// Gets the numeric value of the node.
        /// </summary>
        public decimal Value { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="NumberNode"/> class.
        /// </summary>
        /// <param name="value">The numeric value.</param>
        public NumberNode(decimal value)
        {
            Value = value;
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitNumber(this);
        }

        /// <inheritdoc />
        public override IEnumerable<IExpressionNode> GetChildren()
        {
            return Enumerable.Empty<IExpressionNode>();
        }
    }

    /// <summary>
    /// Represents a variable in the expression.
    /// </summary>
    public class VariableNode : ExpressionNode
    {
        /// <summary>
        /// Gets the name of the variable.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="VariableNode"/> class.
        /// </summary>
        /// <param name="name">The name of the variable.</param>
        public VariableNode(string name)
        {
            Name = name;
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitVariable(this);
        }

        /// <inheritdoc />
        public override IEnumerable<IExpressionNode> GetChildren()
        {
            return Enumerable.Empty<IExpressionNode>();
        }
    }

    /// <summary>
    /// Represents an addition operation.
    /// </summary>
    public class AdditionNode : OperatorNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AdditionNode"/> class.
        /// </summary>
        /// <param name="left">The left operand.</param>
        /// <param name="right">The right operand.</param>
        public AdditionNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitAddition(this);
        }
    }

    /// <summary>
    /// Represents a subtraction operation.
    /// </summary>
    public class SubtractionNode : OperatorNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SubtractionNode"/> class.
        /// </summary>
        /// <param name="left">The left operand.</param>
        /// <param name="right">The right operand.</param>
        public SubtractionNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitSubtraction(this);
        }
    }

    /// <summary>
    /// Represents a multiplication operation.
    /// </summary>
    public class MultiplicationNode : OperatorNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="MultiplicationNode"/> class.
        /// </summary>
        /// <param name="left">The left operand.</param>
        /// <param name="right">The right operand.</param>
        public MultiplicationNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitMultiplication(this);
        }
    }

    /// <summary>
    /// Represents a division operation or LaTeX \frac command.
    /// </summary>
    public class DivisionNode : OperatorNode
    {
        /// <summary>
        /// Gets the numerator of the division.
        /// </summary>
        public IExpressionNode Numerator => Left;

        /// <summary>
        /// Gets the denominator of the division.
        /// </summary>
        public IExpressionNode Denominator => Right;

        /// <summary>
        /// Initializes a new instance of the <see cref="DivisionNode"/> class.
        /// </summary>
        /// <param name="numerator">The numerator.</param>
        /// <param name="denominator">The denominator.</param>
        public DivisionNode(IExpressionNode numerator, IExpressionNode denominator)
            : base(numerator, denominator)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitDivision(this);
        }
    }

    /// <summary>
    /// Represents an exponentiation operation.
    /// </summary>
    public class ExponentNode : OperatorNode
    {
        /// <summary>
        /// Gets the base of the exponentiation.
        /// </summary>
        public IExpressionNode Base => Left;

        /// <summary>
        /// Gets the exponent.
        /// </summary>
        public IExpressionNode Exponent => Right;

        /// <summary>
        /// Initializes a new instance of the <see cref="ExponentNode"/> class.
        /// </summary>
        /// <param name="base">The base of the exponentiation.</param>
        /// <param name="exponent">The exponent.</param>
        public ExponentNode(IExpressionNode @base, IExpressionNode exponent)
            : base(@base, exponent)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitExponent(this);
        }
    }

    /// <summary>
    /// Represents an iterator node, such as summation or product.
    /// </summary>
    public abstract class IteratorNode : ExpressionNode
    {
        /// <summary>
        /// Gets the variable used in the iteration (e.g., i in ∑ or ∏).
        /// </summary>
        public string Variable { get; }

        /// <summary>
        /// Gets the starting value of the iteration (e.g., 1 in ∑ or ∏).
        /// </summary>
        public IExpressionNode Start { get; }

        /// <summary>
        /// Gets the ending value of the iteration (e.g., n in ∑ or ∏).
        /// </summary>
        public IExpressionNode End { get; }

        /// <summary>
        /// Gets the expression to be iterated (e.g., i in ∑ or ∏).
        /// </summary>
        public IExpressionNode Expression { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="IteratorNode"/> class.
        /// </summary>
        /// <param name="variable">The iteration variable.</param>
        /// <param name="start">The starting value of the iteration.</param>
        /// <param name="end">The ending value of the iteration.</param>
        /// <param name="expression">The expression to iterate over.</param>
        protected IteratorNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression)
        {
            Variable = variable;
            Start = start;
            End = end;
            Expression = expression;
        }

        /// <inheritdoc />
        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Start;
            yield return End;
            yield return Expression;
        }
    }

    /// <summary>
    /// Node representing a summation (e.g., \sum_{i=1}^{n} i).
    /// </summary>
    public class SummationNode : IteratorNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SummationNode"/> class.
        /// </summary>
        /// <param name="variable">The iteration variable.</param>
        /// <param name="start">The starting value of the summation.</param>
        /// <param name="end">The ending value of the summation.</param>
        /// <param name="expression">The expression to sum over.</param>
        public SummationNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression) :
            base(variable, start, end, expression) { }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            if (visitor is ILatexExpressionVisitor<T> enhancedVisitor)
            {
                return enhancedVisitor.VisitSummation(this);
            }

            throw new InvalidOperationException($"Visitor of type {visitor.GetType().Name} does not support SummationNode");
        }
    }

    /// <summary>
    /// Node representing a product (e.g., \prod_{i=1}^{n} i).
    /// </summary>
    public class ProductNode : IteratorNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ProductNode"/> class.
        /// </summary>
        /// <param name="variable">The iteration variable.</param>
        /// <param name="start">The starting value of the product.</param>
        /// <param name="end">The ending value of the product.</param>
        /// <param name="expression">The expression to multiply over.</param>
        public ProductNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression) :
            base(variable, start, end, expression) { }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            if (visitor is ILatexExpressionVisitor<T> enhancedVisitor)
            {
                return enhancedVisitor.VisitProduct(this);
            }

            throw new InvalidOperationException($"Visitor of type {visitor.GetType().Name} does not support ProductNode");
        }
    }

    /// <summary>
    /// Represents parentheses around an expression.
    /// </summary>
    public class ParenthesisNode : ExpressionNode
    {
        /// <summary>
        /// Gets the expression inside the parentheses.
        /// </summary>
        public IExpressionNode Expression { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="ParenthesisNode"/> class.
        /// </summary>
        /// <param name="expression">The expression inside the parentheses.</param>
        public ParenthesisNode(IExpressionNode expression)
        {
            Expression = expression;
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitParenthesis(this);
        }

        /// <inheritdoc />
        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Expression;
        }
    }

    /// <summary>
    /// Represents a standard mathematical function (sin, cos, etc.).
    /// </summary>
    public class StandardFunctionNode : FunctionNode
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StandardFunctionNode"/> class.
        /// </summary>
        /// <param name="name">The name of the function.</param>
        /// <param name="arguments">The arguments of the function.</param>
        public StandardFunctionNode(string name, IList<IExpressionNode> arguments)
            : base(name, arguments)
        {
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitFunction(this);
        }
    }

    /// <summary>
    /// Represents a factorial operation.
    /// </summary>
    public class FactorialNode : ExpressionNode
    {
        /// <summary>
        /// Gets the expression to which the factorial is applied.
        /// </summary>
        public IExpressionNode Expression { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="FactorialNode"/> class.
        /// </summary>
        /// <param name="expression">The expression to which the factorial is applied.</param>
        public FactorialNode(IExpressionNode expression)
        {
            Expression = expression;
        }

        /// <inheritdoc />
        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitFactorial(this);
        }

        /// <inheritdoc />
        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Expression;
        }
    }
}