#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "macros.h"
#include <ctype.h> // isdigit()

int isdigitSTR(char *buffer);
int readlinha(int fd, char * buffer, int nbyte);
ssize_t readln (int fd, char *buffer, size_t size);