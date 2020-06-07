#include "include/server.h"

int time_inactivity = -1; // -1 -> infinito
int time_execution = -1;  // -1 -> infinito

int fd_cl_sv, fd_cl_sv_read, fd_sv_cl, fd_sv_cl_write;

int **pidsfilhos;


//int fd_historico; // file descriptor historico



Tarefa* tarefas;
int tar;
/*
char* itoa(int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return b;
}
*/

void signIntHandler(int signum){
	if(fork() == 0){
		int ret = execlp("rm","rm","pipe_task_done",NULL);
		_exit(0);
	}
	wait(0L);
	for(int i = 0; i<20;i++){
		printf("pid:    %d\n",tarefas[i]->pidT );
		printf("status: %d\n",tarefas[i]->status );
		printf("tarefa: %s\n",tarefas[i]->tarefa);
	}
	_exit(0);
}

int isdigitSTR(char *buffer){
	for(int i = 0; buffer[i]; i++){
		if(!isdigit(buffer[i]) && buffer[i] != '\0') {
			printf("%c\n",buffer[i] );
			return 0;
		}
	}
	return 1;
}

void sigusr1SignalHandler(int signum){
	int fd_fifo;
	int res=0;

	char buffer[MAX_LINE_SIZE];
	char buffer2[MAX_LINE_SIZE];


	//if (mkfifo(fifo_name,0666) == -1){
	//	perror("mkfifo from child");
	//}

	if (mkfifo("pipe_task_done",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from child");
	}


	if((fd_fifo = open("pipe_task_done",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-2");
	}

	while((res = read(fd_fifo,buffer,MAX_LINE_SIZE))>0){
		buffer[res] = '\0';
	
	
		if(isdigitSTR(buffer)){ // se for um número -> é o pid de alguma tarefa
			int i = 0;
			int pid = atoi(buffer);
			while(tarefas[i] && tarefas[i]->pidT != pid) i++;
			if (tarefas[i]->pidT == pid){ // já existe tarefa em execucao
				tarefas[i]->status = 2;
			}
			else { // realloc do array
				printf("entrou aqui e não é suposto\n");
			}
		}
		else{
			printf("DEBGUG\n");
		}
		
	}


	close(fd_fifo);

	tar++;
}

void init_tarefa(){
	tarefas = calloc(20, sizeof(Tarefa));
	pidsfilhos = calloc(20,sizeof(int*));
	for(int i = 0; i<20;i++){
		tarefas[i] = calloc(1,sizeof(struct struct_tarefa));
	}
}




void setTimeInactivity(int t){
    time_inactivity = t;
}

void setTimeExecution(int t){
    time_execution = t;
}


void printaAjuda(){
	write(1,"argus$ :\n       ajuda\n       tempo-inatividade $(segs)\n       tempo-execucao $(segs)\n       executar p1 | p2 ... | pn\nargus$ $(args) :\n       -h\n       -i $(segs)\n       -m $(segs)\n       -e 'p1 | p2 ... | pn'\n",210);
}

void tarefaTerminada(){
	int fd_fifo;
	int res;

	char buf[MAX_LINE_SIZE];


	while((fd_fifo = open("pipe_task_done",O_WRONLY)) == -1); // enquanto o pai não criar o fifo, o filho espera

	res = sprintf(buf,"%d",getpid()); // buf <- pid

	write(fd_fifo,buf,res); // filho comunica ao pai o seu pid;

	close(fd_fifo);

}

void tarefaEmExecucao(char *buffer){

}

int exec_command(char* command)
{
    char* exec_args[20];
    char* string;
    int exec_ret = 0;
    int i = 0;

    string = strtok(command, " ");

    while (string != NULL) {
        exec_args[i] = string;
        string = strtok(NULL," ");
        i++;
    }

    exec_args[i] = NULL;

    exec_ret = execvp(exec_args[0], exec_args);

    return exec_ret;
}

void execution_timeHandler(int signum){

}

int exec_pipe(char *buffer){

	//tarefaEmExecucao(buffer);

	// contar número de comandos passados à função
	int n = 1,pid;                                  // -> número de comandos
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '|')
            n++;
    }
    printf("%d\n", tar);
	pidsfilhos[tar] = calloc(n,sizeof(int)); // não dá para fazer assim

    // parse do buffer para comandos
    char commands [n][128];                     // -> array com os comandos
    bzero(commands, n * 128 * sizeof(char));    // -> coloca array a zero

    for (int i_buffer = 0, i_commands = 0, i_command = 0; i_buffer < strlen(buffer); i_buffer++) {
        if (buffer[i_buffer] == '|') {
            commands[i_commands][i_command] = '\0';
            i_commands++;
            i_command = 0;
        }
        else if (buffer[i_buffer] != '\n'){
            commands[i_commands][i_command] = buffer[i_buffer];
            i_command++;
        }
    }

    int p[n-1][2];                              // -> matriz com os fd's dos pipes
    int status[n];                              // -> array que guarda o return dos filhos

    // criar os pipes conforme o número de comandos
    for (int i = 0; i < n-1; i++) {
        if (pipe(p[i]) == -1) {
            perror("Pipe não foi criado");
            return -1;
        }
    }

    // criar processos filhos para executar cada um dos comandos
    for (int i = 0; i < n; i++) {

        if (i == 0) {
            switch(pid=fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:

                    // codigo do filho 0

                    close(p[i][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);
                    
                    exec_command(commands[i]);

                    _exit(0);
                default:
                    close(p[i][1]);
                    pidsfilhos[tar][i] = pid;
            }
        }
        else if (i == n-1) {
            switch(pid = fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    
                    // codigo do filho n-1

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);
                    
                    exec_command(commands[i]);

                    _exit(0);
                default:
                    close(p[i-1][0]);
                    pidsfilhos[tar][i] = pid;
            }
        }
        else {
            switch(pid = fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:

                    // codigo do filho i

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);

                    close(p[i][0]);

                    exec_command(commands[i]);

                    _exit(0);
                default:
                    close(p[i-1][0]);
                    close(p[i][1]);
                    pidsfilhos[tar][i] = pid;
            }
        }

    }

    for (int i = 0; i < n; i++)
    {
        wait(&status[i]);

        //if (WIFEXITED(status[i])) {
        //    printf("[PAI]: filho terminou com %d, %d\n", WEXITSTATUS(status[i]),pidsfilhos[tar][i]);
        //}
    }

    bzero(buffer, MAX_LINE_SIZE * sizeof(char));
    
    //printf("\n\n");

	return 0;
}




int interpreter(char *line){
	int r = 1;
	char *aux = malloc(strlen(line) * sizeof(char));
	strcpy(aux,line);
    char *string = strtok(aux," ");
    int pid;

    if(strcmp(string,"tempo-inatividade") == 0 || strcmp(string,"-i") == 0){
        setTimeInactivity(atoi(strtok(NULL," ")));
		printf("%d\n",time_inactivity );
    }
    else if(strcmp(string,"tempo-execucao") == 0 || strcmp(string,"-m") == 0){
        setTimeExecution(atoi(strtok(NULL," ")));
    	printf("%d\n",time_execution );
	}
	else if(strcmp(string,"-e") == 0 || strcmp(string,"exec") == 0){
		string = strtok(NULL,"\0");
		if((pid = fork()) == 0){
			

			
			dup2(fd_sv_cl_write,1);
			

//******************TESTE*******************
// acrescentar uma tarefa em execução

//******************************************

			exec_pipe(string);

			// tarefa concluida
			kill(getppid(),SIGUSR1);

			tarefaTerminada();

			_exit(0);
		}else{ // inicialização da tarefa
			int i = 0;
			while(tarefas[i] && tarefas[i]->pidT != pid) i++;
			if(!tarefas[i] && tar < i){
				tarefas[tar]->pidT = pid;
				tarefas[tar]->status = 1;
				tarefas[tar]->tarefa = calloc(strlen(string),sizeof(char));
				strcpy(tarefas[tar]->tarefa,string);
			}
			else { // realloc do array

			}
		}

	}
	/*
	else if(strcmp(string,"listar") == 0 || strcmp(string,"-l") == 0) {
		printalogs();
		r = 0; // não insere no logs.txt
	}
	else if(strcmp(string,"terminar") == 0 || strcmp(string,"-t") == 0){
	
	}
	else if(strcmp(string,"historico") == 0 || strcmp(string,"-r") == 0){
	
	}
	*/
	else if(strcmp(string,"ajuda") == 0 || strcmp(string,"-h") == 0){
		printaAjuda();
	}
    else r = 0;

	free(aux);

	return r;
}



int main(int argc, char* argv[]){
	init_tarefa();
	tar = 0;
	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,signIntHandler);
	char buf[MAX_LINE_SIZE];
	int bytes_read;
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

	

//**************TESTE**************************************

	//if((historico = open("historico.txt" , O_CREAT | O_WRONLY | O_TRUNC, 0777)) == -1){ // cria ficheiro caso não exista - caso exista - limpa
	//		perror("open write log");
	//		return -1;
	//}

//*********************************************************











	

	while((bytes_read = read(fd_cl_sv_read,buf,MAX_LINE_SIZE)) > 0){ // lê do pipe, executa
		buf[bytes_read] = '\0';
		interpreter(buf);
    	bzero(buf, MAX_LINE_SIZE * sizeof(char));
	}



//	close(logfile);
	close(fd_cl_sv_read);
	close(fd_cl_sv);
	close(fd_sv_cl);
	close(fd_sv_cl_write);

}