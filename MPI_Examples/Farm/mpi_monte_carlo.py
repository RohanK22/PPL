import csv
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import time

# EXEC Paths
FARM_PI_EXEC = '/cs/home/rk76/Documents/PPL/build/nested_farm_in_mpi_farm_monte_carlo'
SEQ_PI_EXEC = '../build/seq_monte_carlo'

# Function to run Monte Carlo Pi calculation and measure elapsed time
def run_pi(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        print('Running:', exec_path)
        subprocess.run(exec_path, shell=True, check=True)  # Shell=True to execute command through shell
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return elapsed_times

# Function to generate MPI run command
def make_mpi_run_command(num_processes, hostflag, exec_path, num_level_farms, num_worker_threads, num_samples):
    command = ['mpirun', '-n', str(num_processes)]
    if hostflag:
        command.extend(['--host', hostflag])
    command.append(exec_path)
    command.append(str(num_samples))
    command.extend([str(num_level_farms), str(num_worker_threads)])
    return command

# Parameters
SAMPLES = 5
num_samples_list = [10000000, 100000000, 1000000000, 10000000000]
num_node_level_farms_list = [1, 2, 3, 4, 5, 6, 7, 8]
num_cores = 6
host_flag = 'pc7-003-l:1,pc7-005-l:1,pc7-007-l:1,pc7-009-l:1,pc7-011-l:1,pc7-015-l:1,pc7-017-l:1,pc7-019-l:1,pc7-020-l:1,pc7-022-l:1,pc7-023-l:1'

# Data storage for mean elapsed times, standard deviations, and sequential elapsed times
data = {}

# Plotting
plt.figure()  # Create a new figure for all data
for num_samples in num_samples_list:
    seq_elapsed_times = run_pi(SEQ_PI_EXEC + ' ' + str(num_samples))  # Compute sequential elapsed time
    seq_elapsed_time_mean = np.mean(seq_elapsed_times)
    seq_elapsed_time_std = np.std(seq_elapsed_times)
    mean_speedups = []
    std_dev_speedups = []
    for num_workers in num_node_level_farms_list:
        # Calculate np (number of processes)
        num_node_level_farms = num_workers
        num_processes = num_node_level_farms + 3  # Adding 3 processes for master, level 1 farm, and level 2 farm

        # Generate MPI run command
        mpi_command = make_mpi_run_command(num_processes, host_flag, FARM_PI_EXEC, num_node_level_farms, num_cores, num_samples)

        # Run Monte Carlo Pi calculation and measure elapsed time
        elapsed_times = run_pi(' '.join(mpi_command))

        # Compute speedup for each run
        run_speedups = [seq_elapsed_times[i] / elapsed_times[i] for i in range(SAMPLES)]

        mean_speedup = np.mean(run_speedups)
        std_dev_speedup = np.std(run_speedups)

        mean_speedups.append(mean_speedup)
        std_dev_speedups.append(std_dev_speedup)

    # Store data for this num_samples in the dictionary
    data[num_samples] = {
        'num_node_level_farms_list': num_node_level_farms_list,
        'mean_speedups': mean_speedups,
        'std_dev_speedups': std_dev_speedups,
        'seq_elapsed_time_mean': seq_elapsed_time_mean,
        'seq_elapsed_time_std': seq_elapsed_time_std
    }

    # Plotting
    plt.errorbar(num_node_level_farms_list, mean_speedups, yerr=std_dev_speedups, fmt='o-', label=f'{num_samples} Samples')

# Show legend, title, and labels
plt.legend(loc='upper left', title='Monte Carlo Samples')
plt.title('Speedup vs Number of MPI Workers for Monte Carlo Pi Calculation')
plt.xlabel('Number of MPI Workers (Node-Level Farms with 6 Worker Threads Each)')
plt.ylabel('Speedup (T_seq / T_mpi)')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('monte_carlo_pi_speedup.png')
print('Plot saved as monte_carlo_pi_speedup.png')

# Save data as JSON
import json
with open('monte_carlo_pi_speedup.json', 'w') as f:
    json.dump(data, f)