namespace MathSolver2;

/// <summary>
/// Abstract base class for all expression nodes.
/// </summary>
public abstract class ExpressionNode : IExpressionNode
{
    /// <summary>
    /// Gets or sets the source position of the expression node.
    /// </summary>
    public SourcePosition Position { get; set; }

    /// <summary>
    /// Accepts a visitor to process the expression node.
    /// </summary>
    /// <typeparam name="T">The return type of the visitor.</typeparam>
    /// <param name="visitor">The visitor instance.</param>
    /// <returns>The result of the visitor's processing.</returns>
    public abstract T Accept<T>(IExpressionVisitor<T> visitor);

    /// <summary>
    /// Gets the child nodes of the expression node.
    /// </summary>
    /// <returns>An enumerable of child nodes.</returns>
    public abstract IEnumerable<IExpressionNode> GetChildren();

    /// <summary>
    /// Initializes a new instance of the <see cref="ExpressionNode"/> class.
    /// </summary>
    protected ExpressionNode()
    {
        Position = new SourcePosition(0, 0, 0, 0);
    }
}

/// <summary>
/// Base class for all operator nodes (binary operations).
/// </summary>
public abstract class OperatorNode : ExpressionNode
{
    /// <summary>
    /// Gets the left operand of the operator.
    /// </summary>
    public IExpressionNode Left { get; }

    /// <summary>
    /// Gets the right operand of the operator.
    /// </summary>
    public IExpressionNode Right { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="OperatorNode"/> class.
    /// </summary>
    /// <param name="left">The left operand.</param>
    /// <param name="right">The right operand.</param>
    protected OperatorNode(IExpressionNode left, IExpressionNode right)
    {
        Left = left;
        Right = right;
    }

    /// <summary>
    /// Gets the child nodes of the operator node.
    /// </summary>
    /// <returns>An enumerable of child nodes.</returns>
    public override IEnumerable<IExpressionNode> GetChildren()
    {
        yield return Left;
        yield return Right;
    }
}

/// <summary>
/// Base class for all function nodes (operations with one or more arguments).
/// </summary>
public abstract class FunctionNode : ExpressionNode
{
    /// <summary>
    /// Gets the name of the function.
    /// </summary>
    public string Name { get; }

    /// <summary>
    /// Gets the arguments of the function.
    /// </summary>
    public IList<IExpressionNode> Arguments { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="FunctionNode"/> class.
    /// </summary>
    /// <param name="name">The name of the function.</param>
    /// <param name="arguments">The arguments of the function.</param>
    protected FunctionNode(string name, IList<IExpressionNode> arguments)
    {
        Name = name;
        Arguments = arguments;
    }

    /// <summary>
    /// Gets the child nodes of the function node.
    /// </summary>
    /// <returns>An enumerable of child nodes.</returns>
    public override IEnumerable<IExpressionNode> GetChildren()
    {
        return Arguments;
    }
}