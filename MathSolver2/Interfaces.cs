namespace MathSolver2;

/// <summary>
/// Base interface for all nodes in the expression tree.
/// Represents a node that can be visited and provides access to its children and position.
/// </summary>
public interface IExpressionNode
{
    /// <summary>
    /// Accepts a visitor to process this node.
    /// </summary>
    /// <typeparam name="T">The return type of the visitor's operation.</typeparam>
    /// <param name="visitor">The visitor instance to process this node.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T Accept<T>(IExpressionVisitor<T> visitor);

    /// <summary>
    /// Gets the child nodes of this expression node.
    /// </summary>
    /// <returns>An enumerable collection of child nodes.</returns>
    IEnumerable<IExpressionNode> GetChildren();

    /// <summary>
    /// Gets or sets the position information for error reporting.
    /// </summary>
    SourcePosition Position { get; set; }
}

/// <summary>
/// Interface for a LaTeX-specific expression visitor.
/// Extends the general expression visitor with methods for LaTeX-specific nodes.
/// </summary>
/// <typeparam name="T">The return type of the visitor's operation.</typeparam>
public interface ILatexExpressionVisitor<T> : IExpressionVisitor<T>
{
    /// <summary>
    /// Visits a summation node (e.g., \sum_{i=1}^{n} i).
    /// </summary>
    /// <param name="node">The summation node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitSummation(SummationNode node);

    /// <summary>
    /// Visits a product node (e.g., \prod_{i=1}^{n} i).
    /// </summary>
    /// <param name="node">The product node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitProduct(ProductNode node);
}

/// <summary>
/// Visitor interface for traversing and operating on the expression tree.
/// Defines methods for visiting various types of expression nodes.
/// </summary>
/// <typeparam name="T">The return type of the visitor's operation.</typeparam>
public interface IExpressionVisitor<T>
{
    /// <summary>
    /// Visits a number node.
    /// </summary>
    /// <param name="node">The number node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitNumber(NumberNode node);

    /// <summary>
    /// Visits a variable node.
    /// </summary>
    /// <param name="node">The variable node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitVariable(VariableNode node);

    /// <summary>
    /// Visits an addition node.
    /// </summary>
    /// <param name="node">The addition node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitAddition(AdditionNode node);

    /// <summary>
    /// Visits a subtraction node.
    /// </summary>
    /// <param name="node">The subtraction node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitSubtraction(SubtractionNode node);

    /// <summary>
    /// Visits a multiplication node.
    /// </summary>
    /// <param name="node">The multiplication node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitMultiplication(MultiplicationNode node);

    /// <summary>
    /// Visits a division node.
    /// </summary>
    /// <param name="node">The division node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitDivision(DivisionNode node);

    /// <summary>
    /// Visits an exponentiation node.
    /// </summary>
    /// <param name="node">The exponentiation node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitExponent(ExponentNode node);

    /// <summary>
    /// Visits a parenthesis node.
    /// </summary>
    /// <param name="node">The parenthesis node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitParenthesis(ParenthesisNode node);

    /// <summary>
    /// Visits a function node.
    /// </summary>
    /// <param name="node">The function node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitFunction(FunctionNode node);

    /// <summary>
    /// Visits a factorial node.
    /// </summary>
    /// <param name="node">The factorial node to visit.</param>
    /// <returns>The result of the visitor's operation.</returns>
    T VisitFactorial(FactorialNode node);
}

/// <summary>
/// Contains position information for nodes in the original source.
/// Used for error reporting and debugging.
/// </summary>
public struct SourcePosition
{
    /// <summary>
    /// Gets or sets the start index of the position.
    /// </summary>
    public int StartIndex { get; set; }

    /// <summary>
    /// Gets or sets the end index of the position.
    /// </summary>
    public int EndIndex { get; set; }

    /// <summary>
    /// Gets or sets the line number of the position.
    /// </summary>
    public int Line { get; set; }

    /// <summary>
    /// Gets or sets the column number of the position.
    /// </summary>
    public int Column { get; set; }

    /// <summary>
    /// Initializes a new instance of the <see cref="SourcePosition"/> struct.
    /// </summary>
    /// <param name="startIndex">The start index of the position.</param>
    /// <param name="endIndex">The end index of the position.</param>
    /// <param name="line">The line number of the position.</param>
    /// <param name="column">The column number of the position.</param>
    public SourcePosition(int startIndex, int endIndex, int line, int column)
    {
        StartIndex = startIndex;
        EndIndex = endIndex;
        Line = line;
        Column = column;
    }

    /// <summary>
    /// Returns a string representation of the position.
    /// </summary>
    /// <returns>A string in the format "Line {Line}, Column {Column}".</returns>
    public override string ToString()
    {
        return $"Line {Line}, Column {Column}";
    }
}