import subprocess
import sys

def check_machine_status(machine):
    # Ping the machine once to check if it's up
    result = subprocess.run(["ping", "-c", "1", machine], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.returncode == 0

def get_up_machines(machines, n):
    up_machines = []
    for machine in machines:
        if check_machine_status(machine):
            up_machines.append(machine)
            if len(up_machines) == n:
                break
    return up_machines

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python find_up_machines.py <number of machines>")
        sys.exit(1)
    start_index = 1
    end_index = 99
    prefix = "pc7-"
    suffix = "-l"
    n = int(sys.argv[1])
    
    machines = [f"{prefix}{str(i).zfill(3)}{suffix}" for i in range(start_index, end_index+1)]
    
    up_machines = get_up_machines(machines, n)
    print(f"{len(up_machines)} machines are up: {', '.join(up_machines)}")

    # Print in this final format 
    # pc7-003-l:1,pc7-005-l:1,pc7-007-l:1,pc7-009-l:1,pc7-011-l:1,pc7-015-l:1,pc7-017-l:1,pc7-019-l:1,pc7-020-l:1,pc7-022-l:1,pc7-023-l:1

    print(",".join([f"{machine}:1" for machine in up_machines]))