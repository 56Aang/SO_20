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
/*
int exec(int argc, char* argv[]){

	int i, pid;
	int status[argc];
	int p[argc-1][2];

	for(i = 0; i < argc; i++){
		//Primeiro comando
		if(i == 0){
			if(pipe(p[i]) == -1){
				perror("pipe");
				return -1;
			}
			switch(pid = fork()){
				case -1:
					perror("fork");
					return -1;
				case 0:
					close(p[i][0]);
					dup2(p[i][1], 1);
					close(p[i][1]);
					exec_command(comandos[i]);
					_exit(-1);
				default:
					close(p[i][1]);
			}
		}
		//Ultimo comando
		if(i == argc-1){
			switch(pid = fork()){
				case -1:
					perror("fork");
					return -1;
				case 0:
					dup2(p[i-1][0], 0);
					close(p[i-1][0]);
					exec_command(comandos[i]);
					_exit(-1);
				default:
					close(p[i-1][0]);
			}
		}
		//Comandos intermedios
		else{
			if(pipe(p[i]) != 0){
				perror("pipe");
				return -1;
			}
			switch(pid = fork()){
				case -1:
					perror("fork");
					return -1;
				case 0:
					close(p[i][0]);
					dup2(p[i][1], 1);
					close(p[i][1]);

					dup2(p[i-1][0], 0);
					close(p[i-1][0]);

					exec_command(comandos[i]);
					_exit(-1);
				default:
					break;

			}
		}
	}

	for(i = 0; i < argc; i++) wait(&status[i]);

	return 0;
}
*/


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
	int fd;
	//int i = 0;
	if((fd = open("log.txt",O_RDONLY)) == -1){
		perror("open printalogs");
		return -1;
	}
	int res;
	char buf[MAX_LINE_SIZE];

	while((res = read(fd,buf,MAX_LINE_SIZE)) > 0){
		write(1,buf,res);
	}
	
	close(fd);
	return 0;
}



int interpreter(char *line){
	int r = 1;
	char *aux = malloc(strlen(line) * sizeof(char));
	strcpy(aux,line);
    char *string = strtok(aux," ");
    if(strcmp(string,"tempo-inatividade") == 0){
        setTimeInactivity(atoi(strtok(NULL," ")));
		write(1,"changed\n",9);
    }
    else if(strcmp(string,"tempo-execucao") == 0){
        setTimeExecution(atoi(strtok(NULL," ")));
    	write(1,"changed\n",9);
	}
	else if(strcmp(string,"exec") == 0){
		// funcao pipes| | |
	}
	//else if(strcmp(string,"logs") == 0) {
	//	printalogs();
	//	r = 0; // não insere no logs.txt
	//}
    else r = 0;

	free(aux);

	return r;
}



int main(int argc, char* argv[]){
	char buf[MAX_LINE_SIZE];
	int bytes_read;
	int fd_cl_sv, fd_cl_sv_read, fd_sv_cl, fd_sv_cl_write;
	int pid;
//	int logfile;
	int status;
//	int fd_logs[2];

	if((pid = fork()) == 0){ // cria 2 fifos
		execl("/home/joao/SO_20/src/mkfifo","mkfifo",NULL);
		_exit(1);
	}
	else{
		wait(&status);
	}
	/*
	if(pipe(fd_logs)<0){ // cria 2 pipes
		perror("Pipe fd_logs");
		_exit(0);
	}
	


	if((logfile = open("log.txt" , O_CREAT | O_WRONLY | O_TRUNC, 0777)) == -1){ // cria ficheiro caso não exista - caso exista - limpa
			perror("open write log");
			return -1;
	}
	
	// pipe que recebe dados log's, e escreve no .txt
	if((pid = fork()) == 0){
		close(fd_logs[1]);
		dup2(fd_logs[0],0);
		close(fd_logs[0]);
		dup2(logfile,1);
		close(logfile);
		while((bytes_read = read(0,buf,MAX_LINE_SIZE)) > 0){
			write(1,buf,bytes_read);
			write(1,"\n",2);
		}
	}
	
	close(fd_logs[0]);
	*/


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
	

	while((bytes_read = read(fd_cl_sv_read,buf,MAX_LINE_SIZE)) > 0){ // lê do pipe, executa
		if((pid = fork()) == 0){ // executa, escreve no pipe
			dup2(fd_sv_cl_write,1);
			interpreter(buf);

    	    _exit(0);
    	}
	}



//	close(logfile);
	close(fd_cl_sv_read);
	close(fd_cl_sv);
	close(fd_sv_cl);
	close(fd_sv_cl_write);

}