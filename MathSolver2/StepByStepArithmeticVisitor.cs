namespace MathSolver2;

/// <summary>
/// Evaluates an expression tree step by step with arithmetic formatting and records each calculation.
/// </summary>
public class StepByStepArithmeticVisitor : BaseArithmeticVisitor<StepByStepResult>
{
    private readonly FormattingVisitor _formatter;

    /// <summary>
    /// Creates a new step-by-step arithmetic visitor with the specified settings.
    /// </summary>
    /// <param name="variables">A dictionary of variable names and their values.</param>
    /// <param name="arithmeticType">The type of arithmetic to use (e.g., standard or scientific).</param>
    /// <param name="precision">The number of decimal places to use in calculations.</param>
    /// <param name="useSignificantDigits">Whether to use significant digits for formatting.</param>
    public StepByStepArithmeticVisitor(
        Dictionary<string, decimal> variables,
        ArithmeticType arithmeticType,
        int precision,
        bool useSignificantDigits) : base(variables, arithmeticType, precision, useSignificantDigits)
    {
        _formatter = new FormattingVisitor();
    }

    #region Visitor Methods

    /// <summary>
    /// Visits a number node and returns its value as a result.
    /// </summary>
    /// <param name="node">The number node to visit.</param>
    /// <returns>A result containing the value of the number and no steps.</returns>
    public override StepByStepResult VisitNumber(NumberNode node)
    {
        return new StepByStepResult(node.Value, new List<CalculationStep>());
    }

    /// <summary>
    /// Visits a variable node, substitutes its value, and returns the result.
    /// </summary>
    /// <param name="node">The variable node to visit.</param>
    /// <returns>A result containing the value of the variable and the substitution step.</returns>
    /// <exception cref="EvaluationException">Thrown if the variable is not defined.</exception>
    public override StepByStepResult VisitVariable(VariableNode node)
    {
        // First check if it's a mathematical constant
        if (MathConstants.TryGetValue(node.Name, out decimal constValue))
        {
            decimal formattedConstValue = FormatNumber(constValue);

            var steps = new List<CalculationStep>
            {
                new CalculationStep(
                    node.Name,
                    $"Substitute mathematical constant {node.Name}",
                    formattedConstValue.ToString())
            };

            return new StepByStepResult(formattedConstValue, steps);
        }
        if (Variables.TryGetValue(node.Name, out decimal value))
        {
            var steps = new List<CalculationStep>
            {
                new CalculationStep(
                    node.Name,
                    $"Substitute variable {node.Name}",
                    value.ToString())
            };

            return new StepByStepResult(value, steps);
        }

        throw new EvaluationException($"Variable '{node.Name}' is not defined", node.Position);
    }

    /// <summary>
    /// Visits an addition node, evaluates its operands, and returns the result.
    /// </summary>
    /// <param name="node">The addition node to visit.</param>
    /// <returns>A result containing the sum and the calculation steps.</returns>
    public override StepByStepResult VisitAddition(AdditionNode node)
    {
        var leftResult = node.Left.Accept(this);
        var rightResult = node.Right.Accept(this);

        var allSteps = new List<CalculationStep>(leftResult.Steps);
        allSteps.AddRange(rightResult.Steps);

        decimal rawResult = leftResult.Value + rightResult.Value;
        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"Add {leftResult.Value} and {rightResult.Value}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits a subtraction node, evaluates its operands, and returns the result.
    /// </summary>
    /// <param name="node">The subtraction node to visit.</param>
    /// <returns>A result containing the difference and the calculation steps.</returns>
    public override StepByStepResult VisitSubtraction(SubtractionNode node)
    {
        var leftResult = node.Left.Accept(this);
        var rightResult = node.Right.Accept(this);

        var allSteps = new List<CalculationStep>(leftResult.Steps);
        allSteps.AddRange(rightResult.Steps);

        decimal rawResult = leftResult.Value - rightResult.Value;
        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"Subtract {rightResult.Value} from {leftResult.Value}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits a multiplication node, evaluates its operands, and returns the result.
    /// </summary>
    /// <param name="node">The multiplication node to visit.</param>
    /// <returns>A result containing the product and the calculation steps.</returns>
    public override StepByStepResult VisitMultiplication(MultiplicationNode node)
    {
        var leftResult = node.Left.Accept(this);
        var rightResult = node.Right.Accept(this);

        var allSteps = new List<CalculationStep>(leftResult.Steps);
        allSteps.AddRange(rightResult.Steps);

        decimal rawResult = leftResult.Value * rightResult.Value;
        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"Multiply {leftResult.Value} by {rightResult.Value}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits a division node, evaluates its operands, and returns the result.
    /// </summary>
    /// <param name="node">The division node to visit.</param>
    /// <returns>A result containing the quotient and the calculation steps.</returns>
    /// <exception cref="EvaluationException">Thrown if division by zero occurs.</exception>
    public override StepByStepResult VisitDivision(DivisionNode node)
    {
        var numeratorResult = node.Numerator.Accept(this);
        var denominatorResult = node.Denominator.Accept(this);

        var allSteps = new List<CalculationStep>(numeratorResult.Steps);
        allSteps.AddRange(denominatorResult.Steps);

        if (denominatorResult.Value == 0)
        {
            throw new EvaluationException("Division by zero", node.Position);
        }

        decimal rawResult = numeratorResult.Value / denominatorResult.Value;
        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"Divide {numeratorResult.Value} by {denominatorResult.Value}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits an exponent node, evaluates its base and exponent, and returns the result.
    /// </summary>
    /// <param name="node">The exponent node to visit.</param>
    /// <returns>A result containing the power and the calculation steps.</returns>
    public override StepByStepResult VisitExponent(ExponentNode node)
    {
        var baseResult = node.Base.Accept(this);
        var exponentResult = node.Exponent.Accept(this);

        var allSteps = new List<CalculationStep>(baseResult.Steps);
        allSteps.AddRange(exponentResult.Steps);

        decimal rawResult;
        string operation;

        // Handle special cases
        if (exponentResult.Value == 0)
        {
            rawResult = 1;
            operation = $"Any number raised to power 0 is 1";
        }
        else if (baseResult.Value == 0)
        {
            rawResult = 0;
            operation = $"0 raised to any non-zero power is 0";
        }
        else if (exponentResult.Value == 1)
        {
            rawResult = baseResult.Value;
            operation = $"Any number raised to power 1 is the number itself";
        }
        else
        {
            // Check if exponent is an integer
            if (Math.Abs(exponentResult.Value - Math.Round(exponentResult.Value)) < MathConstants.Epsilon)
            {
                int intExponent = (int)Math.Round(exponentResult.Value);
                rawResult = (decimal)Math.Pow((double)baseResult.Value, intExponent);
            }
            else
            {
                // For non-integer exponents, use Math.Pow
                rawResult = (decimal)Math.Pow((double)baseResult.Value, (double)exponentResult.Value);
            }

            operation = $"Raise {baseResult.Value} to the power of {exponentResult.Value}";
        }

        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"{operation}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits a parenthesis node, evaluates its inner expression, and returns the result.
    /// </summary>
    /// <param name="node">The parenthesis node to visit.</param>
    /// <returns>A result containing the value of the inner expression and the calculation steps.</returns>
    public override StepByStepResult VisitParenthesis(ParenthesisNode node)
    {
        var innerResult = node.Expression.Accept(this);

        // Add a step showing the parentheses are being evaluated
        string expression = _formatter.Format(node);
        var allSteps = new List<CalculationStep>(innerResult.Steps);

        if (innerResult.Steps.Count > 0)
        {
            allSteps.Add(new CalculationStep(
                expression,
                "Evaluate parentheses",
                innerResult.Value.ToString()));
        }

        return new StepByStepResult(innerResult.Value, allSteps);
    }

    /// <summary>
    /// Visits a function node, evaluates its arguments, and returns the result.
    /// </summary>
    /// <param name="node">The function node to visit.</param>
    /// <returns>A result containing the function's value and the calculation steps.</returns>
    public override StepByStepResult VisitFunction(FunctionNode node)
    {
        // Evaluate arguments and collect steps
        var argResults = node.Arguments.Select(arg => arg.Accept(this)).ToList();
        var allSteps = new List<CalculationStep>();
        foreach (var argResult in argResults)
        {
            allSteps.AddRange(argResult.Steps);
        }

        // Get argument values
        decimal[] argValues = argResults.Select(r => r.Value).ToArray();

        // Evaluate the function - getting both result and description
        var (result, description) = EvaluateFunction(node.Name, argValues, node.Position);

        // Format the expression
        string expression = _formatter.Format(node);

        // Add the calculation step
        allSteps.Add(new CalculationStep(
            expression,
            description,
            result.ToString()));

        return new StepByStepResult(result, allSteps);
    }

    /// <summary>
    /// Visits a factorial node, evaluates its expression, and returns the result.
    /// </summary>
    /// <param name="node">The factorial node to visit.</param>
    /// <returns>A result containing the factorial value and the calculation steps.</returns>
    /// <exception cref="EvaluationException">Thrown if the value is not a non-negative integer.</exception>
    public override StepByStepResult VisitFactorial(FactorialNode node)
    {
        var innerResult = node.Expression.Accept(this);
        var allSteps = new List<CalculationStep>(innerResult.Steps);

        decimal value = innerResult.Value;

        // Check if value is a non-negative integer
        if (value < 0 || Math.Abs(value - Math.Round(value)) > MathConstants.Epsilon)
        {
            throw new EvaluationException("Factorial is only defined for non-negative integers", node.Position);
        }

        int n = (int)Math.Round(value);

        // Calculate factorial
        decimal rawResult = 1;
        for (int i = 2; i <= n; i++)
        {
            rawResult *= i;
        }

        decimal formattedResult = FormatNumber(rawResult);

        string formatInfo = GetFormatInfo();
        string expression = _formatter.Format(node);

        allSteps.Add(new CalculationStep(
            expression,
            $"Calculate factorial of {n}, {formatInfo}",
            formattedResult.ToString()));

        return new StepByStepResult(formattedResult, allSteps);
    }

    /// <summary>
    /// Visits a summation node and returns the result.
    /// </summary>
    /// <param name="node">The summation node to visit.</param>
    /// <returns>A result containing the summation value and the calculation steps.</returns>
    public override StepByStepResult VisitSummation(SummationNode node)
    {
        return IteratorNodeVisitors.VisitSummation(
            this,
            node,
            Variables,
            _formatter,
            FormatNumber,
            GetFormatInfo);
    }

    /// <summary>
    /// Visits a product node and returns the result.
    /// </summary>
    /// <param name="node">The product node to visit.</param>
    /// <returns>A result containing the product value and the calculation steps.</returns>
    public override StepByStepResult VisitProduct(ProductNode node)
    {
        return IteratorNodeVisitors.VisitProduct(
            this,
            node,
            Variables,
            _formatter,
            FormatNumber,
            GetFormatInfo);
    }

    #endregion
}