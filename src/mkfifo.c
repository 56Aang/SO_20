#include <stdio.h>
#include <sys/stat.h>

int main(){

	if (mkfifo("fifo-sv-cl",0666) == -1){
		perror("mkfifo");
	}
	if (mkfifo("fifo-cl-sv",0666) == -1){
		perror("mkfifo");
	}
	if (mkfifo("pipe_task_inactivityTime",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from parent");
	}
	if (mkfifo("pipe_task_executionTime",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from parent");
	}
	if (mkfifo("pipe_task_done",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from child");
	}
	
	return 0;
}