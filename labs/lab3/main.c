/*
* Description: <write a brief description of your lab>
*
* Author: Jay Shin
*
* Date: 10/9/2020
*
* Notes:
* 1. <add notes we should consider when grading>
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/*-----------------------------Program Main----------------------------------*/
int main() {
	/* Main Function Variables */
	char *copy;
	char *token;
	size_t len = 0;
	size_t lineNum = 0;
	/* Allocate memory for the input buffer. */
	char *line = NULL;

	/*main run loop*/
	while(1){
		/* Print >>> then get the input string */
		printf(">>>");
		lineNum = getline(&line, &len, stdin);
		line[strlen(line) - 1] = '\0';
		copy = line;
		/* Tokenize the input string */
		token = strtok_r(copy, ";", &copy);
		/* Display each token */
		if (lineNum > 1) {
		/* If the user entered <exit> then exit the loop */
			if (strcmp(line, "lfcat") == 0) {
				FILE *opf = freopen("output.txt", "w+", stdout);
				lfcat();
				free(line);
				fclose(opf);
				return 0;
			}
			else printf("Error: Unrecognized command!\n");
		}
	}
	/*Free the allocated memory*/
	free(line);
	return 0;
}
/*-----------------------------Program End-----------------------------------*/
