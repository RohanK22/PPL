import matplotlib.pyplot as plt
import os
import time

# EXEC Paths
FARM_DOT_EXEC = os.path.join('../../cmake-build-debug', 'farm_dot_product')
SEQ_DOT_EXEC = os.path.join('../../cmake-build-debug', 'seq_dot_product')

# Parameters
SAMPLES = 10
vector_sizes = [6400000, 12800000, 25600000, 51200000, 102400000]
num_workers_list = [2, 4, 6, 8]

# Function to run Dot Product and measure elapsed time
def run_dot(exec_path, *args):
    elapsed_times = []
    for _ in range(SAMPLES):
        start_time = time.time()
        os.system(f'{exec_path} {" ".join(map(str, args))} > /dev/null')
        elapsed_time = time.time() - start_time
        elapsed_times.append(elapsed_time)
    return sum(elapsed_times) / len(elapsed_times)

# Different lines for different vector sizes
for size in vector_sizes:
    dot_elapsed_times = []

    for num_workers in num_workers_list:
        dot_elapsed_time = run_dot(FARM_DOT_EXEC, size, num_workers)
        dot_elapsed_times.append(dot_elapsed_time)

    # Calculate speedup
    seq_elapsed_time = run_dot(SEQ_DOT_EXEC, size)
    speedups = [seq_elapsed_time / elapsed_time for elapsed_time in dot_elapsed_times]

    # Plotting
    plt.plot(num_workers_list, speedups, marker='o', label=f'Vector Size: {size}')
    print(f'Vector Size: {size} Done!')

# Show legend, title, and labels
plt.legend()
plt.title('Speedup vs Number of Workers for Different Vector Sizes')
plt.xlabel('Number of Workers')
plt.ylabel('Speedup')
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('dot_speedup.png')
