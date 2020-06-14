#include "include/argus.h"

typedef struct struct_tarefa{
	char *tarefa;
	pid_t pidT;
	int status; // 0 - not used, 1 - em execução, 2 - concluida , 3 - max inatividade , 4 - max execução, 5 - killed
	int o_start;
	int o_size;
}*Tarefa;


int time_inactivity = -1; // -1 -> infinito
int time_execution = -1;  // -1 -> infinito

int fd_cl_sv, fd_cl_sv_read, fd_sv_cl, fd_sv_cl_write;
int **pidsfilhos;
int sizeMax = 20;


Tarefa* tarefas;
int tar; // tarefa atual
int tarefaResp; // tarefa responsável por uma execução


void signIntHandler(int signum){
	close(fd_cl_sv);
	close(fd_cl_sv_read);
	close(fd_sv_cl_write);
	close(fd_sv_cl);
	if(fork() == 0){
		execlp("rm","rm","pipe_task_done",NULL);
		_exit(0);
	}
	if(fork() == 0){
		execlp("rm","rm","pipe_task_executionTime",NULL);
		_exit(0);
	}
	if(fork() == 0){
		execlp("rm","rm","pipe_task_inactivityTime",NULL);
		_exit(0);
	}
	if(fork() == 0){
		execlp("rm","rm","fifo-cl-sv",NULL);
		_exit(0);
	}
	if(fork() == 0){
		execlp("rm","rm","fifo-sv-cl",NULL);
		_exit(0);
	}
	for(int i = 0; i<3;i++){
		wait(0L);
	}
	for(int i = 0; i<sizeMax;i++){
		//printf("pid:    %d\n",tarefas[i]->pidT );
		//printf("status: %d\n",tarefas[i]->status );
		//printf("tarefa: %s\n",tarefas[i]->tarefa);
		//printf("start offset: %d\n",tarefas[i]->o_start );
		//printf("end offset: %d\n",tarefas[i]->o_size );
		if(tarefas[i]->tarefa) free(tarefas[i]->tarefa);
		free(tarefas[i]);
		free(pidsfilhos[i]);
	}
	free(tarefas);
	free(pidsfilhos);
	_exit(0);
}



void sigusr1SignalHandler(int signum){
	int fd_fifo;
	int templog;
	int fd_logs;
	int res=0;
	int aux = 0;

	char buffer[MAX_LINE_SIZE];

	if((fd_fifo = open("pipe_task_done",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-2");
	}

	res = read(fd_fifo,buffer,MAX_LINE_SIZE);
	buffer[res] = '\0';
	close(fd_fifo);
	int t = atoi(buffer);
	if(tarefas[t]){
		tarefas[t]->status = 2;
		char *string = calloc(20,sizeof(char));

		sprintf(string,"temp_out%d.txt",t+1);

		if((templog = open(string,O_RDONLY)) == -1){
			perror("open templog");
		}

		lseek(templog,0,SEEK_SET);
		if((fd_logs = open("logs.txt",O_WRONLY | O_APPEND)) == -1){
			perror("open logs");
		}
		int off = lseek(fd_logs,0,SEEK_END);
		while((res = read(templog,buffer,MAX_LINE_SIZE))>0){

			write(fd_logs,buffer,res);
			aux+=res;
		}
		tarefas[t]->o_start = off;
		tarefas[t]->o_size = aux;
		close(fd_logs);
		close(templog);
		if(!fork()){
			execlp("rm","rm",string,NULL);
			_exit(0);
		}
		free(string);
	}
	else { // realloc do array
		printf("entrou aqui e não é suposto\n");
	}
	write(fd_sv_cl_write,EXIT,sizeOfExit);

}
void printaOutput(int tarefa){
	if(tarefa < 0 || !tarefas[tarefa] || tarefas[tarefa]->status != 2) { // caso não seja válida a tarefa
		write(fd_sv_cl_write,"Tarefa inválida\n",17);
		
	}
	else{
		int fd_logs;
		if((fd_logs = open("logs.txt",O_RDONLY)) == -1){
			perror("open logs");
		}
		int resto = tarefas[tarefa]->o_size;
		lseek(fd_logs,tarefas[tarefa]->o_start,SEEK_SET);
		int res;
		char buffer[MAX_LINE_SIZE];
		int bufSize = MAX_LINE_SIZE;
		bufSize = (resto < bufSize) ? resto : bufSize;
		while((res = read(fd_logs,buffer,bufSize)) > 0){
			write(fd_sv_cl_write,buffer,res);
			resto-= res;
			bufSize = (resto < bufSize) ? resto : bufSize;	
		}
	}
}


void execution_timeHandler(int signum){ // handler do pai, para abrir fifo de comunicação com filho, para que ele passe a tarefa
	int currentTarefa;
	int fd_fifo;
	int res;
	char buffer[MAX_LINE_SIZE];
	int status;

	if((fd_fifo = open("pipe_task_executionTime",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-3");
	}
	
	res = read(fd_fifo,buffer,MAX_LINE_SIZE); // lê número da tarefa
	
	buffer[res] = '\0';

	currentTarefa = atoi(buffer); // transforma em um int

	tarefas[currentTarefa]->status = 4;

	

	waitpid(tarefas[currentTarefa]->pidT, &status, 0);

	char *string = calloc(20,sizeof(char));
	if(!fork()){
		sprintf(string,"temp_out%d.txt",currentTarefa+1);
		execlp("rm","rm",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
	}
	free(string);
	write(fd_sv_cl_write,EXIT,sizeOfExit);

	close(fd_fifo);
}

void realloc_tarefa(){
	tarefas = realloc(tarefas,2*sizeMax*sizeof(Tarefa));
	pidsfilhos = realloc(tarefas,2*sizeMax*sizeof(int*));
	for(int i = sizeMax; i<2*sizeMax;i++){
		tarefas[i] = calloc(1,sizeof(struct struct_tarefa));
	}
	sizeMax = 2*sizeMax;
}

void init_tarefa(){
	tarefas = calloc(sizeMax, sizeof(Tarefa));
	pidsfilhos = calloc(sizeMax,sizeof(int*));
	for(int i = 0; i<sizeMax;i++){
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
	while(pidsfilhos[tar][i] && isdigit(pidsfilhos[tar][i])){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}

	close(fd_fifo);
	free(pidsfilhos[tar]);
	_exit(0);
}

void tarefaTerminada(){
	int fd_fifo;
	int res;

	char buf[MAX_LINE_SIZE];

	while((fd_fifo = open("pipe_task_done",O_WRONLY)) == -1); // enquanto o pai não criar o fifo, o filho espera

	res = sprintf(buf,"%d",tar); // buf <- tarefa

	write(fd_fifo,buf,res); // filho comunica ao pai a sua tarefa;

	close(fd_fifo);


}

void killProcessUSR1_handler(int signum){
	int i = 0;
	while(pidsfilhos[tar][i] && isdigit(pidsfilhos[tar][i])){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}
	free(pidsfilhos[tar]);
	_exit(0);
}

int terminaTarefa(int tarefa){
	if(tarefa <= 0 || !tarefas[tarefa-1] || tarefas[tarefa-1]->status != 1) {
		write(fd_sv_cl_write,"Tarefa inválida\n",strlen("Tarefa inválida\n"));
		return 0;
	}
	int pid = tarefas[tarefa-1]->pidT;
	int status;
	kill(pid,SIGUSR1);
	waitpid(pid,&status,0);
	if(WIFEXITED(status)){
		printf("tarefa terminada - exit status %d\n", WEXITSTATUS(status));
	}
	tarefas[tarefa-1]->status = 5;

	char *string = calloc(20,sizeof(char));
	if(!fork()){
		sprintf(string,"temp_out%d.txt",tarefa);
		execlp("rm","rm",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
	}
	free(string);

	
	return 1;
}

void setTimeInactivity(int t){
    time_inactivity = t;
}

void setTimeExecution(int t){
    time_execution = t;
}


void printaAjuda(){
	write(1,"argus$ :\n",9);
	write(1,"          ajuda\n",16);
	write(1,"          tempo-inatividade $(segs)\n",36);
	write(1,"          tempo-execucao $(segs)\n",33);
	write(1,"          executar p1 | p2 ... | pn\n",36);
	write(1,"          listar\n",17);
	write(1,"          historico\n",20);
	write(1,"          terminar $(tarefa)\n",29);
	write(1,"          output $(tarefa)\n",27);
	write(1,"argus$ $(args) :\n",17);
	write(1,"          -h\n",13);
	write(1,"          -i $(segs)\n",21);
	write(1,"          -m $(segs)\n",21);
	write(1,"          -e 'p1 | p2 ... | pn'\n",32);
	write(1,"          -l\n",13);
	write(1,"          -r\n",13);
	write(1,"          -t $(tarefa)\n",23);
	write(1,"          -o $(tarefa)\n",23);
	write(1,EXIT,sizeOfExit);
}




void sigQuitInactivity(int signum){
	int currentTarefa;
	int fd_fifo;
	int res;
	char buffer[MAX_LINE_SIZE];
	int status;

	if((fd_fifo = open("pipe_task_inactivityTime",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-3");
	}
	
	res = read(fd_fifo,buffer,MAX_LINE_SIZE); // lê número da tarefa
	
	buffer[res] = '\0';

	currentTarefa = atoi(buffer); // transforma em um int

	tarefas[currentTarefa]->status = 3;

	

	waitpid(tarefas[currentTarefa]->pidT, &status, 0);

	write(fd_sv_cl_write,EXIT,sizeOfExit);

	close(fd_fifo);

	char *string = calloc(20,sizeof(char));
	if(!fork()){
		sprintf(string,"temp_out%d.txt",currentTarefa+1);
		execlp("rm","rm",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
	}
	free(string);
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
	while(pidsfilhos[tar][i] && isdigit(pidsfilhos[tar][i])){
		kill(pidsfilhos[tar][i++],SIGKILL);
	}

	close(fd_fifo);
	free(pidsfilhos[tar]);

	_exit(0);
}

void warnParentInactivityHandler(int signum){
	kill(tarefaResp,SIGINT); // responsável 
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

int exec_pipe(char *buffer){
	tarefaResp = getpid();
	signal(SIGALRM,sigExecutionAlarmHandler); // execução
	signal(SIGUSR1,killProcessUSR1_handler);
	signal(SIGINT,killProcessUSR2_handler); // inatividade


	// contar número de comandos passados à função
	int n = 1,pid;                                  // -> número de comandos
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '|')
            n++;
 }
	pidsfilhos[tar] = calloc(n+1,sizeof(int));

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
    int p_aux[2];
    pipe(p_aux);
    char buf[MAX_LINE_SIZE];
    int res;
    // criar os pipes conforme o número de comandos
    for (int i = 0; i < n-1; i++) {
        if (pipe(p[i]) == -1) {
            perror("Pipe não foi criado");
            return -1;
        }
    }
    int i;
    // criar processos filhos para executar cada um dos comandos
    for (i = 0; i < n; i++) {

        if (i == 0) {
            switch(pid=fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho 0
                    if(n>1){
                    close(p[i][0]);
                    	if(time_inactivity > 0){
                    		if(pipe(p_inat) == -1){
                	    		perror("Pipe não foi criado");
                	    		return -1;
							}
							if(!fork()){ //
								signal(SIGALRM,warnParentInactivityHandler);
								close(p_inat[1]);
								alarm(time_inactivity);
							
								while((res = read(p_inat[0],buf,MAX_LINE_SIZE)) > 0){ // enquanto conseguir ler da pipe, dá alarm, e escreve
                    				alarm(time_inactivity);
                    				write(p[i][1],buf,res);
    								bzero(buf, MAX_LINE_SIZE * sizeof(char));
                    			}
                    			close(p[i][1]);
                    			_exit(0);
							}
							else{ // executar para a pipe_inat
								close(p_inat[0]);
								dup2(p_inat[1],1);
								close(p_inat[1]);

								exec_command(commands[i]);
								_exit(0);
							}

                    	}

                    	else{
                        	dup2(p[i][1],1);
							close(p[i][1]);
						}
                    }
                    else{
                    	close(p_aux[0]);
                    	dup2(p_aux[1],1);
                    	close(p_aux[1]);
                	}

                    exec_command(commands[i]);

                    _exit(0);
                default:
                	if(n>1) close(p[i][1]);
                	else close(p_aux[1]);
                    pidsfilhos[tar][i] = pid;
            }
        }
        else if (i == n-1) { // pipe servidor -> cliente
            switch(pid = fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    
                    // codigo do filho n-1

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(p_aux[1],1);
                    close(p_aux[1]);
                    
                    exec_command(commands[i]);

                    _exit(0);
                default:
                    close(p[i-1][0]);
                    close(p_aux[1]);
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
                	if(time_inactivity > 0){
                		if(pipe(p_inat) == -1){
                	    	perror("Pipe não foi criado");
                	    	return -1;
						}
                		if(!fork()){
                			signal(SIGALRM,warnParentInactivityHandler);
                			close(p[i][0]);
                			close(p_inat[1]);
                			alarm(time_inactivity);
                			while((res = read(p_inat[0],buf,MAX_LINE_SIZE)) > 0){
                   				alarm(time_inactivity);
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
                	}

                	else{

                    	dup2(p[i-1][0],0);
                    	close(p[i-1][0]);

                    	dup2(p[i][1],1);
                    	close(p[i][1]);

                    	close(p[i][0]);
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
    if(time_execution == -1);
    else alarm(time_execution);

    char *string = calloc(20,sizeof(char));
    if((pid = fork()) == 0){
    	dup2(p_aux[0],0); // 0 -> p_aux[0]
    	close(p_aux[0]);
    	sprintf(string,"temp_out%d.txt",tar+1);
		execlp("tee","tee",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
    }
    pidsfilhos[tar][i] = pid;
    wait(0L);
    // depois de criar os filhos, alarm
    
    for (i = 0; i < n; i++)
    {
        wait(&status[i]);

    }
    free(pidsfilhos[tar]);
    free(string);

    bzero(buffer, MAX_LINE_SIZE * sizeof(char));
    


	return 0;
}


void printaHistorico(){
	char *aux;
	char aux2[MAX_LINE_SIZE];
	char *string;
	for(int i = 0; i < tar ; i++){
		if(tarefas[i]->status == 1);
		else{
			if(tarefas[i]->status == 2) aux = "concluída";
			else if(tarefas[i]->status == 3) aux = "max inatividade";
			else if(tarefas[i]->status == 4) aux = "max execução";
			else if(tarefas[i]->status == 5) aux = "killed";
			sprintf(aux2,"#%d, %s: %s\n", i+1,aux,tarefas[i]->tarefa);
			int length = strlen(aux2);
			write(fd_sv_cl_write,aux2,length);
		}
	}

}

void printaTarefasEmExecucao(){
	char aux[MAX_LINE_SIZE];
	for(int i = 0; i < tar ; i++){
		if(tarefas[i]->status == 1) {
			sprintf(aux,"#%d : %s\n", i+1,tarefas[i]->tarefa);
			//strcat(string,aux);
			write(fd_sv_cl_write,aux,strlen(aux));
		}
	}

}



int interpreter(char *line){
	int r = 1;
	//char *aux = malloc(strlen(line) * sizeof(char));
	//strcpy(aux,line);
    char *string = strtok(line," ");
    int pid;

    if(strcmp(string,"tempo-inatividade") == 0 || strcmp(string,"-i") == 0){
        char *a = strtok(NULL," ");
        if(a && isdigitSTR(a)) {
        	setTimeInactivity(atoi(a));
        }
        write(fd_sv_cl_write,EXIT,sizeOfExit);
    }
    else if(strcmp(string,"tempo-execucao") == 0 || strcmp(string,"-m") == 0){
        char *a = strtok(NULL," ");
        if(a && isdigitSTR(a)) {
        	setTimeExecution(atoi(a));
        }
        write(fd_sv_cl_write,EXIT,sizeOfExit);
	}
	else if(strcmp(string,"-e") == 0 || strcmp(string,"exec") == 0){
		string = strtok(NULL,"\0");
		
		if(string != NULL){
			if((pid = fork()) == 0){
				dup2(fd_sv_cl_write,1);
				close(fd_sv_cl_write);

				exec_pipe(string);
			// tarefa concluida
				kill(getppid(),SIGUSR1);
				tarefaTerminada();
				_exit(0);
			
			}else{ // inicialização da tarefa
				if(tarefas[tar]){
					tarefas[tar]->pidT = pid;
					tarefas[tar]->status = 1;
					tarefas[tar]->tarefa = calloc(strlen(string),sizeof(char));
					strcpy(tarefas[tar++]->tarefa,string);
				}
				else { // realloc do array
					realloc_tarefa();
					tarefas[tar]->pidT = pid;
					tarefas[tar]->status = 1;
					tarefas[tar]->tarefa = calloc(strlen(string),sizeof(char));
					strcpy(tarefas[tar++]->tarefa,string);
				}
			}
		}
		else{
			write(fd_sv_cl_write,EXIT,sizeOfExit);
		}

	}
	
	else if(strcmp(string,"listar") == 0 || strcmp(string,"-l") == 0) {
		printaTarefasEmExecucao();
		write(fd_sv_cl_write,EXIT,sizeOfExit);

	}
	
	else if(strcmp(string,"terminar") == 0 || strcmp(string,"-t") == 0){
		char *a = strtok(NULL,"\0");
		if(a && isdigitSTR(a)){
			terminaTarefa(atoi(a));
		}
		write(fd_sv_cl_write,EXIT,sizeOfExit);
	}
	else if(strcmp(string,"historico") == 0 || strcmp(string,"-r") == 0){
		printaHistorico();
		write(fd_sv_cl_write,EXIT,sizeOfExit);
	}
	else if(strcmp(string,"ajuda") == 0 || strcmp(string,"-h") == 0){
		int save = dup(1);
		dup2(fd_sv_cl_write,1);
		printaAjuda();
		dup2(save,1);
	}
	else if(strcmp(string,"output") == 0 || strcmp(string,"-o") == 0){
		char *a = strtok(NULL,"\0");
		if(a && isdigitSTR(a)){
			printaOutput(atoi(a)-1);
		}
		write(fd_sv_cl_write,EXIT,sizeOfExit);
	}
    else {
    	r = 0;
    	write(fd_sv_cl_write,EXIT,sizeOfExit);
    }

	return r;
}



int main(int argc, char* argv[]){
	init_tarefa();
	tar = 0;
	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,signIntHandler);
	signal(SIGUSR2,execution_timeHandler);
	signal(SIGQUIT,sigQuitInactivity);
	char buf[MAX_LINE_SIZE];
	int bytes_read;
	int pid;
	int status;
	int fd_logs;

	if((fd_logs = open("logs.txt",O_CREAT | O_TRUNC,0666)) == -1){
		perror("open logs");
		return -1;
	}
	close(fd_logs);
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
		buf[bytes_read] = '\0';
		interpreter(buf);
    	bzero(buf, MAX_LINE_SIZE * sizeof(char));
	}

	close(fd_cl_sv_read);
	close(fd_cl_sv);
	close(fd_sv_cl);
	close(fd_sv_cl_write);

}