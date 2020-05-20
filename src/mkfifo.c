#include <stdio.h>
#include <sys/stat.h>

int main(){
	if (mkfifo("fifo-sv-cl",0666) == -1){
		perror("mkfifo");
	}

	if (mkfifo("fifo-cl-sv",0666) == -1){
		perror("mkfifo");
	}
	
	return 0;
}