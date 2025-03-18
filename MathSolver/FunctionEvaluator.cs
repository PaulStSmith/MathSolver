using System;
using System.Collections.Generic;
using System.Linq;

namespace MathSolver
{
    /// <summary>
    /// Evaluates mathematical functions with support for extensibility through a delegate-based approach
    /// </summary>
    public class FunctionEvaluator
    {
        private readonly Dictionary<string, Func<double[], double>> _functions;
        private readonly ArithmeticHandler _arithmeticHandler;

        /// <summary>
        /// Creates a new function evaluator with default functions
        /// </summary>
        /// <param name="arithmeticHandler">Handler for number formatting</param>
        public FunctionEvaluator(ArithmeticHandler arithmeticHandler)
        {
            _arithmeticHandler = arithmeticHandler ?? throw new ArgumentNullException(nameof(arithmeticHandler));
            _functions = new Dictionary<string, Func<double[], double>>(StringComparer.OrdinalIgnoreCase);

            // Register basic mathematical functions
            RegisterDefaultFunctions();
        }

        /// <summary>
        /// Registers a function implementation with the evaluator
        /// </summary>
        /// <param name="name">Name of the function</param>
        /// <param name="implementation">Function implementation that accepts an array of double arguments</param>
        public void RegisterFunction(string name, Func<double[], double> implementation)
        {
            if (string.IsNullOrWhiteSpace(name))
            {
                throw new ArgumentException("Function name cannot be empty", nameof(name));
            }

            _functions[name] = implementation ?? throw new ArgumentNullException(nameof(implementation));
        }

        /// <summary>
        /// Evaluates a function with the provided arguments
        /// </summary>
        /// <param name="functionName">Name of the function to evaluate</param>
        /// <param name="arguments">Arguments to pass to the function</param>
        /// <returns>Result of the function evaluation</returns>
        public double Evaluate(string functionName, double[] arguments)
        {
            if (string.IsNullOrWhiteSpace(functionName))
            {
                throw new ArgumentException("Function name cannot be empty", nameof(functionName));
            }

            if (_functions.TryGetValue(functionName, out var function))
            {
                double result = function(arguments);
                return _arithmeticHandler.FormatNumber(result);
            }

            throw new InvalidOperationException($"Function '{functionName}' is not registered");
        }

        /// <summary>
        /// Calculates the numerical derivative of a function at a specific point
        /// </summary>
        /// <param name="function">The function to differentiate</param>
        /// <param name="x">The point at which to calculate the derivative</param>
        /// <param name="order">The order of the derivative (1 for first derivative, 2 for second, etc.)</param>
        /// <param name="h">Step size for numerical differentiation (defaults to a small value)</param>
        /// <returns>The numerical derivative value</returns>
        public double NumericalDerivative(Func<double, double> function, double x, int order = 1, double h = 1e-6)
        {
            if (order < 1)
            {
                throw new ArgumentException("Derivative order must be at least 1", nameof(order));
            }

            // First order derivative using central difference
            if (order == 1)
            {
                return (function(x + h) - function(x - h)) / (2 * h);
            }

            // Higher order derivatives calculated recursively
            Func<double, double> firstDerivative = t => NumericalDerivative(function, t, 1, h);
            return NumericalDerivative(firstDerivative, x, order - 1, h);
        }

        /// <summary>
        /// Calculates a Taylor series approximation of a function around a point
        /// </summary>
        /// <param name="function">The function to approximate</param>
        /// <param name="x0">The point around which to expand the series</param>
        /// <param name="x">The point at which to evaluate the series</param>
        /// <param name="terms">Number of terms to include in the approximation</param>
        /// <returns>The Taylor series approximation value</returns>
        public double TaylorSeries(Func<double, double> function, double x0, double x, int terms)
        {
            if (terms < 1)
            {
                throw new ArgumentException("Number of terms must be at least 1", nameof(terms));
            }

            double result = function(x0);
            double dx = x - x0;
            double factorial = 1;
            double power = 1;

            for (int i = 1; i < terms; i++)
            {
                double derivative = NumericalDerivative(function, x0, i);
                factorial *= i;
                power *= dx;
                result += derivative * power / factorial;
            }

            return _arithmeticHandler.FormatNumber(result);
        }

        /// <summary>
        /// Registers the default set of mathematical functions
        /// </summary>
        private void RegisterDefaultFunctions()
        {
            // Basic arithmetic and algebra
            RegisterFunction("abs", args => Math.Abs(args[0]));
            RegisterFunction("sqrt", args => Math.Sqrt(args[0]));
            RegisterFunction("cbrt", args => Math.Pow(args[0], 1.0 / 3.0));
            RegisterFunction("pow", args => Math.Pow(args[0], args[1]));
            RegisterFunction("exp", args => Math.Exp(args[0]));
            RegisterFunction("ln", args => Math.Log(args[0]));
            RegisterFunction("log", args => args.Length > 1
                ? Math.Log(args[0]) / Math.Log(args[1])  // Log with custom base
                : Math.Log10(args[0]));                  // Log base 10

            // Trigonometric functions
            RegisterFunction("sin", args => Math.Sin(args[0]));
            RegisterFunction("cos", args => Math.Cos(args[0]));
            RegisterFunction("tan", args => Math.Tan(args[0]));
            RegisterFunction("asin", args => Math.Asin(args[0]));
            RegisterFunction("acos", args => Math.Acos(args[0]));
            RegisterFunction("atan", args => Math.Atan(args[0]));
            RegisterFunction("atan2", args => Math.Atan2(args[0], args[1]));

            // Hyperbolic functions
            RegisterFunction("sinh", args => Math.Sinh(args[0]));
            RegisterFunction("cosh", args => Math.Cosh(args[0]));
            RegisterFunction("tanh", args => Math.Tanh(args[0]));

            // Rounding functions
            RegisterFunction("floor", args => Math.Floor(args[0]));
            RegisterFunction("ceil", args => Math.Ceiling(args[0]));
            RegisterFunction("round", args => args.Length > 1
                ? Math.Round(args[0], (int)args[1], MidpointRounding.AwayFromZero)
                : Math.Round(args[0], MidpointRounding.AwayFromZero));
            RegisterFunction("trunc", args => Math.Truncate(args[0]));

            // Statistics functions
            RegisterFunction("min", args => args.Min());
            RegisterFunction("max", args => args.Max());
            RegisterFunction("sum", args => args.Sum());
            RegisterFunction("avg", args => args.Average());

            // Factorial already implemented in ArithmeticHandler
            RegisterFunction("factorial", args =>
            {
                if (args[0] < 0 || args[0] != Math.Floor(args[0]))
                {
                    throw new ArgumentException("Factorial is only defined for non-negative integers");
                }
                return _arithmeticHandler.Factorial((int)args[0]);
            });

            // Special functions
            RegisterFunction("gcd", args =>
            {
                if (args.Length < 2) return args[0];
                double result = args[0];
                for (int i = 1; i < args.Length; i++)
                {
                    result = Gcd(result, args[i]);
                }
                return result;
            });

            RegisterFunction("lcm", args =>
            {
                if (args.Length < 2) return args[0];
                double result = args[0];
                for (int i = 1; i < args.Length; i++)
                {
                    result = Lcm(result, args[i]);
                }
                return result;
            });
        }

        /// <summary>
        /// Calculates the greatest common divisor of two numbers
        /// </summary>
        private double Gcd(double a, double b)
        {
            // Ensure we're working with integers
            if (a != Math.Floor(a) || b != Math.Floor(b))
            {
                throw new ArgumentException("GCD is only defined for integers");
            }

            a = Math.Abs(a);
            b = Math.Abs(b);

            while (b > 0)
            {
                double temp = b;
                b = a % b;
                a = temp;
            }

            return a;
        }

        /// <summary>
        /// Calculates the least common multiple of two numbers
        /// </summary>
        private double Lcm(double a, double b)
        {
            // Ensure we're working with integers
            if (a != Math.Floor(a) || b != Math.Floor(b))
            {
                throw new ArgumentException("LCM is only defined for integers");
            }

            a = Math.Abs(a);
            b = Math.Abs(b);

            return (a * b) / Gcd(a, b);
        }
    }
}