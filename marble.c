#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curses.h>
#include <term.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* Author: Alexander Klein
 * Date: 10/6/2021
 * Assignment: Marble
 */

//Declares global variables.
int runningP = 1;
int runningC = 1;

//Child signal handler.
void sigHandlerC(int n) {
	if (n == SIGUSR1) {
		runningC = 0;
	}
}

//Parent signal handler.
void sigHandlerP(int n) {
	if (n == SIGINT) {
		runningP = 0;
	}
}

//Move cursor function.
void move_cursor(int col, int row) {
    char *cap = tigetstr("cup");
    char *cmd = tparm(cap, row, col);
    putp(cmd);
    fflush(stdout);
}

//Main function.
int main(int argc, char *argv[]) {
	//Delcares base variables.
	int rows, cols, velX, velY;

	//Gets values for base variables based on arguments
	//passed into program.
	if (argc ==  5) {
		rows = atoi(strtok(argv[1], " "));
		cols = atoi(strtok(argv[2], " "));
		velX = atoi(strtok(argv[3], " "));
		velY = atoi(strtok(argv[4], " "));
		if (rows < 1 || cols < 1 || velX > 2 || velX < -2 || velY > 2 || velY < -2) {
			printf("Invalid arguments. Exiting.\n");
			return 1;
		}
		if (velX == 0 && velY == 0) {
			printf("Velocity must be greater than 0. Exiting\n");
			return 1;
		}
	} else if (argc == 3) {
		rows = atoi(strtok(argv[1], " "));
		cols = atoi(strtok(argv[2], " "));
		do {
			velX = ((rand() % 5) - 2);
			velY = ((rand() % 5) - 2);
		} while (velX == 0 && velY == 0);
		if (rows < 1 || cols < 1) {
            printf("Invalid arguments. Exiting.\n");
            return 1;
        }
	} else {
		rows = 1;
		cols = 2;
		do {
			velX = ((rand() % 5) - 2);
			velY = ((rand() % 5) - 2);
		} while (velX == 0 && velY == 0);
	}
	
	//Sets up the terminal
	setupterm(NULL, fileno(stdout), NULL);
	int tRows = tigetnum("lines");
	int tCols = tigetnum("cols");
	putp(tigetstr("clear"));

	//Calculates cell size.
	int cWidth = (tCols / cols) - 1;
	int cHeight = (tRows / rows) - 2;
	
	//Enters Cursor Addressing mode.
	putp(tigetstr("smcup"));
	fflush(stdout);

	//Generates the pipes.
	int pvcc[rows][cols][2];
	int pvcp[rows][cols][2];

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j ++) {
			pipe(pvcc[i][j]);
			pipe(pvcp[i][j]);
		}
	}

	//Generates the children.
	pid_t childrenIds[rows][cols];
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if ((childrenIds[i][j] = fork()) < 0) {
				printf("Failed to fork.\n");
				return 1;
			} else if (childrenIds[i][j] == 0) {
				//Child process.
				
				//Declares signal handlers.
				struct sigaction my_act;
				my_act.sa_handler = sigHandlerC;
				sigemptyset(&my_act.sa_mask);
				sigaction(SIGUSR1, &my_act, NULL);

				signal(SIGINT, SIG_IGN);

				//Declares important variables.
				int passes = 0;
				int currX, currY, velX, velY;
							
				while (runningC) {
					//Read cell data from parent.
					read(pvcp[i][j][0], &currX, sizeof(currX));
					if (errno == EINTR) {
						break;
					}
					read(pvcp[i][j][0], &currY, sizeof(currY));
					if (errno == EINTR) {
                        break;
                    }
					read(pvcp[i][j][0], &velX, sizeof(velX));
					if (errno == EINTR) {
                        break;
                    }
					read(pvcp[i][j][0], &velY, sizeof(velY));
					if (errno == EINTR) {
                        break;
                    }
	
					//Update passes.
					passes++;

					//Print grid.
					for (int k = 0; k < rows; k++) {
        				for (int l = 0; l < cols; l++) {
           					for (int m = 1; m < cHeight; m++) {
           				    	if ((m % 2 == 0) || (m == cHeight - 1) || (k == i && l == j)) {
               				    	move_cursor((l * cWidth + l), (k * cHeight + m + k));
               				    	printf("|");
									fflush(stdout);
               				    	move_cursor((l * cWidth + l + cWidth), (k * cHeight + m + k));
               				    	printf("|");
									fflush(stdout);
           						} else {
									move_cursor((l * cWidth + l), (k * cHeight + m + k));
           	                        printf(" ");
           	                        fflush(stdout);
           	                        move_cursor((l * cWidth + l + cWidth), (k * cHeight + m + k));
           	                        printf(" ");
           	                        fflush(stdout);
								}
        				    }
           					for (int m = 0; m < cWidth; m++) {
           		 				if ((m % 2 == 0) || (m == cWidth - 1) || (k == i && l == j)) {
                 	 				move_cursor((l * cWidth + m + l), (k * cHeight + k));
                    				printf("_");
									fflush(stdout);
                    				move_cursor((l * cWidth + m + l), (k * cHeight + k + cHeight));
                    				printf("_");
									fflush(stdout);
                				} else {
									move_cursor((l * cWidth + m + l), (k * cHeight + k));
                               	    printf(" ");
                               	    fflush(stdout);
                               	    move_cursor((l * cWidth + m + l), (k * cHeight + k + cHeight));
                               	    printf(" ");
                               	    fflush(stdout);
								}
            				}
        				}
    				}
					//Prints the number of passes.
                	move_cursor((j * cWidth + 1 + j), (i * cHeight + 1 + i));
                	printf("%d", passes);
                	fflush(stdout);
					
					//While cursor is in cell.
					int inCellX = 1;
					int inCellY = 1;
					while (inCellX && inCellY && runningC) {
						//Prints the cursor. (Bounces off air bug)
						move_cursor((j * cWidth + currX + j), (i * cHeight + currY + i));
						//Moves the cursor to new spot and check if out of bounds.
						if (currX + velX >= cWidth + 1) {
							if (j != cols - 1) {
								inCellX = 0;
							} else {
								velX *= -1;
							}
						} else if (currX + velX < 0) {
							if (j != 0) {
								inCellX = 0;
							} else {
								velX *= -1;
							}
						} else {
							currX += velX;
						}

						if (currY + velY >= cHeight + 1) {
							if (i != rows - 1) {
								inCellY = 0;
							} else {
								velY *= -1;
							}
						} else if (currY + velY < 0) {
							if (i != 0) {
								inCellY = 0;
							} else {
								velY *= -1;
							}
						} else {
							currY += velY;
						}
						usleep(30000);
					}
						
					//Sends data back to parent.
					write(pvcc[i][j][1], &inCellX, sizeof(inCellX));
					if (errno == EINTR) {
                        break;
                    }
   	            	write(pvcc[i][j][1], &inCellY, sizeof(inCellY));
					if (errno == EINTR) {
                        break;
                    }
					write(pvcc[i][j][1], &currX, sizeof(currX));
					if (errno == EINTR) {
                        break;
                    }
					write(pvcc[i][j][1], &currY, sizeof(currY));
					if (errno == EINTR) {
                        break;
                    }
					write(pvcc[i][j][1], &velX, sizeof(velX));
					if (errno == EINTR) {
                        break;
                    }
					write(pvcc[i][j][1], &velY, sizeof(velY));
					if (errno == EINTR) {
                        break;
                    }
				}
				
				//Kills the child processes.
   				 for (int m = 0; m < rows; m++) {
        			for (int n = 0; n < cols; n++) {
   		       			//Closes the parent pipe.
            			close(pvcp[m][n][1]);
            			close(pvcp[m][n][0]);
            			close(pvcc[m][n][1]);
            			close(pvcc[m][n][0]);
        			}
    			}
				
				return 0;
			}
		}
	}

	//Rest of parent process.
	
	//Delcares signal handler.
	struct sigaction my_act;
	my_act.sa_handler = sigHandlerP;
	my_act.sa_flags = SA_RESTART;
	sigemptyset(&my_act.sa_mask);
	sigaction(SIGINT, &my_act, NULL);

	//Generates the initial position in the cell and on the terminal.
    int currCellR = rand() % rows;
	int currCellC = rand() % cols;
    int currX = rand() % cWidth;
    int currY = rand() % cHeight;
	int borderCrosses = 0;
	
	//Main parent loop.
	while (runningP) {
		//Prints what child the cursor is in.
        move_cursor(0, tRows);
        printf("Marble is in child: %d   Border Crosses: %d", childrenIds[currCellR][currCellC], borderCrosses);
		fflush(stdout);

		//Writes data to cell.
		write(pvcp[currCellR][currCellC][1], &currX, sizeof(currX));
		if (errno == EINTR) {
            break;
        }
		write(pvcp[currCellR][currCellC][1], &currY, sizeof(currY));
		if (errno == EINTR) {
            break;
        }
		write(pvcp[currCellR][currCellC][1], &velX, sizeof(velX));
		if (errno == EINTR) {
            break;
        }
		write(pvcp[currCellR][currCellC][1], &velY, sizeof(velY));
		if (errno == EINTR) {
            break;
        }
				

		int inCellX, inCellY;

		//Reads data from cell if it has to be passed to a new one.
		read(pvcc[currCellR][currCellC][0], &inCellX, sizeof(inCellX));
		if (errno == EINTR) {
        	break;
        }
   		read(pvcc[currCellR][currCellC][0], &inCellY, sizeof(inCellY));
		if (errno == EINTR) {
            break;
        }
		read(pvcc[currCellR][currCellC][0], &currX, sizeof(currX));
		if (errno == EINTR) {
            break;
        }
		read(pvcc[currCellR][currCellC][0], &currY, sizeof(currY));
		if (errno == EINTR) {
            break;
        }
		read(pvcc[currCellR][currCellC][0], &velX, sizeof(velX));
		if (errno == EINTR) {
            break;
        }
		read(pvcc[currCellR][currCellC][0], &velY, sizeof(velY));
		if (errno == EINTR) {
            break;
        }

	
		//Determines new cell to send it to.
		if (inCellX == 0) {
			if (currX + velX >= cWidth) {
    			currCellC++;
				currX = cWidth - currX;
        	} else if (currX + velX < 0) {
        		currCellC--;
				currX = cWidth - currX;
			}
		}
		if (inCellY == 0) {
        	if (currY + velY >= cHeight) {
        		currCellR++;
				currY = cHeight - currY;
        	} else if (currY + velY < 0) {
        		currCellR--;
				currY = cHeight - currY;
        	}
		}
		
		//Increments number of times marble crossed a border.
		borderCrosses++;

		//Terminates program if child crossed 10 borders.
		if (borderCrosses < 0) {
			runningP = 0;
		}
	}

	//Kills the child processes.
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			//Terminates child.
			int status;
			kill(childrenIds[i][j], SIGUSR1);
			wait(&status);

			//Prints which child was terminated.
            move_cursor((j * cWidth + 1 + j), (i * cHeight + 1 + i));
            printf("%d terminated.", childrenIds[i][j]);
            fflush(stdout);

			//Closes the pipes on the parent's side.
			close(pvcp[i][j][1]);
			close(pvcp[i][j][0]);
			close(pvcc[i][j][1]);
            close(pvcc[i][j][0]);
		}
	}

	//Moves the cursor to the bottom of the screen so the
	//terminal can read the next user entered command.
	move_cursor(0, tRows);
	printf("");
	fflush(stdout);
	
	return 0;
}
