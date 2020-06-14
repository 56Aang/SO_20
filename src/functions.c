#include "include/functions.h"

int isdigitSTR(char *buffer){
	int i = 0;
	if(buffer[0] && buffer[0] == '-') i++;
	for(; buffer[i]; i++){
		if(!isdigit(buffer[i]) && buffer[i] != '\0') {
			return 0;
		}
	}
	return 1;
}
