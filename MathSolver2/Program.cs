namespace MathSolver2
{
    class Program
    {

        public static void Main(string[] args)
        {
            Console.WriteLine("MathSolver Arithmetic Enhancement Demo");
            Console.WriteLine("======================================");

            // Create a basic expression to test
            string expression = @"e^0.15";
            Console.WriteLine($"Expression: {expression}");
            Console.WriteLine();

            // Test with Normal arithmetic (no formatting)
            Console.WriteLine("Normal Arithmetic:");
            DemoArithmeticMode(expression, ArithmeticType.Normal, 4, false);
            Console.WriteLine();

            // Test with Round to significant digits
            Console.WriteLine("Round to 3 Significant Digits:");
            DemoArithmeticMode(expression, ArithmeticType.Round, 3, true);
            Console.WriteLine();

            Console.WriteLine("\nDemo complete!");
        }

        private static void DemoArithmeticMode(
            string expression,
            ArithmeticType arithmeticType,
            int precision,
            bool useSignificantDigits)
        {
            // Create a solver with the specified arithmetic settings
            EnhancedMathSolver solver = new EnhancedMathSolver(
                arithmeticType,
                precision,
                useSignificantDigits);

            // Evaluate with steps
            CalculationResult result = solver.EvaluateWithSteps(expression);

            // Print the arithmetic settings
            Console.WriteLine($"Arithmetic Mode: {result.ArithmeticMode}");
            Console.WriteLine($"Precision: {result.PrecisionInfo}");
            Console.WriteLine($"Direction: {result.Direction}");

            // Print the result
            Console.WriteLine($"Result: {result.FormattedResult}");

            // Print the steps
            Console.WriteLine("Steps:");
            foreach (var step in result.Steps)
            {
                Console.WriteLine($"  {step.Expression} => {step.Operation} => {step.Result}");
            }
        }


        //static void Main(string[] args)
        //{
        //    // Create a new MathSolver instance
        //    MathSolver solver = new MathSolver();

        //    // Set some variables
        //    solver.SetVariable("x", 10);
        //    solver.SetVariable("y", 5);

        //    Console.WriteLine("Welcome to MathSolver!");
        //    Console.WriteLine("Enter an expression to evaluate or 'exit' to quit.");
        //    Console.WriteLine("You can use variables x and y in your expressions.");
        //    Console.WriteLine("LaTeX commands like \\frac{a}{b} are supported.");

        //    while (true)
        //    {
        //        Console.Write("\nEnter expression: ");
        //        string input = Console.ReadLine();

        //        if (string.IsNullOrWhiteSpace(input) || input.ToLower() == "exit")
        //        {
        //            break;
        //        }

        //        try
        //        {
        //            // Validate the expression
        //            if (!solver.Validate(input, out string error))
        //            {
        //                Console.WriteLine($"Error: {error}");
        //                continue;
        //            }

        //            // Format in LaTeX
        //            Console.WriteLine($"LaTeX format: {solver.Format(input, OutputFormat.LaTeX)}");

        //            // Evaluate step by step
        //            StepByStepResult result = solver.EvaluateStepByStep(input);

        //            Console.WriteLine("\nStep-by-step calculation:");
        //            foreach (var step in result.Steps)
        //            {
        //                Console.WriteLine($"  {step.Expression} => {step.Operation} => {step.Result}");
        //            }

        //            Console.WriteLine($"\nResult: {result.Value}");
        //        }
        //        catch (Exception ex)
        //        {
        //            Console.WriteLine($"Error: {ex.Message}");
        //        }
        //    }

        //    Console.WriteLine("Thank you for using MathSolver!");
        //}
    }
}