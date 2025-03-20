namespace MathSolver2
{
    /// <summary>
    /// Provides standard mathematical constants
    /// </summary>
    public static class MathConstants
    {
        /// <summary>
        /// Represents a small value used for precision comparisons.
        /// </summary>
        internal const decimal Epsilon = 0.0000000001m;

        /// <summary>
        /// The constant π (pi), representing the ratio of a circle's circumference to its diameter
        /// </summary>
        public const decimal Pi = 3.1415926535897932384626433832795028841971693993751058m;

        /// <summary>
        /// The constant e, the base of the natural logarithm
        /// </summary>
        public const decimal E = 2.7182818284590452353602874713526624977572470936999595m;

        /// <summary>
        /// The golden ratio φ (phi)
        /// </summary>
        public const decimal Phi = 1.6180339887498948482045868343656381177203091798057628m;

        /// <summary>
        /// Gets the raw value of a known math constant.
        /// </summary>
        /// <param name="name">The name of the constant (e.g., "pi", "e", "phi").</param>
        /// <param name="value">The output parameter that will hold the constant's value if found.</param>
        /// <returns>
        /// True if the constant is found; otherwise, false.
        /// </returns>
        public static bool TryGetValue(string name, out decimal value)
        {
            switch (name.ToLowerInvariant())
            {
                case "pi":
                case "π":
                    value = Pi;
                    return true;
                case "e":
                    value = E;
                    return true;
                case "phi":
                case "φ":
                    value = Phi;
                    return true;
                default:
                    value = 0;
                    return false;
            }
        }
    }
}