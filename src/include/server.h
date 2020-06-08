#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h> // isdigit()

#define MAX_LINE_SIZE 128
typedef struct struct_tarefa{
	//pid_t *pid;
	char *tarefa;
	pid_t pidT;
	int status; // 0 - not used, 1 - em execução, 2 - concluida , 3 - max inatividade , 4 - max execução, 5 - killed
}*Tarefa;
