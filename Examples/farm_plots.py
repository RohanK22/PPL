
# Script to plot speedups from farm pattern

# Usage: ./mandelbrot_seq <width> <height> <maxIterations>
# Usage: ./mandelbrot <width> <height> <maxIterations> <numRowChunks> <numColChunks> <numWorkers>

import matplotlib.pyplot as plt
import os
import subprocess
import time

# EXEC Paths
FARM_MANDELBROT_EXEC = os.path.join('../cmake-build-debug', 'mandelbrot')
SEQ_MANDELBROT_EXEC = os.path.join('../cmake-build-debug', 'mandelbrot_seq')

# Parameters
SAMPLES = 5
problem_sizes = [(500, 500, 1000), (800, 800, 1000), (1000, 1000, 1000), (1200, 1200, 1000)]
num_workers_list = [2, 4, 6, 8]

# Function to run Mandelbrot and measure elapsed time
def run_mandelbrot(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        # subprocess.run([exec_path] + list(map(str, args)), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        os.system(f'{exec_path} {" ".join(map(str, args))} > /dev/null')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return sum(elapsed_times) / len(elapsed_times)

# Different lines for different problem sizes
for problem_size in problem_sizes:
    width, height, max_iterations = problem_size
    farm_elapsed_times = []

    for num_workers in num_workers_list:
        chunk_row = num_workers
        chunk_col = num_workers

        farm_elapsed_time = run_mandelbrot(FARM_MANDELBROT_EXEC, width, height, max_iterations, chunk_row, chunk_col, num_workers)
        farm_elapsed_times.append(farm_elapsed_time)

    # Calculate speedup
    seq_elapsed_time = run_mandelbrot(SEQ_MANDELBROT_EXEC, width, height, max_iterations)
    speedups = [seq_elapsed_time / elapsed_time for elapsed_time in farm_elapsed_times]

    # Plotting
    plt.plot(num_workers_list, speedups, marker='o', label=f'Problem Size: {width}x{height}')
    print(f'Problem size: {width}x{height} Done!')

# Show legend, title, and labels
plt.legend()
plt.title('Speedup vs Number of Workers for Different Problem Sizes')
plt.xlabel('Number of Workers')
plt.ylabel('Speedup')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('farm_speedup.png')