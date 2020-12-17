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
int main(int argc, char* argv[]) {
	/* Main Function Variables */
	FILE *opf;
	opf = fopen("output.txt", "w+");

	if (argc < 2){
		fprintf(opf, "Enter an input file.");
		fclose(opf);
		exit(EXIT_FAILURE);
	}
	FILE *ipf = fopen(argv[1],"r");
	if(ipf == NULL){
		fprintf(opf, "Not valid file.\n");
		fclose(opf);
		exit(EXIT_FAILURE);
	}

	int count = 0;
	char *copy;
	char *token;
	size_t len = 0;
	ssize_t lineNum = 0;
	/* Allocate memory for the input buffer. */
	char *line = NULL;
	/*main run loop*/
	lineNum = getline(&line, &len, ipf);
	while(lineNum >= 0){
		copy = line;
		/* Tokenize the input string */
		token = strtok_r(copy, " ", &copy);
		/* Display each token */
		if (lineNum > 1) {
			while(token){
				fprintf(opf, "T%d:\t%s\n", count++, token);
				token = strtok_r(NULL, " ", &copy); //getting next token
			}
			count = 0;
		}
		lineNum = getline(&line, &len, ipf); //getting next line
	}
	/*Free the allocated memory*/
	free(line);
	fclose(ipf);
	fclose(opf);
	return EXIT_SUCCESS;
}
/*-----------------------------Program End-----------------------------------*/
