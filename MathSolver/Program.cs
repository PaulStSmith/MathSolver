using System;
using System.Collections.Generic;
using System.Linq;

namespace MathSolver
{
    /// <summary>
    /// Example usage of the refactored MathSolver
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            // Test with the example from the requirements
            string exampleExpr = "1 + 2 + 3 * 4 + 5";
            Console.WriteLine("Example Expression Test:");

            // Create solver with right-to-left evaluation and truncate with 1 decimal place
            var solver = new MathSolver(
                ArithmeticType.Truncate,
                1,
                false,
                CalculationDirection.RightToLeft);

            var result = solver.SolveWithSteps(exampleExpr);
            Console.WriteLine($"Original Expression: {result.OriginalExpression}");
            Console.WriteLine($"Arithmetic Mode: {result.ArithmeticMode}");
            Console.WriteLine($"Direction: {result.Direction}");

            Console.WriteLine("\nCalculation Steps:");
            foreach (var step in result.Steps)
            {
                Console.WriteLine($"{step.Expression} = {step.Result}");
            }

            Console.WriteLine($"\nFormatted Result: {result.FormattedResult}");
            Console.WriteLine($"Actual Result (without formatting): {result.ActualResult}");

            Console.WriteLine("\n====================================\n");

            // Additional test expressions
            TestExpression("2.5 * 3.6 / 1.2 + 4.8", ArithmeticType.Truncate, 2, false);
            TestExpression("9.75 - 3.25 + 7.5 / 2.5", ArithmeticType.Round, 1, true);
            TestExpression("\\frac{2.5 + 3.7}{1.5}", ArithmeticType.Truncate, 3, false);

            // Test summation operations
            TestExpression("\\sum_{i=1}^{10} \\frac{1}{2^i}", ArithmeticType.Truncate, 6, false);
            TestExpression("\\sum_{i=1}^{5} i^2", ArithmeticType.Truncate, 2, false);

            // Test factorial operation
            TestExpression("5!", ArithmeticType.Normal, 0, false);

            // Additional tests using new features

            // Test Taylor series
            Console.WriteLine("\nTaylor Series Test:");
            var taylorSolver = new MathSolver(ArithmeticType.Truncate, 6, false);
            double taylorResult = taylorSolver.CalculateTaylorSeries("sin(x)", "x", 0, 0.5, 5);
            Console.WriteLine($"Taylor series for sin(x) around 0, evaluated at 0.5 with 5 terms: {taylorResult}");
            Console.WriteLine($"Actual sin(0.5): {Math.Sin(0.5)}");

            // Test custom function registration
            Console.WriteLine("\nCustom Function Test:");
            var customSolver = new MathSolver();
            customSolver.RegisterFunction("quadratic", args => args[0] * args[0] + args[1] * args[0] + args[2]);
            double customResult = customSolver.Solve("quadratic(2, 3, 4)");
            Console.WriteLine($"quadratic(2, 3, 4) = 2² + 3·2 + 4 = {customResult}");

            Console.ReadLine();
        }

        static void TestExpression(
            string expression,
            ArithmeticType arithmeticType,
            int precision,
            bool useSignificantDigits)
        {
            Console.WriteLine($"Testing: {expression}");

            // Test left-to-right
            var leftToRightSolver = new MathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.LeftToRight);

            var leftToRightResult = leftToRightSolver.SolveWithSteps(expression);
            Console.WriteLine("LEFT-TO-RIGHT EVALUATION:");
            Console.WriteLine($"Original Expression: {leftToRightResult.OriginalExpression}");
            Console.WriteLine($"Arithmetic Mode: {leftToRightResult.ArithmeticMode}");
            Console.WriteLine($"Direction: {leftToRightResult.Direction}");

            Console.WriteLine("\nCalculation Steps:");
            foreach (var step in leftToRightResult.Steps)
            {
                Console.WriteLine($"{step.Expression} = {step.Result}");
            }

            Console.WriteLine($"\nFormatted Result: {leftToRightResult.FormattedResult}");
            Console.WriteLine($"Actual Result (without formatting): {leftToRightResult.ActualResult}");

            // Test right-to-left
            var rightToLeftSolver = new MathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.RightToLeft);

            var rightToLeftResult = rightToLeftSolver.SolveWithSteps(expression);
            Console.WriteLine("\nRIGHT-TO-LEFT EVALUATION:");
            Console.WriteLine($"Original Expression: {rightToLeftResult.OriginalExpression}");
            Console.WriteLine($"Arithmetic Mode: {rightToLeftResult.ArithmeticMode}");
            Console.WriteLine($"Direction: {rightToLeftResult.Direction}");

            Console.WriteLine("\nCalculation Steps:");
            foreach (var step in rightToLeftResult.Steps)
            {
                Console.WriteLine($"{step.Expression} = {step.Result}");
            }

            Console.WriteLine($"\nFormatted Result: {rightToLeftResult.FormattedResult}");
            Console.WriteLine($"Actual Result (without formatting): {rightToLeftResult.ActualResult}");

            Console.WriteLine("\n====================================\n");
        }
    }
}