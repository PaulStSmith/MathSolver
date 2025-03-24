namespace MathSolver2;

/// <summary>
/// Extension methods for the EnhancedStepByStepVisitor to handle iterator nodes.
/// </summary>
public static class IteratorNodeVisitors
{
    /// <summary>
    /// Visits a summation node and records the steps for evaluating the summation.
    /// </summary>
    /// <param name="visitor">The arithmetic visitor used for evaluating expressions.</param>
    /// <param name="node">The summation node to be visited.</param>
    /// <param name="variables">The dictionary of variables and their current values.</param>
    /// <param name="formatter">The formatter used to format expressions.</param>
    /// <param name="formatNumber">A function to format numeric results.</param>
    /// <param name="getFormatInfo">A function to retrieve formatting information.</param>
    /// <returns>A <see cref="StepByStepResult"/> containing the result and the steps taken.</returns>
    public static StepByStepResult VisitSummation(
        this StepByStepArithmeticVisitor visitor,
        SummationNode node,
        Dictionary<string, decimal> variables,
        FormattingVisitor formatter,
        Func<decimal, decimal> formatNumber,
        Func<string> getFormatInfo)
    {
        // Get the start value with steps
        var startResult = node.Start.Accept(visitor);

        // Get the end value with steps
        var endResult = node.End.Accept(visitor);

        // Collect all steps so far
        var allSteps = new List<CalculationStep>(startResult.Steps);
        allSteps.AddRange(endResult.Steps);

        // We need to handle non-integer bounds as an error
        if (Math.Abs(startResult.Value - Math.Round(startResult.Value)) > MathConstants.Epsilon ||
            Math.Abs(endResult.Value - Math.Round(endResult.Value)) > MathConstants.Epsilon)
        {
            throw new EvaluationException("Summation bounds must be integers", node.Position);
        }

        // Convert to integers for the loop
        var startInt = (int)Math.Round(startResult.Value);
        var endInt = (int)Math.Round(endResult.Value);

        // Store the original value of the iteration variable (if it exists)
        var hasOriginalValue = variables.TryGetValue(node.Variable, out var originalValue);

        try
        {
            // Format and add the summation setup step
            var expression = formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Setup summation with {node.Variable} from {startInt} to {endInt}",
                $"Calculate each term and sum"));

            // Initialize the result
            var result = 0m;

            // Evaluate the expression for each value of the iteration variable
            for (var i = startInt; i <= endInt; i++)
            {
                // Set the iteration variable
                variables[node.Variable] = i;

                // Record the current value of the iteration variable
                var currentExpression = $"{node.Variable} = {i}";
                allSteps.Add(new CalculationStep(
                    currentExpression,
                    $"Set iteration variable {node.Variable} to {i}",
                    i.ToString()));

                // Evaluate the expression with the current value
                var termResult = node.Expression.Accept(visitor);

                // Add the term result steps
                allSteps.AddRange(termResult.Steps);

                // Add step for adding this term to the sum
                var newResult = result + termResult.Value;
                var formattedNewResult = formatNumber(newResult);

                allSteps.Add(new CalculationStep(
                    $"sum + {termResult.Value}",
                    $"Add term value {termResult.Value} to current sum {result}, {getFormatInfo()}",
                    formattedNewResult.ToString()));

                // Update the result
                result = formattedNewResult;
            }

            // Add final summation result step
            allSteps.Add(new CalculationStep(
                expression,
                $"Complete summation from {startInt} to {endInt}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }
        finally
        {
            // Restore the original value of the iteration variable
            if (hasOriginalValue)
            {
                variables[node.Variable] = originalValue;
            }
            else
            {
                variables.Remove(node.Variable);
            }
        }
    }

    /// <summary>
    /// Visits a product node and records the steps for evaluating the product.
    /// </summary>
    /// <param name="visitor">The arithmetic visitor used for evaluating expressions.</param>
    /// <param name="node">The product node to be visited.</param>
    /// <param name="variables">The dictionary of variables and their current values.</param>
    /// <param name="formatter">The formatter used to format expressions.</param>
    /// <param name="formatNumber">A function to format numeric results.</param>
    /// <param name="getFormatInfo">A function to retrieve formatting information.</param>
    /// <returns>A <see cref="StepByStepResult"/> containing the result and the steps taken.</returns>
    public static StepByStepResult VisitProduct(
        this StepByStepArithmeticVisitor visitor,
        ProductNode node,
        Dictionary<string, decimal> variables,
        FormattingVisitor formatter,
        Func<decimal, decimal> formatNumber,
        Func<string> getFormatInfo)
    {
        // Get the start value with steps
        var startResult = node.Start.Accept(visitor);

        // Get the end value with steps
        var endResult = node.End.Accept(visitor);

        // Collect all steps so far
        var allSteps = new List<CalculationStep>(startResult.Steps);
        allSteps.AddRange(endResult.Steps);

        // We need to handle non-integer bounds as an error
        if (Math.Abs(startResult.Value - Math.Round(startResult.Value)) > MathConstants.Epsilon ||
            Math.Abs(endResult.Value - Math.Round(endResult.Value)) > MathConstants.Epsilon)
        {
            throw new EvaluationException("Product bounds must be integers", node.Position);
        }

        // Convert to integers for the loop
        var startInt = (int)Math.Round(startResult.Value);
        var endInt = (int)Math.Round(endResult.Value);

        // Store the original value of the iteration variable (if it exists)
        var hasOriginalValue = variables.TryGetValue(node.Variable, out var originalValue);

        try
        {
            // Format and add the product setup step
            var expression = formatter.Format(node);
            allSteps.Add(new CalculationStep(
                expression,
                $"Setup product with {node.Variable} from {startInt} to {endInt}",
                $"Calculate each term and multiply"));

            // Initialize the result
            var result = 1m;

            // Evaluate the expression for each value of the iteration variable
            for (var i = startInt; i <= endInt; i++)
            {
                // Set the iteration variable
                variables[node.Variable] = i;

                // Record the current value of the iteration variable
                var currentExpression = $"{node.Variable} = {i}";
                allSteps.Add(new CalculationStep(
                    currentExpression,
                    $"Set iteration variable {node.Variable} to {i}",
                    i.ToString()));

                // Evaluate the expression with the current value
                var termResult = node.Expression.Accept(visitor);

                // Add the term result steps
                allSteps.AddRange(termResult.Steps);

                // Add step for multiplying this term to the product
                var newResult = result * termResult.Value;
                var formattedNewResult = formatNumber(newResult);

                allSteps.Add(new CalculationStep(
                    $"product * {termResult.Value}",
                    $"Multiply term value {termResult.Value} with current product {result}, {getFormatInfo()}",
                    formattedNewResult.ToString()));

                // Update the result
                result = formattedNewResult;
            }

            // Add final product result step
            allSteps.Add(new CalculationStep(
                expression,
                $"Complete product from {startInt} to {endInt}",
                result.ToString()));

            return new StepByStepResult(result, allSteps);
        }
        finally
        {
            // Restore the original value of the iteration variable
            if (hasOriginalValue)
            {
                variables[node.Variable] = originalValue;
            }
            else
            {
                variables.Remove(node.Variable);
            }
        }
    }
}