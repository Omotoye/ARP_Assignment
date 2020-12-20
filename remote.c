#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

char command;

// function for error management
void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Signal handler function to stop the HOIST
void handle_sigtstp(int sig)
{
    command = 't'; // The command t will be sent to the HOIST server to terminate the HOIST
}

int main(int argc, char *argv[])
{

    // Signal Handler
    signal(SIGTSTP, &handle_sigtstp);

    // Socket Creation
    int sockfd, portno, n, h_height, l_height;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char feedback[256];

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0],
          (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Connection Successful\nConnection details-->  (IP Address: %s and Port Number: %s\n", argv[1], argv[2]);
    printf("\nWelcome to our online HOIST control\n");
    printf("Your HOIST has a range 200cm in the vertical direction\nand a speed of 5cm/s\n");

    // Remote Commands and Feedback

    while (1)
    {
        printf("\nEnter l to lift, d to drop, press CTRL+Z to stop and e to terminate\n");
        printf("Please enter a command: ");
        fflush(stdout);
        scanf(" %c", &command);
        if (command == 'l' || command == 'd') // Checks to see if the command is to lift or drop
        {
            if (write(sockfd, &command, sizeof(char)) == -1)
                error("ERROR in writing to server");
            while (1)
            {
                bzero(feedback, 256);
                if (read(sockfd, feedback, 256) == -1)
                    error("ERROR in reading from server");
                h_height = strcmp(feedback, "The HOIST has reached the highest height");
                l_height = strcmp(feedback, "The HOIST has reached the lowest height");
                if (h_height == 0 || l_height == 0)
                {
                    printf("\n%s\n", feedback);
                    break;
                }
                else
                {
                    int rising = strcmp(feedback, "The HOIST is rising");
                    int dropping = strcmp(feedback, "The HOIST is dropping");
                    if (rising == 0 || dropping == 0)
                        printf("\n%s\n\n", feedback);
                    else
                        printf("HOIST height (Z axis): %scm\n", feedback);
                }
                if (command == 't')
                {
                    if (write(sockfd, &command, sizeof(char)) == -1)
                        error("ERROR in writing to socket");
                    bzero(feedback, 256);
                    if (read(sockfd, feedback, 256) == -1)
                        error("ERROR in reading from server");
                    printf("HOIST height (Z axis): %scm\n", feedback);
                    printf("\nThe HOIST has been stopped\n");
                    break;
                }
            }
        }
        else if (command == 'e')
        {
            if (write(sockfd, &command, sizeof(char)) == -1)
                error("ERROR in writing to server");
            printf("The remote control is disconnected\n");
            break;
        }
        else
        {
            printf("You have entered an Invalid command, try again\n");
        }
    }
    return 0;
}
