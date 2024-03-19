# Script to plot speedups from farm pattern for matrix multiplication

import matplotlib.pyplot as plt
import os
import time

# EXEC Paths
FARM_MATMUL_EXEC = os.path.join('../build', 'farm_matmul')
SEQ_MATMUL_EXEC = os.path.join('../build', 'seq_matmul')

# Parameters
SAMPLES = 5
matrix_dimensions = [128, 256, 512, 1024]
    #[512, 1024, 2048]
num_workers_list = [1, 2, 4, 6, 8, 12, 14, 16, 18, 20, 22, 24, 26, 28]

# Function to run Matrix Multiplication and measure elapsed time
def run_matmul(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        os.system(f'{exec_path} {" ".join(map(str, args))} > /dev/null')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return sum(elapsed_times) / len(elapsed_times)

# Different lines for different matrix dimensions
for dim in matrix_dimensions:
    matmul_elapsed_times = []

    for num_workers in num_workers_list:
        matmul_elapsed_time = run_matmul(FARM_MATMUL_EXEC, dim, num_workers)
        matmul_elapsed_times.append(matmul_elapsed_time)

    # Calculate speedup
    seq_elapsed_time = run_matmul(SEQ_MATMUL_EXEC, dim)
    speedups = [seq_elapsed_time / elapsed_time for elapsed_time in matmul_elapsed_times]

    # Plotting
    plt.plot(num_workers_list, speedups, marker='o', label=f'Matrix Dimension: {dim}x{dim}')
    print(f'Matrix Dimension: {dim}x{dim} Done!')

# Show legend, title, and labels
plt.legend()
plt.title('Node-Level Farm Speedup vs Number of Workers for Matrix Multiplication Problem')
plt.xlabel('Number of Workers (Threads)')
plt.ylabel('Speedup (Time_seq / Time_farm)')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('matmul_speedup.png')
