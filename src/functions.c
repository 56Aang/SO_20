#include "include/functions.h"

int isdigitSTR(char *buffer){
	for(int i = 0; buffer[i]; i++){
		if(!isdigit(buffer[i]) && buffer[i] != '\0') {
			return 0;
		}
	}
	return 1;
}
