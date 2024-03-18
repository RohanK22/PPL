import matplotlib.pyplot as plt
import subprocess
import numpy as np

# EXEC Paths
FARM_PI_EXEC = '../cmake-build-debug/farm_monte_carlo'
SEQ_PI_EXEC = '../cmake-build-debug/seq_monte_carlo'

# Parameters
SAMPLES = 10
num_samples_list = [10000000, 50000000, 100000000]
num_workers_list = [2, 4, 6, 8]

# Function to run Monte Carlo Pi calculation and measure elapsed time
def run_pi(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        subprocess.run([exec_path, *map(str, args)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return np.mean(elapsed_times), np.std(elapsed_times)

# Plotting
for num_samples in num_samples_list:
    plt.figure()  # Create a new figure for each set of data
    pi_elapsed_times = []
    std_devs = []

    for num_workers in num_workers_list:
        pi_elapsed_time, std_dev = run_pi(FARM_PI_EXEC, num_samples, num_workers)
        pi_elapsed_times.append(pi_elapsed_time)
        std_devs.append(std_dev)

    # Calculate speedup
    seq_elapsed_time, _ = run_pi(SEQ_PI_EXEC, num_samples)
    speedups = [seq_elapsed_time / elapsed_time for elapsed_time in pi_elapsed_times]

    # Plotting with error bars
    plt.errorbar(num_workers_list, speedups, yerr=std_devs, fmt='o-', label=f'Num Samples: {num_samples}')
    print(f'Num Samples: {num_samples} Done!')

    # Show legend, title, and labels
    plt.legend()
    plt.title('Speedup vs Number of Workers for Different Number of Samples')
    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')
    plt.grid(True)
    plt.show()

    # Save the plot
    plt.savefig(f'pi_speedup_{num_samples}.png')
