#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX 1024

void lfcat(void) {
  char cwd[MAX];
  DIR* dirPtr;
  struct dirent *dePtr;

  if (getcwd(cwd, MAX) == NULL) {
    write(1, "Error getting cwd\n", strlen("error getting cwd\n"));
  }
  dirPtr = opendir(cwd);
  if (dirPtr == NULL) {
    write(1, "Error opening directory\n", strlen("error opening directory\n"));
  }

  char* banName[] = {"pseudu-shell", "a.out", "command.c", "command.h", "command.o", "main.c", "main.o", "output.txt"};
  int i = 0;
  size_t len = 0;
	ssize_t lineS;
	char *line = NULL;
  char *dName = NULL;
  FILE *ipf = NULL;

  while ((dePtr = readdir(dirPtr)) != NULL) {
    dName = dePtr->d_name;
    int flag = 1;
    for (i = 0; i < 8; i++) {
      if (strcmp(dName, banName[i]) == 0) {
        flag = 0;
      }
    }
    if (flag) {
      write(1, "File: ", strlen("File: "));
      write(1, dName, strlen(dName));
      write(1, "\n", 1);
      ipf = fopen(dName, "r");
      lineS = getline(&line, &len, ipf);
      while (lineS >= 0) {
        write(1, line, lineS);
        lineS = getline(&line, &len, ipf);
      }
      write(1, "\n----------------------------------------", strlen("\n----------------------------------------"));
      write(1, "----------------------------------------\n", strlen("----------------------------------------\n"));
      fclose(ipf);
    }
  }
  free(line);
  closedir(dirPtr);
}
