/*
 * File: GraduateStudents.c
 * Author: Semyon Guretskiy
 * Date: September 25, 2023
 * Description:
 *  This program receives a configuration file with input, students' directory, and correct output paths.
 *  For every student, it finds a .c file, compiles, executes, compares outputs, and assigns grades.
 *  At the end of the program, two files will remain in the directory:
 *    - results.csv: containing grades of students
 *    - errors.txt: containing all encountered errors.
 */


#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>  //for open
#include <unistd.h> // for close
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define GEN_ERROR -1
#define SAME 1
#define DIFF 2
#define SIMILAR 3
#define COMPL_ERROR 4
#define NO_C_FILE 5
#define TIME_OUT 6
#define MAX_LEN 151


/*
 * is_C_file - Check if the given path represents a C source file by examining its extension.
 * 
 * Parameters:
 *   char* name - Path to the file.
 * 
 * Returns:
 *   1 - If it's a C source file (ends with ".c").
 *   0 - Otherwise.
 */
int is_C_file(char *name)
{
    int n = strlen(name);
    if (name[n - 2] == '.' && name[n - 1] == 'c')
        return 1;
    return 0;
}

/*
 * path_executed - Generate the path for the executable file
 * by replacing the file extension with ".out".
 *
 * Parameters:
 *   char* path - Original path to the file.
 *   char* path_exe - Path to the executable file.
 */
void path_executed(char *path, char *path_exe)
{
    strcpy(path_exe, path);
    int n = strlen(path);
    path_exe[n - 1] = 'o';
    path_exe[n] = 'u';
    path_exe[n + 1] = 't';
    path_exe[n + 2] = '\0';
}

/*
 * add_to_path - Given a base path and a new relative path, concatenate them and save the result.
 * 
 * Parameters:
 *   char* result - Variable to store the resulting path.
 *   const char* path_b - The base part of the path.
 *   char* path_f - The new part of the path to be added.
 */
void add_to_path(char *result, const char *path_b, char *path_f)
{
    strcpy(result, path_b);
    strcat(result, "/");
    strcat(result, path_f);
}

/*
 * open_input_output - Open "output.txt" for output, the input file for input, and perform I/O redirection.
 * Redirects STD_OUT to "output.txt," STD_IN to the input file, and STD_ERROR to the provided error file descriptor.
 * 
 * Parameters:
 *   int* ios - Array to store the previous STD_IN, STD_OUT, and STD_ERROR file descriptors.
 *   char* input - Path to the input file.
 *   int errors - File descriptor of the "errors.txt" file.
 */
void open_input_output(int *ios, char *input, int errors)
{
    /// Try to open "output.txt" with create, read, and write permissions.
    int fd_output = open("output.txt", O_WRONLY | O_RDONLY | O_CREAT, 0644);
    if (fd_output < 0)
    {
        perror("Error in: open");
        exit(GEN_ERROR);
    }
    // Try to open the input file with read-only permission.
    int fd_input = open(input, O_RDONLY);
    if (fd_input < 0)
    {
        perror("Error in: open");
        exit(GEN_ERROR);
    }

    // Perform I/O redirection: Redirect errors to the provided error file descriptor.
    int fd_err = dup2(errors, STDERR_FILENO);
    if (fd_err < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }

    // Perform I/O redirection: Redirect output to "output.txt."
    int fd_out = dup2(fd_output, STDOUT_FILENO);
    if (fd_out < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }
    close(fd_output);

    // Perform I/O redirection: Redirect input to the input file.
    int fd_in = dup2(fd_input, STDIN_FILENO);
    if (fd_in < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }
    close(fd_input);

    // Save the previous file descriptors in the ios array.
    ios[0] = fd_out;
    ios[1] = fd_in;
    ios[2] = fd_err;
}

/*
 * delete_file - Delete the specified file. Exits if unlink fails.
 * 
 * Parameters:
 *   char* filename - Path to the file to delete.
 */
void delete_file(char *filename)
{

    if (unlink(filename) != 0)
    {
        perror("Error in: unlink");
        exit(GEN_ERROR);
    }
}

void alarm_handler(int c) {}

/*
 * execute_file - Create a new process to execute the specified file with a timeout of 5 seconds.
 * Exits if system calls fail.
 * 
 * Parameters:
 *   char* path - Path to the executable file.
 * 
 * Returns:
 *   0 - Success.
 *   TIME_OUT - If the executable doesn't finish execution within 5 seconds.
 *   GEN_ERROR - General error.
 */
int execute_file(char *path)
{
    // Create a new process to execute the file.
    pid_t pid;
    int status;
    pid = fork();

    if (pid == 0)
    {
        // Create an alarm in 5 seconds to prevent infinite execution.
        signal(SIGALRM, alarm_handler);
        alarm(5);
        execlp(path, path, NULL);
        perror("Error in: execlp");
        exit(GEN_ERROR);
    }
    else if (pid < 0)
    {
        perror("Error in: fork");
    }
    else
    {   
        // Wait for the child process.
        if (waitpid(pid, &status, 0) == GEN_ERROR)
        {
            perror("Error in: waitpid");
            exit(GEN_ERROR);
        }
        // If there was a timeout.
        if (WIFSIGNALED(status))
        {
            return TIME_OUT;
        }
        // Check the return status.
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
            {
                return GEN_ERROR;
            }
            return 0;
        }
    }
}

/*
 * compile_file - Compile the file located at the given path and generate the executable at the specified path.
 * Exits if system calls fail.
 * 
 * Parameters:
 *   char* path - Path to the source file to be compiled.
 *   char* path_exe - Path to the generated executable file.
 * 
 * Returns:
 *   0 - Success.
 *   COMPL_ERROR - Compilation error.
 */
int compile_file(char *path, char *path_exe)
{   
    // Create a new process to compile the file.
    pid_t pid;
    int status;
    pid = fork();

    // Child process calls exec with gcc.
    if (pid == 0)
    {
        execlp("gcc", "gcc", path, "-o", path_exe, NULL);
        perror("Error in: execlp");
        exit(GEN_ERROR);
    }
    else if (pid < 0)
    {
        perror("Error in: fork");
    }
    else
    {   
        // Parent process waits for the child to finish.
        if (waitpid(pid, &status, 0) == GEN_ERROR)
        {
            perror("Error in: waitpid");
            exit(GEN_ERROR);
        }
        // Check the status returned by the child process.
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0) // If status isn't 0, then it's a compilation error.
                return COMPL_ERROR;

            return 0;
        }
    }
}

/*
 * check_output - Run the comp.out executable to compare the content of the current "output.txt" file
 * with the content of the specified "correct_output" file. Exits with -1 when an error occurs.
 * 
 * Parameters:
 *   char* correct_output - Path to the correct_output file.
 * 
 * Returns:
 *   0 - If the comparison was successful.
 *   GEN_ERROR - General error.
 */
int check_output(char *correct_output)
{

    // Create a new process to run the command.
    pid_t pid;
    int status;
    pid = fork();

    if (pid == 0)
    {
        // Run the comp.out executable to compare "output.txt" and "correct_output.txt".
        execlp("./comp.out", "./comp.out", "./output.txt", correct_output, NULL);
        perror("Error in: execlp");
        exit(GEN_ERROR);
    }
    else if (pid < 0)
    {
        perror("Error in: fork");
    }
    else
    {   
        // Wait for the child process.
        if (waitpid(pid, &status, 0) == GEN_ERROR)
        {
            perror("Error in: waitpid");
            exit(GEN_ERROR);
        }
        // Check the exit status of the child.
        if (WIFEXITED(status))
        {
            int result = WEXITSTATUS(status);
            if (result == 255 || result == GEN_ERROR) // Indicates an error.
            {
                return GEN_ERROR;
            }
            return result;
        }
    }
}

/*
 * return_ios - Restore standard file descriptors (stdin, stdout, stderr) to their original values.
 * 
 * Parameters:
 *   int in - Original stdin file descriptor.
 *   int out - Original stdout file descriptor.
 *   int err - Original stderr file descriptor.
 */
void return_ios(int in, int out, int err)
{
    if (dup2(out, STDOUT_FILENO) < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }
    if (dup2(in, STDOUT_FILENO) < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }
    if (dup2(err, STDOUT_FILENO) < 0)
    {
        perror("Error in: dup2");
        exit(GEN_ERROR);
    }

}



/*
 * handle_student - Find a C file in the student's directory, compile it, execute it, and compare
 * the output with the given correct output.
 * 
 * Parameters:
 *   const char* path - Path to the student's directory.
 *   int errors - File descriptor of errors.txt.
 *   char* input - Path to the input file.
 *   char* output - Path to the output file.
 * 
 * Return Values:
 *   GEN_ERROR - Error in system calls.
 *   SAME - Output is correct.
 *   DIFF - Output is different.
 *   SIMILAR - Output is similar.
 *   COMPL_ERROR - Compile error.
 *   NO_C_FILE - No C file found in the directory.
 *   TIME_OUT - Timeout.
 */
int handle_student(const char *path, int errors, char *input, char *output)
{

    DIR *dir;
    struct dirent *entry;

    // Try to open the directory
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Error in: opendir");
        return GEN_ERROR;
    }

    // Open output.txt and perform I/O redirections for input, output, and errors.
    int count = 0;
    int result = 0;
    int fd_ios[3];
    open_input_output((int *)&fd_ios, input, errors);

    // Read directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        // Check if the entry is a regular file with a .c extension
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".c") != NULL)
        {
            // Check if the current entry is a C file.
            if (is_C_file(entry->d_name) == 1)
            {
                count = 1;
                // Create an absolute path to the C file and the future executable file.
                char path_to_file[MAX_LEN];
                add_to_path(path_to_file, path, entry->d_name);
                char path_exe[MAX_LEN];
                path_executed(path_to_file, path_exe);

                // Compile the file
                result = compile_file(path_to_file, path_exe);
                if (result != 0)
                    break;

                // Execute the file
                result = execute_file(path_exe);
                if (result != 0)
                    break;
                ;

                // Delete the executable file
                delete_file(path_exe);

                // Check the output with the correct output
                result = check_output(output);
                break;
            }
        }
    }

    // Return file descriptors to their original states before I/O redirection.
    return_ios(fd_ios[1], fd_ios[0], fd_ios[2]);

    // Close the directory
    if (closedir(dir) < 0)
    {
        perror("Error in: closedir");
        exit(GEN_ERROR);
    }
    
    // If count is 0, then there is no C file in the directory.
    if (!count)
        return NO_C_FILE;

    return result;
}

/*
 * graduate_student - Write the student's name, grade, and explanation for the grade
 * in results.csv.
 * 
 * Parameters:
 *   int code - The code for student graduation, indicating the error or result of comparing outputs.
 *   char* name - Student's name.
 *   int fd_results - File descriptor of results.csv.
 */
int graduate_student(int code, char *name, int fd_results)
{
    char line[MAX_LEN];
    strcpy(line, name);
    strcat(line, ",");

    switch (code)
    {
    case 1:
        strcat(line, "100,EXCELLENT\n");
        break;
    case 2:
        strcat(line, "50,WRONG\n");
        break;
    case 3:
        strcat(line, "75,SIMILAR\n");
        break;
    case 4:
        strcat(line, "10,COMPILATION_ERROR\n");
        break;
    case 5:
        strcat(line, "0,NO_C_FILE\n");
        break;
    case 6:
        strcat(line, "20,TIMEOUT\n");
        break;
    default:
        break;
    }

    if (write(fd_results, line, strlen(line)) == GEN_ERROR)
    {
        perror("Error in: write");
        close(fd_results);
        exit(GEN_ERROR);
    }
}


/*
 * open_files - Attempt to open the "results.csv" file to output results
 * and the "errors.txt" file to save errors.
 * If the `open` function calls fail, the program exits with an error code.
 * 
 * Parameters:
 *   int* results - Pointer to a variable to store the file descriptor of "results.csv".
 *   int* errors - Pointer to a variable to store the file descriptor of "errors.txt".
 */
void open_files(int *results, int *errors)
{   

    /// Try to open "results.csv" with create, write-append permissions.
    int fd_results = open("results.csv", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    if (fd_results < 0)
    {
        perror("Error in: open");
        exit(GEN_ERROR);
    }
    // Try to open "errors.txt" with create, read, and write permissions.
    int fd_error = open("errors.txt", O_WRONLY | O_RDONLY | O_CREAT, 0644);
    if (fd_error < 0)
    {
        perror("Error in: open");
        exit(GEN_ERROR);
    }
    *results = fd_results;
    *errors = fd_error;
}


/*
 * handle_students - Iterate through the students' directory, handle each student, and graduate them.
 * 
 * Parameters:
 *   char conf[][] - Configuration data file.
 */
int handle_students(char conf[][MAX_LEN])
{

    
    DIR *pDir;
    struct dirent *pDirent;
    int response = 0;

    // Open necessary files.
    int results;
    int errors;
    open_files(&results, &errors);

    // Try to open the directory with students.
    if ((pDir = opendir(conf[0])) == NULL)
        exit(GEN_ERROR);

    // Iterate through nodes in the students' directory.
    while ((pDirent = readdir(pDir)) != NULL)
    {   
        // If the current node is a directory and not ".", ".."
        if (pDirent->d_type == DT_DIR && strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0)
        {
            // Add the new directory to the path.
            char path_to_file[151];
            add_to_path(path_to_file, conf[0], pDirent->d_name);

            response = handle_student(path_to_file, errors, conf[1], conf[2]);

            delete_file("output.txt");
            
            // Write the results of the student to results.csv.
            graduate_student(response, pDirent->d_name, results);
        }
    }

    if (closedir(pDir) < 0)
        exit(GEN_ERROR);
    return response;
}



/*
 * read_conf - Read data from a configuration file and store it in a given array.
 *
 * Parameters:
 *   const char* file_name - Path to the configuration file.
 *   char conf[][MAX_LEN] - Array to store data from the file.
 */
void read_conf(const char *file_name, char conf[][MAX_LEN])
{
    // Try to open the file.
    int fd_conf = open(file_name, O_RDONLY);
    if (fd_conf < 0)
    {
        perror("Error in: open");
        exit(GEN_ERROR);
    }

    // Start reading the file.
    int x;
    int i = 0, j = 0;
    char buff;
    while (1)
    {   
        // Try to read the next character.
        x = read(fd_conf, &conf[i][j], 1);
        if (x < 0)
        {
            perror("Error in: read");
            break;
        }

        // If reached the end of the file, then stop.
        if (x == 0)
            break;

        // If reached the end of a line, then move to the next line in conf[][].
        if (conf[i][j] == '\n'){
            conf[i][j] = '\0';  // Null-terminate the string.
            i++;
            j = 0;
        }
        else{
            j++;
        }
    }

    // Try to close the file.
    if (close(fd_conf) < 0){
        perror("Error in: close");
        exit(GEN_ERROR);
    }
}

/* 
 * try_to_open_conf - Attempt to access all the files specified
 * in the configuration array.
 * 
 * Parameters:
 *   char conf[][MAX_LEN] - Array containing file paths in each line.
 * 
 * Returns:
 *   -1 (GEN_ERROR) - If an error occurs.
 *    0 - Otherwise.
 */
int try_to_open_conf(char conf[][MAX_LEN])
{
    // Attempt to access each file.
    if (access(conf[0], F_OK) == GEN_ERROR)
    {
        printf("Not a valid directory\n");
        return GEN_ERROR;
    }
    if (access(conf[1], F_OK) == GEN_ERROR)
    {
        printf("Input file not exist\n");
        return GEN_ERROR;
    }
    if (access(conf[2], F_OK) == GEN_ERROR)
    {
        printf("Output file not exist\n");
        return GEN_ERROR;
    }

    return 0;
}


int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        return GEN_ERROR;
    }

    char conf[3][MAX_LEN];
    read_conf(argv[1], conf);
    if (try_to_open_conf(conf))
        return GEN_ERROR;

    handle_students(conf);
}
