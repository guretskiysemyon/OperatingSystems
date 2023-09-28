import os
import subprocess



c_file = "CompareFiles.c"
executable_name = "comp.out"

# Compile the C source file
compilation_command = ["gcc", c_file, "-o", executable_name]
subprocess.run(compilation_command, check=True)

# Get the absolute path of the executable
program_path = os.path.abspath(executable_name)



def compare_folder_case(folder_path):
    print(folder_path)
    os.chdir(folder_path)

    # Loop through all subdirectories in the current directory
    for subdir in os.listdir():
        if os.path.isdir(subdir):
            # Get the path to the two txt files in the current subdirectory
            txt_files = [os.path.join(subdir, file) for file in os.listdir(subdir) if file.endswith('.txt')]

            # Run the program with the two txt files as arguments
            command = [program_path] + txt_files
            command = [program_path, txt_files[0], txt_files[1]]

            return_value = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            # Print the return value
            print(f"Program returned {return_value.returncode}")
    
    # Return to parent directory
    os.chdir(os.path.pardir)


# Itterate through all folder and run for every case test
os.chdir("./textComparison")
for subdir in os.listdir():
    compare_folder_case(subdir)