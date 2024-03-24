import matplotlib.pyplot as plt
import os
import time
import numpy as np

# EXEC Paths
FARM_PI_EXEC = os.path.join('../../build', 'farm_monte_carlo')
SEQ_PI_EXEC = os.path.join('../../build', 'seq_monte_carlo')

# Parameters
SAMPLES = 5
num_samples_list = [10000000, 100000000, 1000000000, 10000000000]
num_workers_list = [1, 2, 3, 4, 5, 6]

# Function to run Monte Carlo Pi calculation and measure elapsed time
def run_pi(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        os.system(f'{exec_path} {" ".join(map(str, args))}')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return elapsed_times

# Data storage for mean elapsed times, standard deviations, and sequential elapsed times
data = {}

# Different lines for different number of samples
for num_samples in num_samples_list:
    seq_elapsed_times = run_pi(SEQ_PI_EXEC, num_samples)
    seq_elapsed_time_mean = np.mean(seq_elapsed_times)
    seq_elapsed_time_std = np.std(seq_elapsed_times)

    mean_speedups = []
    std_dev_speedups = []
    for num_workers in num_workers_list:
        farm_elapsed_times = run_pi(FARM_PI_EXEC, num_samples, num_workers)
        farm_elapsed_time_mean = np.mean(farm_elapsed_times)
        farm_elapsed_time_std = np.std(farm_elapsed_times)
        
        # Compute speedup for each run
        run_speedups = [seq_elapsed_times[i] / farm_elapsed_times[i] for i in range(SAMPLES)]
        
        mean_speedup = np.mean(run_speedups)
        std_dev_speedup = np.std(run_speedups)

        mean_speedups.append(mean_speedup)
        std_dev_speedups.append(std_dev_speedup)
    
    # Plotting with error bars
    plt.errorbar(num_workers_list, mean_speedups, yerr=std_dev_speedups, label=f'{num_samples} samples')

    data[num_samples] = {
        'num_workers_list': num_workers_list,
        'mean_speedups': mean_speedups,
        'std_dev_speedups': std_dev_speedups,
        'seq_elapsed_time_mean': seq_elapsed_time_mean,
        'seq_elapsed_time_std': seq_elapsed_time_std
    }
    
    print('Speedups for problem size', num_samples, ':', mean_speedups)

# Show legend, title, and labels
plt.legend()
plt.title('Speedup vs Number of Node-Farm Workers for Different Number of Monte Carlo Samples')
plt.xlabel('Number of Node-Level Workers (Number of worker threads)')
plt.ylabel('Speedup  (T_seq / T_node_farm)')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('node_farm_pi_speedup.png')


# Save data as JSON
import json
with open('node_farm_pi_speedup.json', 'w') as f:
    json.dump(data, f)