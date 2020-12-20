#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int fil_des_0 = atoi(argv[1]); // to read from pipe 1
    int fil_des_1 = atoi(argv[2]); // to write to pipe 2

    char command;
    char feedback[256];
    char end_of_run_up[] = "The HOIST has reached the highest height";
    char end_of_run_down[] = "The HOIST has reached the lowest height";
    char stop[] = "The HOIST has stopped";
    int HOIST_height = 0;
    int v_max = 200;
    int fd = fil_des_0;

    while (1)
    {

        if (read(fil_des_0, &command, sizeof(char)) == -1)
            error("ERROR in reading from pipe 1");

        if (command == 'l' || command == 'd')
        {

            struct timeval tv;
            int retval;

            while (1)
            {
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(fil_des_0, &rfds);
                tv.tv_sec = 0;
                tv.tv_usec = 0;

                retval = select((fd + 1), &rfds, NULL, NULL, &tv);
                if (retval == 1)
                {
                    if (read(fil_des_0, &command, sizeof(char)) == -1)
                        error("ERROR in reading from pipe 1");
                }

                if (command == 'l')
                {
                    if (HOIST_height == v_max)
                    {
                        if (write(fil_des_1, &end_of_run_up, sizeof(end_of_run_up)) == -1)
                            error("ERROR in writing to pipe 2");
                        break;
                    }
                    HOIST_height++;
                }
                else if (command == 'd')
                {
                    if (HOIST_height == 0)
                    {
                        if (write(fil_des_1, &end_of_run_down, sizeof(end_of_run_down)) == -1)
                            error("ERROR in writing to pipe 2");
                        break;
                    }
                    HOIST_height--;
                }
                else if (command == 't')
                {
                    if (write(fil_des_1, &stop, sizeof(stop)) == -1)
                        error("ERROR in writing to pipe 2");
                    break;
                }

                usleep(200000);
                sprintf(feedback, "%d", HOIST_height);
                if (write(fil_des_1, &feedback, sizeof(feedback)) == -1)
                    error("ERROR in writing to pipe 2");

                printf("HOIST height (Z axis): %dcm\n", HOIST_height);
            }
        }
        else if (command == 'e')
        {
            printf("The HOIST has been swicted off\n");
            break;
        }
    }

    exit(EXIT_SUCCESS);
}