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


char* createBuf(int argc, char* argv[]){
    int tam = 0;
    int i;
    int k = 0;
    for(i = 1; i < argc ; i++) tam += strlen(argv[i]);
    char * buf = malloc(sizeof(char) * (tam + argc-1 + 2)); // tamanho dos argumentos + o numero de espaços + \0
    for(i = 1 ; i<argc; i++){
        strcpy(buf+k,argv[i]);
        k += strlen(argv[i]);
        if(i != argc-1){
            buf[k++] = ' ';
        }
    }
    return buf;
}

int main(int argc, char* argv[]){
    int res;
    char buf[MAX_LINE_SIZE];
    int fd_cl_sv_write, fd_sv_cl_read;
    int pid;
    int status;

    if((fd_cl_sv_write = open("fifo-cl-sv",O_WRONLY)) == -1){ // open named pipe for write (cliente -> sv)
        perror("open");
        return -1;
    }
    else 
        printf("[DEBUG] opened fifo cl-sv for [writing]\n");


    if((fd_sv_cl_read = open("fifo-sv-cl",O_RDONLY)) == -1){ // open named pipe for read (sv -> cliente)
       perror("open");
        return -1;
    }
    else
        printf("[DEBUG] opened fifo cl-sv for [reading]\n");

    if((pid = fork()) == 0){
        if(argc > 1){
            res = read(fd_sv_cl_read,buf,MAX_LINE_SIZE);
            write(1,buf,res);
        }
        
        else{
            while((res = read(fd_sv_cl_read,buf,MAX_LINE_SIZE)) > 0){ // escrever tudo que vem do pipe sv->cl no terminal
                write(1,buf,res);
            }
            _exit(0);
        }
    }
    else{ // lê do ecrã, escreve no pipe (cl -> sv)

        if(argc > 1){ // ./prog xxx xxx xxx (linha de comandos)
            char *a = createBuf(argc,argv);
            write(fd_cl_sv_write,a,strlen(a));
            write(fd_cl_sv_write,"\n",2);

        }
        else{ // ./prog -> interface interpretada

            while((res = read(0,buf,MAX_LINE_SIZE)) > 0){
                write(fd_cl_sv_write,buf,res);
            }        
            kill(pid,SIGKILL); // quando acaba, mata o processo que está a ler do pipe -> prog termina
        }

    }
    

    close(fd_cl_sv_write);
    close(fd_sv_cl_read);

    return 0;
}