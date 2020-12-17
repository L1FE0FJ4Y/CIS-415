#include "command-1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 256
#define SEPARATORS " \t\n"
#define NEWCOMMAND ";"
/*---------------------------------------------------------------------------*/
/*-----------------------------Program Main----------------------------------*/
char *command[] = {
  "ls",
  "pwd",
  "mkdir",
  "cd",
  "cp",
  "mv",
  "rm",
  "cat"
};

void runCommand(char *args[]) {
	int i;
	if (args[0] && !args[3]) {
		for (i = 0; i < 8; i++) {
			if (strcmp(args[0], command[i]) == 0) {
				break;
			}
		}
		switch (i) {

			case 0:
				if (!args[1]) {
					listDir();
				} else printf("Error! Too many arguments.\n");
				break;

			case 1:
				if (!args[1]) {
					showCurrentDir();
				} else printf("Error! Too many arguments.\n");
				break;

			case 2:
				if (!args[2]) {
					makeDir(args[1]);
				} else printf("Error! Too many arguments.\n");
				break;

			case 3:
				if (!args[2]) {
					changeDir(args[1]);
				} else printf("Error! Too many arguments.\n");
				break;

			case 4:
				if (!args[3]) {
					copyFile(args[1], args[2]);
				} else printf("Error! Too many arguments.\n");
				break;

			case 5:
				if (!args[3]) {
					moveFile(args[1], args[2]);
				} else printf("Error! Too many arguments.\n");
				break;

			case 6:
				if (!args[2]) {
					deleteFile(args[1]);
				} else printf("Error! Too many arguments.\n");
				break;

			case 7:
				if (!args[2]) {
					displayFile(args[1]);
				} else printf("Error! Too many arguments.\n");
				break;

			default : printf("Error! Invalid command\n");
		}
	}
}

int tokenize(char *str, char *tokens[], char *type) {
	char *token = strtok(str, type);
	int count = 0;
	while (token) {
		tokens[count++] = token;
		token = strtok(NULL, type);
	}
	free(token);
	return count;
}

void execution(char *line) {

	char ** tokens = malloc(MAX * sizeof(char*));
  char ** args = malloc(5 * sizeof(char*));
	char * arg;
	int lim, count = 0;


	lim = tokenize(line, tokens, NEWCOMMAND);
	for (int i = 0; i < lim; i++) {
		arg = strtok(tokens[i], SEPARATORS);
		while (arg) {
			if (count == 3) {
				printf("Error! Too many arguments.\n");
				break;
			}
			args[count++] = arg;
			arg = strtok(NULL, SEPARATORS);
		}
		runCommand(args);
		free(args);
	}
	free(arg);
	free(tokens);
}

int main (int argc, char ** argv)
{
	FILE *ipf;
	FILE *opf;
	size_t len = 0;
	size_t lineNum = 0;
	char *line = NULL;

	if (argc == 1) {
		while (1) {
	    printf(">>>\t");
	    lineNum = getline(&line, &len, stdin);
	    if (lineNum >= 0) {
				execution(line);
			}
		}
	} else if (argc == 3 && strcmp(argv[1], "-f")) {
		ipf = fopen(argv[2], "r");
		opf = freopen("output.txt", "w", stdout);
		if (ipf == NULL) {
			exit(EXIT_FAILURE);
		}
		while(getline(&line, &len, ipf) != EOF) {
			execution(line);
		}
	}

	free(line);
	fclose(ipf);
	fclose(opf);
  exit(EXIT_SUCCESS);
}
