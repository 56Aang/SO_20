#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_LINE_SIZE 1024


int readlinha(int fd, char * buffer, int nbyte){
	int i = 0;

	while(i < nbyte-1 && read(fd, buffer + i, 1 ) > 0 && buffer[i] != '\n')
        i++;
    if (i >= nbyte)
        buffer[i] = '\n';
    else
        buffer[i] = '\0';

    return i;
}

int main(){
    int res;
    char buf[MAX_LINE_SIZE];
    int fd_cl_sv_write, fd_sv_cl_read;
    int pid;

    if((fd_cl_sv_write = open("fifo-cl-sv",O_WRONLY)) == -1){
        perror("open");
        return -1;
    }
    else 
        printf("[DEBUG] opened fifo cl-sv for [writing]\n");


    if((fd_sv_cl_read = open("fifo-sv-cl",O_RDONLY)) == -1){
       perror("open");
        return -1;
    }
    else
        printf("[DEBUG] opened fifo cl-sv for [reading]\n");

    if((pid = fork()) == 0){
        while((res = read(fd_sv_cl_read,buf,MAX_LINE_SIZE)) > 0){
            write(1,buf,res);
        }
        _exit(0);
    }
    else{ // lê do ecrã, escreve no pipe
        while((res = read(0,buf,MAX_LINE_SIZE)) > 0){
            write(fd_cl_sv_write,buf,res);
        }        
        kill(pid,SIGKILL);

    }
    

    close(fd_cl_sv_write);
    close(fd_sv_cl_read);

    return 0;
}