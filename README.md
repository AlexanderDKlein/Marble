# Marble
This project was assigned to me in my Operating Systems Concepts class. Its purpose was to demonstrate my knowledge of multiprocessing and inter-process communication via pipes. When run, it will turn the terminal into a number of boxes, where each box is a child process under the control of a background parent process. The marble will be transferred to each box by the parent as it passes into them, and the child process containing it will be in charge of its movement.
## How to Run:
- Compile marble.c into a runnable program.
- Run the executable in a linux terminal using "./(executable) x y a b", where:
&emsp;  
* x is number of rows.
&emsp;  
* y is number of columns.
&emsp;  
* a is the marble's velocity along the x axis (0-2).
&emsp;  
* b is the marble's velocity along the y axis (0-2).
- Use control + c to terminate the program.
