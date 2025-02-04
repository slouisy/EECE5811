#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const short STR_LEN = 20; /*message string lengths*/
const short MAX = 5; /*Maxiumum number of values*/
int count = 0; /*To keep track of entries*/

int main() {
    int pipe1[2]; /*File descriptors for pipe 1*/
    int pipe2[2]; /*File descriptors for pipe 2*/
    pid_t pid; /*Process ID*/
    char read_msg_1[STR_LEN]; /*C string For reading from pipe*/
    char write_msg_1[STR_LEN]; /*C string For writing to pipe*/
    char read_msg_2[STR_LEN]; /*C string For reading from pipe*/
    char write_msg_2[STR_LEN]; /*C string For writing to pipe*/

    /*Create pipes and check for failure*/
     if (pipe(pipe1) == -1) { 
        perror("Pipe failed");
        exit(1);
    } 
    if (pipe(pipe2) == -1) { 
        perror("Pipe failed");
        exit(1);
    } 
    
    /*Create processes*/
    pid = fork();

/*-----------------------------------------------------*/
    /*Check process ID*/
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }


    /*---------Child | Consumer -----------*/
     if (pid == 0) { /*Child if process ID = 0*/
        close(pipe1[1]); /*Close pipe1 writing*/
        close(pipe2[0]); /*Close pipe2 reading*/

        while(count < MAX) {
            int bytesRead = read(pipe1[0], read_msg_1, sizeof(read_msg_1) - 1);
            /*If received message from pipe1 (from producer)*/
            if (bytesRead > 0) {
                read_msg_1[bytesRead] = '\0';  /*Null-terminate the string*/ 
                printf("Consumer: %s\n", read_msg_1);  /*Print message from pipe1 (from producer)*/
                /*Send consumed signal across pipe2 to producer*/
                strcpy(write_msg_2, "CONSUMED");
                write(pipe2[1], write_msg_2, strlen(write_msg_2) + 1);
            }
        }

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);

     } 
      /*---------Parent  | Producer -----------*/
     else { /*Parent if process ID is non zero*/
        close(pipe1[0]); /*Close pipe1 reading*/
        close(pipe2[1]); /*Close pipe2 writing*/

        int input; /*temp variable for entry*/
        /*run until we have the required number of entries*/
        while(count < MAX) { 
            int bytesRead = 0; 
            if(count > 0) { /*read only if there is at least one entry*/
                bytesRead = read(pipe2[0], read_msg_2, sizeof(read_msg_2) - 1);
            }
            read_msg_2[bytesRead] = '\0'; /*add null terminator*/

            /*If first entry or messgae from pipe is a valid consumer signal*/
            if ((count == 0) || (bytesRead > 0 && !(strcmp(read_msg_2, "CONSUMED")))) { 
                    printf("Producer: ");
                    scanf("%d", &input); /*enter number*/
                    getchar();
                    sprintf(write_msg_1, "%d", input); /*write number to message for pipe1*/
                    write(pipe1[1], write_msg_1, strlen(write_msg_1) + 1); /*write to pipe1*/
                    count++; /*increment entries*/
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);
     }

    exit(0);
    return 0;
}