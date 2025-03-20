namespace MathSolver2
{
    /// <summary>
    /// Represents a numeric literal in the expression
    /// </summary>
    public class NumberNode : ExpressionNode
    {
        public decimal Value { get; }

        public NumberNode(decimal value)
        {
            Value = value;
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitNumber(this);
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            return Enumerable.Empty<IExpressionNode>();
        }
    }

    /// <summary>
    /// Represents a variable in the expression
    /// </summary>
    public class VariableNode : ExpressionNode
    {
        public string Name { get; }

        public VariableNode(string name)
        {
            Name = name;
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitVariable(this);
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            return Enumerable.Empty<IExpressionNode>();
        }
    }

    /// <summary>
    /// Represents an addition operation
    /// </summary>
    public class AdditionNode : OperatorNode
    {
        public AdditionNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitAddition(this);
        }
    }

    /// <summary>
    /// Represents a subtraction operation
    /// </summary>
    public class SubtractionNode : OperatorNode
    {
        public SubtractionNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitSubtraction(this);
        }
    }

    /// <summary>
    /// Represents a multiplication operation
    /// </summary>
    public class MultiplicationNode : OperatorNode
    {
        public MultiplicationNode(IExpressionNode left, IExpressionNode right)
            : base(left, right)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitMultiplication(this);
        }
    }

    /// <summary>
    /// Represents a division operation or LaTeX \frac command
    /// </summary>
    public class DivisionNode : OperatorNode
    {
        /// <summary>
        /// Gets the numerator of the division
        /// </summary>
        public IExpressionNode Numerator => Left;

        /// <summary>
        /// Gets the denominator of the division
        /// </summary>
        public IExpressionNode Denominator => Right;

        public DivisionNode(IExpressionNode numerator, IExpressionNode denominator)
            : base(numerator, denominator)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitDivision(this);
        }
    }

    /// <summary>
    /// Represents an exponentiation operation
    /// </summary>
    public class ExponentNode : OperatorNode
    {
        /// <summary>
        /// Gets the base of the exponentiation
        /// </summary>
        public IExpressionNode Base => Left;

        /// <summary>
        /// Gets the exponent
        /// </summary>
        public IExpressionNode Exponent => Right;

        public ExponentNode(IExpressionNode @base, IExpressionNode exponent)
            : base(@base, exponent)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitExponent(this);
        }
    }

    public abstract class IteratorNode: ExpressionNode
    {
        /// <summary>
        /// The variable used in the summation (e.g., i in \sum_{i=1}^{n} i)
        /// </summary>
        public string Variable { get; }

        /// <summary>
        /// The starting value of the variable (e.g., 1 in \sum_{i=1}^{n} i)
        /// </summary>
        public IExpressionNode Start { get; }

        /// <summary>
        /// The ending value of the variable (e.g., n in \sum_{i=1}^{n} i)
        /// </summary>
        public IExpressionNode End { get; }

        /// <summary>
        /// The expression to be summed (e.g., i in \sum_{i=1}^{n} i)
        /// </summary>
        public IExpressionNode Expression { get; }

        protected IteratorNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression)
        {
            Variable = variable;
            Start = start;
            End = end;
            Expression = expression;
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Start;
            yield return End;
            yield return Expression;
        }
    }

    /// <summary>
    /// Node representing a summation (e.g., \sum_{i=1}^{n} i)
    /// </summary>
    public class SummationNode : IteratorNode
    {
        public SummationNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression) :
            base(variable, start, end, expression) { }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            // Cast to our extended visitor interface if possible, otherwise throw
            if (visitor is ILatexExpressionVisitor<T> enhancedVisitor)
            {
                return enhancedVisitor.VisitSummation(this);
            }

            throw new InvalidOperationException($"Visitor of type {visitor.GetType().Name} does not support SummationNode");
        }

    }

    /// <summary>
    /// Node representing a product (e.g., \prod_{i=1}^{n} i)
    /// </summary>
    public class ProductNode : IteratorNode
    {
        public ProductNode(string variable, IExpressionNode start, IExpressionNode end, IExpressionNode expression) :
            base(variable, start, end, expression) { }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            // Cast to our extended visitor interface if possible, otherwise throw
            if (visitor is ILatexExpressionVisitor<T> enhancedVisitor)
            {
                return enhancedVisitor.VisitProduct(this);
            }

            throw new InvalidOperationException($"Visitor of type {visitor.GetType().Name} does not support ProductNode");
        }
    }

    /// <summary>
    /// Represents parentheses around an expression
    /// </summary>
    public class ParenthesisNode : ExpressionNode
    {
        public IExpressionNode Expression { get; }

        public ParenthesisNode(IExpressionNode expression)
        {
            Expression = expression;
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitParenthesis(this);
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Expression;
        }
    }

    /// <summary>
    /// Represents a standard mathematical function (sin, cos, etc.)
    /// </summary>
    public class StandardFunctionNode : FunctionNode
    {
        public StandardFunctionNode(string name, IList<IExpressionNode> arguments)
            : base(name, arguments)
        {
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitFunction(this);
        }
    }

    /// <summary>
    /// Represents a factorial operation
    /// </summary>
    public class FactorialNode : ExpressionNode
    {
        public IExpressionNode Expression { get; }

        public FactorialNode(IExpressionNode expression)
        {
            Expression = expression;
        }

        public override T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitFactorial(this);
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Expression;
        }
    }
}