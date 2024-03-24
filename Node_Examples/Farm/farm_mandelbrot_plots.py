
# Script to plot speedups from farm pattern for the mandelbrot problem

# Usage: ./mandelbrot_seq <width> <height> <maxIterations>
# Usage: ./mandelbrot <width> <height> <maxIterations> <numRowChunks> <numColChunks> <numWorkers>

import matplotlib.pyplot as plt
import os
import time
import numpy as np
import json

# EXEC Paths
FARM_MANDELBROT_EXEC = os.path.join('../../build', 'mandelbrot')
SEQ_MANDELBROT_EXEC = os.path.join('../../build', 'mandelbrot_seq')

# Parameters
SAMPLES = 5
problem_sizes = [(200, 200, 1000), (400, 400, 1000), (800, 800, 1000), (1600, 1600, 1000)]
# [(1024, 1024, 1000), (2048, 2048, 1000), (4096, 4096, 1000)]
num_workers_list = [1, 2, 4, 6, 8, 12, 14, 16, 18, 20, 22, 24, 26, 28]

data = {}

# Function to run Mandelbrot and measure elapsed time
def run_mandelbrot(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        os.system(f'{exec_path} {" ".join(map(str, args))} > /dev/null')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return elapsed_times

# Different lines for different problem sizes
for problem_size in problem_sizes:
    width, height, max_iterations = problem_size

    seq_elapsed_times = run_mandelbrot(SEQ_MANDELBROT_EXEC, width, height, max_iterations)
    seq_elapsed_time_mean = np.mean(seq_elapsed_times)
    seq_elapsed_time_std = np.std(seq_elapsed_times)

    farm_elapsed_times = []
    run_speedups = []
    mean_speedups = []
    std_dev_speedups = []
    for num_workers in num_workers_list:
        chunk_row = num_workers
        chunk_col = num_workers

        mandelbrot_farm_elapsed_times = run_mandelbrot(FARM_MANDELBROT_EXEC, width, height, max_iterations, chunk_row, chunk_col, num_workers)
        
        run_speedups = [seq_elapsed_times[i] / mandelbrot_farm_elapsed_times[i] for i in range(SAMPLES)]

        mean_speedup = np.mean(run_speedups)
        std_dev_speedup = np.std(run_speedups)

        mean_speedups.append(mean_speedup)
        std_dev_speedups.append(std_dev_speedup)

        farm_elapsed_times.append(mandelbrot_farm_elapsed_times)
        run_speedups.append(run_speedups)

    # Plotting
    plt.errorbar(num_workers_list, mean_speedups, yerr=std_dev_speedups, label=f'Image size: {width}x{height}')
    print(f'Problem size: {width}x{height} Done!')

    farm_elapsed_time_mean = np.mean(farm_elapsed_times)
    farm_elapsed_time_std = np.std(farm_elapsed_times)

    data[str(problem_size[0]) + 'x' + str(problem_size[1])] = {
        'num_workers_list': num_workers_list,
        'mean_speedups': mean_speedups,
        'std_dev_speedups': std_dev_speedups,
        'farm_elapsed_times': farm_elapsed_times,
        'farm_elapsed_time_mean': farm_elapsed_time_mean,
        'farm_elapsed_time_std': farm_elapsed_time_std,
        'seq_elapsed_times': seq_elapsed_times,
        'seq_elapsed_time_mean': seq_elapsed_time_mean,
        'seq_elapsed_time_std': seq_elapsed_time_std
    }

# Show legend, title, and labels
plt.legend()
plt.title('Speedup vs Number of Workers for Mandelbrot Image Generation')
plt.xlabel('Number of Workers (#threads)')
plt.ylabel('Speedup (T_seq / T_farm)')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('mandelbrot_farm_speedup.png')

# Save data as JSON
with open('mandelbrot_farm_speedup_data.json', 'w') as f:
    json.dump(data, f, indent=4)