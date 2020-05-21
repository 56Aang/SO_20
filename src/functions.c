#include "include/functions.h"

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