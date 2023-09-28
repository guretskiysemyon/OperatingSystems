#### Exersice explanation:
Write a program that takes paths to two files as arguments to main, and checks if the two files are identical/similar/different. For clarity, the paths include the file names.

If the files are identical (contain exactly the same content), the program returns 1, if the files are similar (explanation below) the program returns 3, otherwise if the files are different it returns 2 (pay attention, this is not a program that prints to the screen).

Identical files are files where all the characters in them are equal: for example (Hello World and Hello World).

Similar files are files that are not identical but contain the same text and there is a difference in use of upper/lower case letters, spaces or newline characters.

Different files are files that are neither identical nor similar.


 **Note:**
Files may contain upper case letters (or lower case), numbers, spaces or newline characters.
You can use strcmp even though in the comments at the end of the exercise it was stated not to use library functions that can be implemented using tools we learned in the tutorials.
When compiling the program, name it comp.out instead of a.out.


Running the program:

1. You can compile by yourself and run it as you want.
2. You can run the python_test.py script which will compile it for you, and will run a test on all cases from the textComparison folder.
