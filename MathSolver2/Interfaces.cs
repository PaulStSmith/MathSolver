namespace MathSolver2
{
    /// <summary>
    /// Base interface for all nodes in the expression tree
    /// </summary>
    public interface IExpressionNode
    {
        /// <summary>
        /// Accept a visitor to process this node
        /// </summary>
        T Accept<T>(IExpressionVisitor<T> visitor);

        /// <summary>
        /// Gets the child nodes of this expression node
        /// </summary>
        IEnumerable<IExpressionNode> GetChildren();

        /// <summary>
        /// Position information for error reporting
        /// </summary>
        SourcePosition Position { get; set; }
    }

    public interface ILatexExpressionVisitor<T> : IExpressionVisitor<T>
    {
        /// <summary>
        /// Visit a summation node (e.g., \sum_{i=1}^{n} i)
        /// </summary>
        T VisitSummation(SummationNode node);

        /// <summary>
        /// Visit a product node (e.g., \prod_{i=1}^{n} i)
        /// </summary>
        T VisitProduct(ProductNode node);
    }

    /// <summary>
    /// Visitor interface for traversing and operating on the expression tree
    /// </summary>
    public interface IExpressionVisitor<T>
    {
        T VisitNumber(NumberNode node);
        T VisitVariable(VariableNode node);
        T VisitAddition(AdditionNode node);
        T VisitSubtraction(SubtractionNode node);
        T VisitMultiplication(MultiplicationNode node);
        T VisitDivision(DivisionNode node);
        T VisitExponent(ExponentNode node);
        T VisitParenthesis(ParenthesisNode node);
        T VisitFunction(FunctionNode node);
        T VisitFactorial(FactorialNode node);
    }

    /// <summary>
    /// Contains position information for nodes in the original source
    /// </summary>
    public struct SourcePosition
    {
        public int StartIndex { get; set; }
        public int EndIndex { get; set; }
        public int Line { get; set; }
        public int Column { get; set; }

        public SourcePosition(int startIndex, int endIndex, int line, int column)
        {
            StartIndex = startIndex;
            EndIndex = endIndex;
            Line = line;
            Column = column;
        }

        public override string ToString()
        {
            return $"Line {Line}, Column {Column}";
        }
    }
}