# Run ../../cmake-build-debug/seq_text_processing
# Run ../../cmake-build-debug/pipeline_text_processing

import os
import time
import numpy as np

SAMPLES = 5
seq_times = []
pipeline_times = []

for i in range(SAMPLES):
    start_time = time.time()
    # subprocess exec
    os.system('./seq_text_processing')
    elapsed_time = time.time() - start_time
    seq_times.append(elapsed_time)

    start_time = time.time()
    os.system('./pipeline_text_processing')
    elapsed_time = time.time() - start_time
    pipeline_times.append(elapsed_time)

    print(f'Iteration {i + 1} Done!')

speedups = [seq_times[i] / pipeline_times[i] for i in range(SAMPLES)]

print(f'Seq Times: {seq_times}')
print(f'Pipeline Times: {pipeline_times}')
print(f'Average Seq Time: {sum(seq_times) / len(seq_times)}')
print(f'Average Pipeline Time: {sum(pipeline_times) / len(pipeline_times)}')
print(f'Standard Deviation of Seq Times: {np.std(seq_times)}')
print(f'Standard Deviation of Pipeline Times: {np.std(pipeline_times)}')
print(f'Speedups: {speedups}')
print(f'Average Speedup: {sum(speedups) / len(speedups)}')
print(f'Standard Deviation of Speedups: {np.std(speedups)}')
