#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_LINE_SIZE 1024

int time_inactivity = -1; // -1 -> infinito
int time_execution = -1;  // -1 -> infinito

void setTimeInactivity(int t){
    time_inactivity = t;
}

void setTimeExecution(int t){
    time_execution = t;
}

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

int printalogs(){
	int fd = open("log.txt",O_RDONLY);
	printf("%d\n",fd);
	int res;
	char buf[MAX_LINE_SIZE];
	while((res = read(fd,buf,MAX_LINE_SIZE)) > 0)
		write(1,buf,res);
	return 0;
}


int interpreter(char *line){
    char *string = strtok(line," ");
    if(strcmp(string,"tempo-inatividade") == 0){
        setTimeInactivity(atoi(strtok(NULL," ")));
		write(1,"changed\n",9);
    }
    else if(strcmp(string,"tempo-execucao") == 0){
        setTimeExecution(atoi(strtok(NULL," ")));
    	write(1,"changed\n",9);
	}
	else if(strcmp(string,"logs")) 
		printalogs();
    
	return 1;
}

int main(){
	char buf[MAX_LINE_SIZE];
	int bytes_read;
	int logfile, fd_cl_sv, fd_cl_sv_read, fd_sv_cl, fd_sv_cl_write;
	int pid;

	if((pid = fork()) == 0){
		execl("/home/joao/SO/src/mkfifo","mkfifo",NULL);
		_exit(-1);
	}
	else{
		wait(0L);
	}


	if((logfile = open("log.txt",O_CREAT | O_WRONLY),0666) == -1){
		perror("open");
		return -1;
	}


	// open named pipe for reading 
	if((fd_cl_sv_read = open("fifo-cl-sv",O_RDONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo cl-sv for [reading]\n");

	// open named pipe for writing to handle asynchronous clients
	if((fd_cl_sv = open("fifo-cl-sv", O_WRONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo cl-sv for writing\n");


	// open named pipe for writing
	if((fd_sv_cl_write = open("fifo-sv-cl",O_WRONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo sv-cl for [writing]\n");

	// open named pipe for writing to handle asynchronous clients
	if((fd_sv_cl = open("fifo-sv-cl", O_RDONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo sv-cl for reading\n");
	

	while((bytes_read = readlinha(fd_cl_sv_read,buf,MAX_LINE_SIZE)) > 0){ // lê do pipe, executa
		if((pid = fork()) == 0){ // executa, escreve no pipe
			dup2(fd_sv_cl_write,1);
    	    interpreter(buf);

    	    _exit(0);
    	}
		write(logfile,buf,bytes_read);
    	
	}



	close(logfile);
	close(fd_cl_sv_read);
	close(fd_cl_sv);
	close(fd_sv_cl);
	close(fd_sv_cl_write);

}