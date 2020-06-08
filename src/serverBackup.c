#include "include/server.h"

int time_inactivity = -1; // -1 -> infinito
int time_execution = -1;  // -1 -> infinito

int fd_cl_sv, fd_cl_sv_read, fd_sv_cl, fd_sv_cl_write;
int tarefaResp;
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

void sigIntHandler(int signum){
	if(fork() == 0){
		int ret = execlp("rm","rm","pipe_task_done",NULL);
		_exit(0);
	}
	wait(0L);
	for(int i = 0; i<20;i++){
		printf("pid:    %d\n",tarefas[i]->pidT );
		printf("status: %d\n",tarefas[i]->status );
		printf("tarefa: %s\n",tarefas[i]->tarefa);
		free(tarefas[i]);
	}
	free(pidsfilhos);
	_exit(0);
}

int isdigitSTR(char *buffer){
	for(int i = 0; buffer[i]; i++){
		if(!isdigit(buffer[i]) && buffer[i] != '\0') {
			return 0;
		}
	}
	return 1;
}

void sigusr1SignalHandler(int signum){
	int fd_fifo;
	int res=0;

	char buffer[MAX_LINE_SIZE];

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

}
void execution_timeHandler(int signum){ // handler do pai, para abrir fifo de comunicação com filho, para que ele passe a tarefa
	int currentTarefa;
	int fd_fifo;
	int res;
	char buffer[MAX_LINE_SIZE];
	int status;
	if (mkfifo("pipe_task_executionTime",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from parent");
	}

	if((fd_fifo = open("pipe_task_executionTime",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-3");
	}
	
	res = read(fd_fifo,buffer,MAX_LINE_SIZE); // lê número da tarefa
	
	buffer[res] = '\0';

	currentTarefa = atoi(buffer); // transforma em um int

	tarefas[currentTarefa]->status = 4;

	

	waitpid(tarefas[currentTarefa]->pidT, &status, 0);

	close(fd_fifo);
}

void init_tarefa(){
	tarefas = calloc(20, sizeof(Tarefa));
	pidsfilhos = calloc(20,sizeof(int*));
	for(int i = 0; i<20;i++){
		tarefas[i] = calloc(1,sizeof(struct struct_tarefa));
	}
}

void sigExecutionAlarmHandler(int signum){ // handler do filho que recebe o sinal a informar que tarefa ultrapassou o tempo permitido
	kill(getppid(),SIGUSR2); // avisa o pai que vai comunicar via fifo - pipe_task_executionTime
	int fd_fifo;
	int res;
	char buf[MAX_LINE_SIZE];
	while((fd_fifo = open("pipe_task_executionTime",O_WRONLY)) == -1); // enquanto o pai não criar o fifo, o filho espera

	res = sprintf(buf,"%d",tar); // buf <- tarefa em questão

	write(fd_fifo,buf,res); // filho comunica ao pai o seu pid;

	int i = 0;
	while(pidsfilhos[tar][i]){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}

	close(fd_fifo);

	_exit(0);
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

void killProcessUSR1_handler(int signum){
	int i = 0;
	while(pidsfilhos[tar][i]){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}
	_exit(0);
}

void terminaTarefa(int tarefa){
	if(!tarefas[tarefa-1] || tarefas[tarefa-1]->status != 1) {
		printf("Tarefa inválida\n");
		return;
	}
	int pid = tarefas[tarefa-1]->pidT;
	int status;
	kill(pid,SIGUSR1);
	waitpid(pid,&status,0);
	if(WIFEXITED(status)){
		printf("tarefa terminada - exit status %d\n", WEXITSTATUS(status));
	}
	tarefas[tarefa-1]->status = 5;
}

void setTimeInactivity(int t){
    time_inactivity = t;
}

void setTimeExecution(int t){
    time_execution = t;
}


void printaAjuda(){
	write(1,"argus$ :\n       ajuda\n       tempo-inatividade $(segs)\n       tempo-execucao $(segs)\n       executar p1 | p2 ... | pn\n       listar\n       historico\n       terminar $(tarefa)\nargus$ $(args) :\n       -h\n       -i $(segs)\n       -m $(segs)\n       -e 'p1 | p2 ... | pn'\n       -l\n       -r\n       -t $(tarefa)\n\0",307);
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

void warnParentInactivityHandler(int signum){
	kill(tarefaResp,SIGINT); // responsável 

}

void killProcessUSR2_handler(int signum){ // comunica com pai a dizer que tarefa vai terminar por tempo de inatividade excedido
	kill(getppid(),SIGQUIT);
	int fd_fifo;
	int res;
	char buf[MAX_LINE_SIZE];
	while((fd_fifo = open("pipe_task_inactivityTime",O_WRONLY)) == -1); // enquanto o pai não criar o fifo, o filho espera

	res = sprintf(buf,"%d",tar); // buf <- tarefa em questão

	write(fd_fifo,buf,res); // filho comunica ao pai a sua tarefa;

	int i = 0;
	while(pidsfilhos[tar][i]){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}

	close(fd_fifo);

	_exit(0);
}

void sigQuitInactivity(int signum){
	int currentTarefa;
	int fd_fifo;
	int res;
	char buffer[MAX_LINE_SIZE];
	int status;
	if (mkfifo("pipe_task_inactivityTime",0666) == -1){ // cria fifo com pipe_task(tarefa)
		perror("mkfifo from parent");
	}

	if((fd_fifo = open("pipe_task_inactivityTime",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-3");
	}
	
	res = read(fd_fifo,buffer,MAX_LINE_SIZE); // lê número da tarefa
	
	buffer[res] = '\0';

	currentTarefa = atoi(buffer); // transforma em um int

	tarefas[currentTarefa]->status = 3;

	

	waitpid(tarefas[currentTarefa]->pidT, &status, 0);

	close(fd_fifo);
}

int exec_pipe(char *buffer){
	tarefaResp = getpid();
	signal(SIGALRM,sigExecutionAlarmHandler);
	signal(SIGUSR1,killProcessUSR1_handler); // kill command
	signal(SIGINT,killProcessUSR2_handler); // inatividade


	//tarefaEmExecucao(buffer);

	// contar número de comandos passados à função
	int n = 1,pid;                                  // -> número de comandos
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '|')
            n++;
    }

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
    int p_inat[2];
    int res;
    char buf[MAX_LINE_SIZE];
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
                	if(pipe(p_inat) == -1){
                	    perror("Pipe não foi criado");
                	    return -1;
					}
                	if(!fork()){

						signal(SIGALRM,warnParentInactivityHandler);
                		close(p[i][0]);
                		close(p_inat[1]);

                		if(time_inactivity > 0) 
						{
								alarm(time_inactivity);
						}
                    	while((res = read(p_inat[0],buf,MAX_LINE_SIZE)) > 0){
                    		if(time_inactivity > 0) alarm(time_inactivity);
                    		write(p[i][1],buf,res);

    						bzero(buf, MAX_LINE_SIZE * sizeof(char));
                    	}
                    	
                    	close(p[i][1]);
                	}
                	else{
                    	
                    	close(p_inat[0]);

                    	dup2(p_inat[1],1);
                    	close(p_inat[1]);
                    
                    
                    	exec_command(commands[i]);

                	}
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
                	if(pipe(p_inat) == -1){
                	    perror("Pipe não foi criado");
                	    return -1;
					}
					if(!fork()){
						signal(SIGALRM,warnParentInactivityHandler);

                    	close(p[i][0]);
                   		close(p_inat[1]);
                   		if(time_inactivity > 0) alarm(time_inactivity);
                   		while((res = read(p_inat[0],buf,MAX_LINE_SIZE)) > 0){
                   			if(time_inactivity > 0) alarm(time_inactivity);
                    		write(p[i][1],buf,res);

    						bzero(buf, MAX_LINE_SIZE * sizeof(char));
                    	}
                    	close(p[i][1]);


					}
					else{
                    	dup2(p[i-1][0],0);
                    	close(p[i-1][0]);

                   		dup2(p_inat[1],1);
                   		close(p_inat[1]);
                   		close(p_inat[0]);
                    	exec_command(commands[i]);
                	}
                    _exit(0);
                default:
                    close(p[i-1][0]);
                    close(p[i][1]);
                    pidsfilhos[tar][i] = pid;
            }
        }

    }
    // depois de criar os filhos, alarm
    if(time_execution == -1);
    else alarm(time_execution);
    
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


void printaHistorico(){
	char *aux = "";
	char aux2[MAX_LINE_SIZE];
	char *string = calloc(tar*MAX_LINE_SIZE,sizeof(char));
	for(int i = 0; i < tar ; i++){
		if(tarefas[i]->status == 1);
		else{
			if(tarefas[i]->status == 2) aux = "concluída";
			else if(tarefas[i]->status == 3) aux = "max inatividade";
			else if(tarefas[i]->status == 4) aux = "max execução";
			else if(tarefas[i]->status == 5) aux = "killed";
			sprintf(aux2,"#%d, %s: %s\n", i+1,aux,tarefas[i]->tarefa);
			strcat(string,aux2);
		}
	}
	printf("%s", string);
}

void printaTarefasEmExecucao(){
	char aux[MAX_LINE_SIZE];
	char *string = calloc(tar*MAX_LINE_SIZE,sizeof(char));
	for(int i = 0; i < tar ; i++){
		if(tarefas[i]->status == 1) {
			sprintf(aux,"#%d : %s\n", i+1,tarefas[i]->tarefa);
			strcat(string,aux);
		}
	}
	printf("%s", string);
}



int interpreter(char *line){
	
	signal(SIGUSR2,execution_timeHandler);
	
	int r = 1;
	char *aux = malloc(strlen(line) * sizeof(char));
	strcpy(aux,line);
    char *string = strtok(aux," ");
    int pid;

    if(strcmp(string,"tempo-inatividade") == 0 || strcmp(string,"-i") == 0){
        setTimeInactivity(atoi(strtok(NULL," ")));
    }
    else if(strcmp(string,"tempo-execucao") == 0 || strcmp(string,"-m") == 0){
        setTimeExecution(atoi(strtok(NULL," ")));
	}
	else if(strcmp(string,"-e") == 0 || strcmp(string,"exec") == 0){
		string = strtok(NULL,"\0");
		if((pid = fork()) == 0){
			printf("tarefa responsável: %d\n",getpid() );

			
			dup2(fd_sv_cl_write,1);
			close(fd_sv_cl_write);
			

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
				strcpy(tarefas[tar++]->tarefa,string);
			}
			else { // realloc do array

			}
		}

	}
	
	else if(strcmp(string,"listar") == 0 || strcmp(string,"-l") == 0) {
		int save = dup(1);
		dup2(fd_sv_cl_write,1);
		printaTarefasEmExecucao();
		dup2(save,1);
	}
	
	else if(strcmp(string,"terminar") == 0 || strcmp(string,"-t") == 0){
		//printf("%d\n",atoi(strtok(NULL,"\0"))); 	// [DEBUG]
		terminaTarefa(atoi(strtok(NULL,"\0")));
	}
	else if(strcmp(string,"historico") == 0 || strcmp(string,"-r") == 0){
		int save = dup(1);
		dup2(fd_sv_cl_write,1);
		printaHistorico();
		dup2(save,1);
	}
	else if(strcmp(string,"ajuda") == 0 || strcmp(string,"-h") == 0){
		int save = dup(1);
		dup2(fd_sv_cl_write,1);
		printaAjuda();
		dup2(save,1);
	}
    else r = 0;

	free(aux);

	return r;
}



int main(int argc, char* argv[]){
	printf("main %d\n",getpid() );
	init_tarefa();
	tar = 0;
	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,sigIntHandler);
	signal(SIGQUIT,sigQuitInactivity);
	char buf[MAX_LINE_SIZE];
	int bytes_read;
	int pid;
	int status;

	if((pid = fork()) == 0){ // cria 2 fifos
		execl("/home/joao/SO_20/src/mkfifo","mkfifo",NULL);
		_exit(1);
	}
	else{
		wait(&status);
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