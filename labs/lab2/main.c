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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/
int main() {
	/* Main Function Variables */
	int count = 0;
	char *copy;
	char *token;
	size_t len = 0;
	ssize_t lineNum = 0;
	/* Allocate memory for the input buffer. */
	char *line = NULL;

	/*main run loop*/
	while(1){
		/* Print >>> then get the input string */
		printf(">>>\t");
		lineNum = getline(&line, &len, stdin);
		line[strlen(line) - 1] = '\0';
		copy = line;
		/* Tokenize the input string */
		token = strtok_r(copy, " ", &copy);
		/* Display each token */
		if (lineNum > 1) {
			printf("\n");
		/* If the user entered <exit> then exit the loop */
			if (strcasecmp(line, "exit") == 0) {
				free(line);
				return 0;
			}
			while(token){
				printf("T%d:\t%s\n", count++, token);
				token = strtok_r(NULL, " ", &copy); //getting next one
			}
			count = 0;
		}
	}
	/*Free the allocated memory*/

	return 0;
}
/*-----------------------------Program End-----------------------------------*/
