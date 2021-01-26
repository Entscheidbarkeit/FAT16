#include <stdint.h>

char nameBuf[13];

char *sanitizeName(char *string) {
	uint8_t notSpace = -1;
	int i = 0;
	for(int i = 0; i < 8; i++){
		nameBuf[i] = string[i];
		if(string[i] != ' ') notSpace = i;
	}
	i = notSpace + 1;
	if(string[8] != ' '){
		nameBuf[i++] = '.';
		for(int j = 8; j < 11; j++){
			if(string[j] != ' '){
				nameBuf[i++] = string[j];
			}
		}
	}

	nameBuf[i++] = '\0';
	return nameBuf;
}
