#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    printf("Waiting for connection...\n");
    // Socket Creation for Communication with remote control
    int sockfd, newsockfd, portno, status;
    socklen_t clilen;
    char command;
    char feedback[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, x, y;

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    printf("Connection Successful\n");

    // Pipe Creation for communication with HOIST
    int fil_des[2][2];

    if (pipe(fil_des[0]) == -1)
        error("ERROR in pipe 1 creation");
    if (pipe(fil_des[1]) == -1)
        error("ERROR in pipe 2 creation");

    char fil_des_0[2];
    char fil_des_1[2];

    // Converting the file descriptor into string
    sprintf(fil_des_0, "%d", fil_des[0][0]);
    sprintf(fil_des_1, "%d", fil_des[1][1]);

    // HOIST Process creation
    pid_t hoist_pid = fork();
    if (hoist_pid == -1)
        error("ERROR in fork");
    if (hoist_pid == 0)
    {
        /* HOIST PROCESS */
        if ((execlp("./hoist", "./hoist", fil_des_0, fil_des_1, (char *)NULL)) == -1)
        {
            perror("Exec Receiver");
        }
        exit(EXIT_FAILURE);
    }

    /* HOIST SERVER */
    while (1)
    {
        bzero(feedback, 256);
        if (read(newsockfd, &command, sizeof(char)) == -1)
            error("ERROR in reading from client");

        if (command == 'l' || command == 'd')
        {
            int fd = newsockfd;

            if (write(fil_des[0][1], &command, sizeof(char)) == -1)
                error("ERROR in writing to pipe 1");

            if (command == 'l')
            {
                strcpy(feedback, "The HOIST is rising");
                printf("\nThe HOIST is rising\n\n");
            }
            else if (command == 'd')
            {
                strcpy(feedback, "The HOIST is dropping");
                printf("\nThe HOIST is dropping\n\n");
            }

            if (write(newsockfd, feedback, sizeof(feedback)) == -1)
                error("ERROR in writing to client");

            struct timeval tv;
            int retval;

            while (1)
            {
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(newsockfd, &rfds);
                tv.tv_sec = 0;
                tv.tv_usec = 0;

                retval = select((newsockfd + 1), &rfds, NULL, NULL, &tv); // to check for a 't' command for stopping
                if (retval == 1)
                {
                    if (read(newsockfd, &command, sizeof(char)) == -1)
                        error("ERROR in reading from client");
                    if (write(fil_des[0][1], &command, sizeof(char)) == -1)
                        error("ERROR in writing to pipe 1");
                    printf("\nHOIST server will now stop\n");
                }
                if (retval != 1)
                {
                    if (read(fil_des[1][0], feedback, sizeof(feedback)) == -1)
                        error("ERROR in reading from pipe 2");
                    if (write(newsockfd, feedback, sizeof(feedback)) == -1)
                        error("ERROR in writing to client");
                }
                x = strcmp(feedback, "The HOIST has reached the highest height");
                y = strcmp(feedback, "The HOIST has reached the lowest height");
                if (x == 0 || y == 0)
                {
                    printf("\n%s\n", feedback);
                    break;
                }
                if (command == 't')
                {
                    if (read(fil_des[1][0], feedback, sizeof(feedback)) == -1)
                        error("ERROR in reading from pipe 2");
                    printf("%s\n", feedback);
                    break;
                }
            }
        }
        else if (command == 'e')
        {
            if (write(fil_des[0][1], &command, sizeof(char)) == -1)
                error("ERROR in writing to pipe 1");
            printf("The HOIST server has terminated\n");
            break;
        }
    }
    wait(&status);
    return 0;
}
