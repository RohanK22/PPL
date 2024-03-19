// Sequential Monte Carlo Pi calculation

// time ./seq_monte_carlo 10000000000

#include <iostream>
#include <cmath>
#include <cstdlib>

#define ull unsigned long long

double estimatePiSequential(ull num_samples)
{
    ull inside_circle = 0;

    for (ull i = 0; i < num_samples; ++i)
    {
        double x = static_cast<double>(rand()) / RAND_MAX;
        double y = static_cast<double>(rand()) / RAND_MAX;

        if (std::sqrt(x * x + y * y) <= 1.0)
        {
            inside_circle++;
        }
    }

    return (4.0 * inside_circle) / num_samples;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <num_samples>" << std::endl;
        return 1;
    }

    srand(100);

    ull num_samples = std::stoull(argv[1]);

    double estimated_pi = estimatePiSequential(num_samples);

    std::cout << "Estimated Pi (Sequential): " << estimated_pi << std::endl;

    return 0;
}
