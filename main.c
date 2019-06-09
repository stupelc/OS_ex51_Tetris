//chagit stupel 209089960

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <wait.h>

#define WRITE_ERROR write(fileno(stderr), "Error in system call\n", strlen("Error in system call\n"));
#define MAX_SIZE 150


/**
 *
 * @return the char that the user typed
 */
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}


//listener to the keyboard
int main() {
    //create pipe
    int fd[2];
    if (pipe(fd) < 0) {
        WRITE_ERROR
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        WRITE_ERROR
    } else {
        if (pid > 0) { // **father** writing to the pipe the user input

            // close the read fd (very important)
            if (close(fd[0]) == -1) {
                WRITE_ERROR
            }

            // read key from the user
            char user_input = getch();

            //read and send the input char
            while (user_input != 'q') {
                //not a correct input
                if (!(user_input == 'a' || user_input == 's' || user_input == 'd' || user_input == 'w')) {
                    user_input = getch();
                    continue;
                }
                //if a correct input write it to the pipe
                if (write(fd[1], &user_input, sizeof(user_input)) == -1)
                    WRITE_ERROR
                if (kill(pid, SIGUSR2) == -1)
                    WRITE_ERROR

                user_input = getch();
            }

            //if the user press 'q' - quit

            if (write(fd[1], &user_input, sizeof(user_input)) == -1)
                WRITE_ERROR

            // sends SIGUSR2 signal to child process
            kill(pid, SIGUSR2);
            // wait for child to finish
            waitpid(pid, NULL, 0);


            //todo need or not??
            // close the write fd (very important)
            /*if (close(fd[1]) == -1) {
                WRITE_ERROR
            }*/

        } else { // **child** reading from pipe

            // close the write fd
            if (close(fd[1]) == -1) {
                WRITE_ERROR
            }
            char current_path[MAX_SIZE] = {0};

            // get the current path and create a path to the draw.out
            if (getcwd(current_path, sizeof(current_path)) != NULL) {
                strcat(current_path, "/draw.out");
            } else {
                WRITE_ERROR;
            }

            //set the args
            char *arguments[2] = {"draw.out", NULL};

            //take the input from the first place in the pipe
            if (dup2(fd[0], STDIN_FILENO) == -1) {
                WRITE_ERROR;
            }
            //get the input from the compile file ex52
            execvp(current_path, arguments);
            WRITE_ERROR;

            //close the read fd
            if (close(fd[0]) == -1) {
                WRITE_ERROR
            }
        }
    }
    return 0;
}