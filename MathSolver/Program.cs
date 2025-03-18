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
            // Additional test expressions
            TestExpression(@"(0.000125 + 30.78666 - 0.080945) + (0.855^2 - \frac{0.75}/{8})^3", ArithmeticType.Truncate, 4, false);
            TestExpression(@"\sum_{i=1}^{10} \frac{1}/{2^i}", ArithmeticType.Normal, 10, false);
            TestExpression(@"\sum_{i=1}^{10} \frac{1}/{2^i}", ArithmeticType.Round, 3, true);
            TestExpression(@"\sum_{i=1}^{10} \frac{1}/{2^i}", ArithmeticType.Truncate, 3, true);

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