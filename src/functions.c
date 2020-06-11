#include "include/functions.h"

int isdigitSTR(char *buffer){
	for(int i = 0; buffer[i]; i++){
		if(!isdigit(buffer[i]) && buffer[i] != '\0') {
			return 0;
		}
	}
	return 1;
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

ssize_t readln (int fd, char *buffer, size_t size) {

	int resultado = 0, i = 0;

	ssize_t read_bytes = 0;

	while ((resultado = read (fd, &buffer[i], 1)) > 0 && i < size) {
		if (buffer[i] == '\n') {
			i += resultado;
			return i;
		}

		i += resultado;
	}

	return i;
}