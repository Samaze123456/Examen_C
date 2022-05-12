#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    pid_t pid;

    int pipeFD[2];

    if((pipe(pipeFD)) < 0)
    {
        perror("Pipe error ");
        exit(errno);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork error ");
        exit(errno);
    }

    if (!pid)
    {
        // child code
       
        if(!(dup2(pipeFD[1], STDERR_FILENO)))// replace stderr
        {
            perror("Dup2 STDERR error ");
            exit(errno);
        }

        if(!(dup2(pipeFD[1], STDOUT_FILENO)))// replace stdout
        {
            perror("Dup2 STDOUT error ");
            exit(errno);
        }

        char **commandLine = malloc((argc + 1) * sizeof(char *));
        
        if (argc > 1)
        {
            commandLine[0] = argv[1];
            for (int i = 0; i < argc; ++i)
            {
                commandLine[i+1] = argv[i+2];
            }
            commandLine[argc]=NULL;
        }

        if(!(execvp(commandLine[0], commandLine)))
        {
            perror("Execlp child error ");
            exit(errno);
        }

        write(pipeFD[1], "Bad command entered", strlen("Bad command entered"));

        exit(1);
    }
    else
    {
        // parent code

        int waitRespons, status = 1, timeout = 2000, counter = 0;
        char bufferData[PATH_MAX];

        if((close(pipeFD[1])) < 0) //close pipe writing
        {
            perror("Close pipe writing parent error ");
            exit(errno);
        }

        while(WIFEXITED(status) == 0 && timeout > counter)
        {
            waitRespons = waitpid(pid, &status, WNOHANG);
            counter ++;
            sleep(0.1);
        }

        if (timeout <= counter)
        {
            kill(pid, SIGKILL);
            printf("Child process time out\n");
        }

        if ((read(pipeFD[0], bufferData, PATH_MAX)) == -1)
        {
            perror("Read error ");
            exit(errno);
        }
        
        printf("Results :\n%s\n", bufferData);

    }
    exit(0);
}
