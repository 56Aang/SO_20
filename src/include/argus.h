#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EXIT "//exit"
#define sizeOfExit 6

#define MAX_LINE_SIZE 4096