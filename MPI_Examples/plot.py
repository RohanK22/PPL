import json
import matplotlib.pyplot as plt
import numpy as np

# Read JSON data from file
with open('monte_carlo_pi_speedup.json', 'r') as file:
    data = json.load(file)

fig, ax = plt.subplots()

for key, value in data.items():
    num_workers = np.array(value["num_node_level_farms_list"])
    speedups = np.array(value["mean_speedups"])
    std_dev = np.array(value["std_dev_speedups"])
    seq_elapsed_time_mean = value["seq_elapsed_time_mean"]
    seq_elapsed_time_std = value["seq_elapsed_time_std"]

    # Calculate the error bars
    lower_error = std_dev
    upper_error = std_dev

    # Plot with error bars
    ax.errorbar(num_workers, speedups, yerr=[lower_error, upper_error], label=key)

ax.set_xlabel('Number of Workers')
ax.set_ylabel('Speedup')
ax.set_title('Speedup vs Num Workers')
ax.legend()
plt.show()

plt.savefig('monte_carlo_pi_speedup____.png')