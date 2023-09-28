/*
 * File: CompareFiles.c
 * Author: Semyon Guretskiy
 * Date: September 25, 2023
 * Description:
    Given a path to 2 file the program will compare them by next rules.
    - If the files are identical, the program returns 1.
        Identical files are files where all the characters in them are equal.
        For example (Hello World and Hello World).

    
    - If the files are similar the program returns 3.
      Similar files are files that are not identical but contain the same text 
      and there is a difference in use of upper/lower case letters, spaces or newline characters.
    
    
    - Otherwise if the files are different it returns 2.
      Different files are files that are neither identical nor similar.
*/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h> //for open
#include <unistd.h> // for close
#include <ctype.h>


// buffer for two files and number of bytes have been read for two files.
char buffer_one;
char buffer_two;
int x_one;
int x_two;

/*
 * is_whitespace - Check if the given character is a whitespace character.
 *
 * Parameters:
 *   c - The character to be checked.
 *
 * Returns:
 *   1 - If 'c' is a whitespace character (space, tab, newline, vertical tab,
 *       form feed, or carriage return).
 *   0 - Otherwise.
 */
int is_whitespace(char c){
    switch (c) {
    case ' ': 
    case '\t':     
    case '\n':     
    case '\v':    
    case '\f':     
    case '\r':      
            return 1;
    default:
        return 0;
    }
}

/*
 * white_till_end - Read from the specified file descriptor until the first
 * non-whitespace character is encountered or until the end of the file.
 *
 * Parameters:
 *   fd - File descriptor number.
 *
 * Returns:
 *   -1 - Error occurred while reading the file.
 *    2 - First non-whitespace character was found before the end of the file.
 *    3 - Reached the end of the file without finding a non-whitespace character.
 */
int white_till_end(int fd) {
    int buff, x = 1;

    while (x != 0) {
        x = read(fd, &buff, 1);
        if (x == -1) {
            printf("Error in reading file\n");
            return -1; // Error in reading file
        }
        if (!is_whitespace(buff)) {
            return 2; // Found first non-whitespace character
        }
    }
    return 3; // Reached the end of the file without finding a non-whitespace character
}





int main(int argc, char const *argv[])
{
    // Check number of arguments
    if (argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }
    
    // Open files
    int fd_one = open(argv[1],O_RDONLY);
    if (fd_one < 0){
        printf("First open failed\n");
        return -1;
    }
    int fd_two = open(argv[2], O_RDONLY);
    if (fd_two < 0){
        printf("Second open failed\n");
        close(fd_one);
        return -1;
    }

    // Initialize the result to 1; it will be changed if differences are found
    int result = 1;
    int flag = 1;


    // First loop to check if two files are identical.

    while(flag){
        // Read a character from the first and second files. Close the files if an error occurs.
        x_one = read(fd_one, &buffer_one, 1);
        if (x_one < 0){
            printf("Error in readin file 1\n");
            close(fd_one);
            close(fd_two);
            return -1;
        }
        x_two = read(fd_two, &buffer_two, 1);
        if (x_two <0){
            printf("Error in readin file 2\n");
            close(fd_one);
            close(fd_two);
            return -1;
        }
        
        /*
        * If x_one and x_two are 0, then we have reached the end of both files, and they are identical.
        * Return 1.
        */
        if (x_one == 0 && x_two == 0){
            x_two = read(fd_two, &buffer_two, 1);
            close(fd_one);
            close(fd_two);
            return 1;
        }

        /*
        * If only one file has reached its end, we want to check the other file that still has characters.
        * If all remaining characters are whitespace, then white_till_end returns 3, and we return 3.
        * Otherwise, return 2.
        */
        if (x_one == 0){
            result = white_till_end(fd_two);
            close(fd_one);
            close(fd_two);
            return result;
        }
        if(x_two == 0) {
            result = white_till_end(fd_one);
            close(fd_one);
            close(fd_two);
            return result;
        }
        
        // If the read characters are not the same, the files are not identical, and we will check for similarity.
        if (buffer_one != buffer_two )
            flag = 0;
        
    }


    flag = 1;
    do{
        // Skip all whitespace characters in the first file.
        if (is_whitespace(buffer_one)){
            while(is_whitespace(buffer_one) && x_one != 0){
                x_one = read(fd_one, &buffer_one, 1);
                if (x_one < 0){
                    printf("Error in readin file 1\n");
                    close(fd_one);
                    close(fd_two);
                    return -1;
                }
            }
        }
        // Skip all whitespace characters in the second file.
        if (is_whitespace(buffer_two)){
            while(is_whitespace(buffer_two) && x_two != 0){
                x_two = read(fd_two, &buffer_two, 1);
                if (x_two < 0){
                    printf("Error in readin file 1\n");
                    close(fd_one);
                    close(fd_two);
                    return -1;
                }
            }
        }
        // If x_one and x_two are 0, then we've reached the end of both files and they are similar.
        if (x_one == 0 && x_two == 0){
            close(fd_one);
            close(fd_two);
            return 3;
        }

        /*
        * If only one of them is 0, then the files are different because we skipped all whitespace characters,
        * and now we know that the second character is not whitespace.
        */
        if (x_one == 0 || x_two == 0){
            close(fd_one);
            close(fd_two);
            return 2;    
        }
        // If characters are different.
        if (tolower(buffer_one) != tolower(buffer_two)){
            close(fd_one);
            close(fd_two);
            return 2;
        } 

        // Read next character in both files.
        x_one = read(fd_one, &buffer_one, 1);
        if (x_one < 0){
            printf("Error in readin file 1\n");
            close(fd_one);
            close(fd_two);
            return -1;
        }
        x_two = read(fd_two, &buffer_two, 1);
        if (x_two <0){
            printf("Error in readin file 2\n");
            close(fd_one);
            close(fd_two);
            return -1;
        }
    }
    while (flag);
    
}



