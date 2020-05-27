#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

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
			printf("%d\n",k);
	}
	}
	return buf;
}

int main(int argc, char* argv[]){
	char* b = createBuf(argc, argv);
	printf("%s\n", b);
}