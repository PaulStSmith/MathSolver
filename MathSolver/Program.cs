using System;
using System.Collections.Generic;
using System.Linq;

namespace MathSolver
{
    /// <summary>
    /// Example usage of the StepByStepMathSolver
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            // Test with the example from the requirements
            string exampleExpr = "1 + 2 + 3 * 4 + 5";
            Console.WriteLine("Example Expression Test:");

            // Create solver with right-to-left evaluation and truncate with 1 decimal place
            var solver = new StepByStepMathSolver(
                ArithmeticType.Truncate,
                1,
                false,
                CalculationDirection.RightToLeft);

            var result = solver.SolveWithSteps(exampleExpr);
            Console.WriteLine(result);
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
            var leftToRightSolver = new StepByStepMathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.LeftToRight);

            var leftToRightResult = leftToRightSolver.SolveWithSteps(expression);
            Console.WriteLine("LEFT-TO-RIGHT EVALUATION:");
            Console.WriteLine(leftToRightResult);

            // Test right-to-left
            var rightToLeftSolver = new StepByStepMathSolver(
                arithmeticType,
                precision,
                useSignificantDigits,
                CalculationDirection.RightToLeft);

            var rightToLeftResult = rightToLeftSolver.SolveWithSteps(expression);
            Console.WriteLine("\nRIGHT-TO-LEFT EVALUATION:");
            Console.WriteLine(rightToLeftResult);

            Console.WriteLine("\n====================================\n");
        }
    }
}