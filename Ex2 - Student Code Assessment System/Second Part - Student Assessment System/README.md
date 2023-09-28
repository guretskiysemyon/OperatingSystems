### Exersice exmplanation:
Write a program that takes a path to a configuration file as an argument to main. You can assume the file exists and is not a directory.

The configuration file contains 3 lines:

Line 1: Path to a directory containing subdirectories at the first level, each subdirectory represents a user and should contain a c file.

Line 2: Path to a file containing input.

Line 3: Path to a file containing the correct output for the input file in line 2.

The configuration file ends with a newline character.

Your program should enter all the subdirectories and ignore other files (and directories) that are not subdirectories in the directory in line 1 (and at deeper levels), search in each of its subdirectories for a c file (there should be at most one c file per directory, there may not be a c file at all, and there can also be files and directories of other types except for files with .out suffix), compile it and run the resulting executable with the input that appears in the file in the path in line 2 (the program it will run will take input from stdin and print to stdout so you need to use i/o redirection).

The output of the program should be compared to the expected output file whose path comes from line 3, using the comp.out program you implemented in part A of the exercise (run the comp.out program and give it as arguments to main the path of the correct output found in the configuration file in line 3, and the output of the user's program you ran).

Your program should create a file in the directory it was run from called results.csv containing for each user name, subdirectory name, grade between 0-100 according to the return value from comp.out, and reason. The "," character should be written between the user name, grade, and reason (without spaces).

**Possible reasons**:
- NO_C_FILE - There is no file with .c suffix in the user's directory. Grade given will be 0.
- COMPILATION_ERROR – Compilation error (file does not compile). Grade given will be 10.
- TIMEOUT – The compiled c file ran for more than 5 seconds. Grade given will be 20.
- WRONG – Output is different than expected output. Grade given will be 50.
- SIMILAR – Output is different than expected output but similar. Grade given will be 75.
- EXCELLENT – Output matches expected output. Grade given will be 100.


Example contents of results.csv file:
| Student  | Grade | Explanation         |
|----------|-------|---------------------|
| Monica   | 100   | EXCELLENT           |
| Phoebe   | 0     | NO_C_FILE           |
| Rachel   | 20    | TIMEOUT             |
| Ross     | 10    | COMPILATION_ERROR   |
| Joey     | 50    | WRONG               |


### Running the program:

##### First option:

Run make in the terminal
Run ./a.out conf.txt
After this you will see the errors.txt file with errors, results.csv with grades of students from the "students" folder, and the comp.out and a.out files.

##### Second option:

Run the "run.py" Python file.
You will see the errors.txt file with errors, results.csv with grades of students from the "students" folder. The a.out and comp.out files will be deleted.
