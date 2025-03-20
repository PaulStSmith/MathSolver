namespace MathSolver2
{
    /// <summary>
    /// Abstract base class for all expression nodes
    /// </summary>
    public abstract class ExpressionNode : IExpressionNode
    {
        public SourcePosition Position { get; set; }

        public abstract T Accept<T>(IExpressionVisitor<T> visitor);

        public abstract IEnumerable<IExpressionNode> GetChildren();

        protected ExpressionNode()
        {
            Position = new SourcePosition(0, 0, 0, 0);
        }
    }

    /// <summary>
    /// Base class for all operator nodes (binary operations)
    /// </summary>
    public abstract class OperatorNode : ExpressionNode
    {
        public IExpressionNode Left { get; }
        public IExpressionNode Right { get; }

        protected OperatorNode(IExpressionNode left, IExpressionNode right)
        {
            Left = left;
            Right = right;
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            yield return Left;
            yield return Right;
        }
    }

    /// <summary>
    /// Base class for all function nodes (operations with one or more arguments)
    /// </summary>
    public abstract class FunctionNode : ExpressionNode
    {
        public string Name { get; }
        public IList<IExpressionNode> Arguments { get; }

        protected FunctionNode(string name, IList<IExpressionNode> arguments)
        {
            Name = name;
            Arguments = arguments;
        }

        public override IEnumerable<IExpressionNode> GetChildren()
        {
            return Arguments;
        }
    }
}