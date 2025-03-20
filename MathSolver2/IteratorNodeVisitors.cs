using System;
using System.Collections.Generic;
using System.Text;

namespace MathSolver2
{
    /// <summary>
    /// Extension methods for the EnhancedStepByStepVisitor to handle iterator nodes
    /// </summary>
    public static class IteratorNodeVisitors
    {
        /// <summary>
        /// Visits a summation node and records the steps
        /// </summary>
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
            if (Math.Abs(startResult.Value - Math.Round(startResult.Value)) > Precision.Epsilon ||
                Math.Abs(endResult.Value - Math.Round(endResult.Value)) > Precision.Epsilon)
            {
                throw new EvaluationException("Summation bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(startResult.Value);
            int endInt = (int)Math.Round(endResult.Value);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Format and add the summation setup step
                string expression = formatter.Format(node);
                allSteps.Add(new CalculationStep(
                    expression,
                    $"Setup summation with {node.Variable} from {startInt} to {endInt}",
                    $"Calculate each term and sum"));

                // Initialize the result
                decimal result = 0;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    variables[node.Variable] = i;

                    // Record the current value of the iteration variable
                    string currentExpression = $"{node.Variable} = {i}";
                    allSteps.Add(new CalculationStep(
                        currentExpression,
                        $"Set iteration variable {node.Variable} to {i}",
                        i.ToString()));

                    // Evaluate the expression with the current value
                    var termResult = node.Expression.Accept(visitor);

                    // Add the term result steps
                    allSteps.AddRange(termResult.Steps);

                    // Add step for adding this term to the sum
                    decimal newResult = result + termResult.Value;
                    decimal formattedNewResult = formatNumber(newResult);

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
        /// Visits a product node and records the steps
        /// </summary>
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
            if (Math.Abs(startResult.Value - Math.Round(startResult.Value)) > Precision.Epsilon ||
                Math.Abs(endResult.Value - Math.Round(endResult.Value)) > Precision.Epsilon)
            {
                throw new EvaluationException("Product bounds must be integers", node.Position);
            }

            // Convert to integers for the loop
            int startInt = (int)Math.Round(startResult.Value);
            int endInt = (int)Math.Round(endResult.Value);

            // Store the original value of the iteration variable (if it exists)
            bool hasOriginalValue = variables.TryGetValue(node.Variable, out decimal originalValue);

            try
            {
                // Format and add the product setup step
                string expression = formatter.Format(node);
                allSteps.Add(new CalculationStep(
                    expression,
                    $"Setup product with {node.Variable} from {startInt} to {endInt}",
                    $"Calculate each term and multiply"));

                // Initialize the result
                decimal result = 1;

                // Evaluate the expression for each value of the iteration variable
                for (int i = startInt; i <= endInt; i++)
                {
                    // Set the iteration variable
                    variables[node.Variable] = i;

                    // Record the current value of the iteration variable
                    string currentExpression = $"{node.Variable} = {i}";
                    allSteps.Add(new CalculationStep(
                        currentExpression,
                        $"Set iteration variable {node.Variable} to {i}",
                        i.ToString()));

                    // Evaluate the expression with the current value
                    var termResult = node.Expression.Accept(visitor);

                    // Add the term result steps
                    allSteps.AddRange(termResult.Steps);

                    // Add step for multiplying this term to the product
                    decimal newResult = result * termResult.Value;
                    decimal formattedNewResult = formatNumber(newResult);

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
}