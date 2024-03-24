import matplotlib.pyplot as plt
import os
import time
import numpy as np
import json

# Function to run Matrix Multiplication and measure elapsed time
def run_matmul(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        os.system(f'{exec_path} {" ".join(map(str, args))} > /dev/null')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return elapsed_times

# EXEC Paths
FARM_MATMUL_EXEC = os.path.join('../../build', 'farm_matmul')
SEQ_MATMUL_EXEC = os.path.join('../../build', 'seq_matmul')

# Parameters
SAMPLES = 5
matrix_dimensions = [128, 256, 512, 1024]
num_workers_list = [1, 2, 4, 6, 8, 12, 14, 16, 18, 20, 22, 24, 26, 28]

data = {}

# Different lines for different matrix dimensions
for dim in matrix_dimensions:
    seq_elapsed_times = run_matmul(SEQ_MATMUL_EXEC, dim)
    seq_elapsed_time_mean = np.mean(seq_elapsed_times)
    seq_elapsed_time_std = np.std(seq_elapsed_times)

    farm_elapsed_times = [] # array of arrays
    run_speedups = [] # array of arrays
    mean_speedups = []
    std_dev_speedups = []
    for num_workers in num_workers_list:
        matmul_elapsed_times = run_matmul(FARM_MATMUL_EXEC, dim, num_workers)
        
        # Compute speedup for each run
        run_speedups = [seq_elapsed_times[i] / matmul_elapsed_times[i] for i in range(SAMPLES)]

        mean_speedup = np.mean(run_speedups)
        std_dev_speedup = np.std(run_speedups)

        mean_speedups.append(mean_speedup)
        std_dev_speedups.append(std_dev_speedup)

        farm_elapsed_times.append(matmul_elapsed_times)
        run_speedups.append(run_speedups)

    # Plotting
    plt.errorbar(num_workers_list, mean_speedups, yerr=std_dev_speedups, label=f'Matrix Dimension: {dim}x{dim}')
    print(f'Matrix Dimension: {dim}x{dim} Done!')

    farm_elapsed_time_mean = np.mean(farm_elapsed_times)
    farm_elapsed_time_std = np.std(farm_elapsed_times)

    data[dim] = {
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
plt.title('Speedup vs Number of Workers for Matrix Multiplication Problem')
plt.xlabel('Number of Node-Level Workers (#threads)')
plt.ylabel('Speedup (T_seq / T_farm)')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('matmul_speedup.png')

# Save data as JSON
with open('matmul_speedup_data_2.json', 'w') as f:
    json.dump(data, f, indent=4)
