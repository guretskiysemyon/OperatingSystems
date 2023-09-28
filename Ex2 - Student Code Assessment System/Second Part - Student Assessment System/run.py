import subprocess
import os

# Step 1: Run 'make' in the directory
make_directory = "./"  # Replace with the path to your directory
make_command = "make"

try:
    subprocess.check_call(make_command, cwd=make_directory, shell=True)
except subprocess.CalledProcessError:
    print("Error while running 'make'")
    exit(1)

# Step 2: Run 'a.out' with 'conf.txt' as an argument
executable = os.path.join(make_directory, "a.out")
conf_file = os.path.join(make_directory, "conf.txt")
a_out_command = f"{executable} {conf_file}"

try:
    subprocess.check_call(a_out_command, shell=True)
except subprocess.CalledProcessError:
    print("Error while running 'a.out'")
    exit(1)

# Step 3: Delete 'a.out' and 'comp.out'
try:
    os.remove(executable)
    os.remove(os.path.join(make_directory, "comp.out"))
except OSError:
    print("Error while deleting 'a.out' or 'comp.out'")
    exit(1)

print("Program completed successfully.")
