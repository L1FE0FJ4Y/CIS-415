#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#define SEPARATORS " \t\n"

void scheduler (pid_t *childPids, int size, int signum) {
  for (int i = 0; i < size; i++) {
    printf("Parent process: %d - Sending signal: %d to child process: %d\n", getpid(), signum, childPids[i]);
    kill(childPids[i], signum);
  }
}

void murder (pid_t childPid, int signum) {
  kill(childPid, signum);
  printf("Parent process: %d - Sending signal: %d to child process: %d\n", getpid(), signum, childPid);
}

void tokenize(char *str, char *tokens[], char *type) {
	char *token = strtok(str, type);
	int count = 0;
	while (token) {
		tokens[count++] = token;
		token = strtok(NULL, type);
	}
  tokens[count] = NULL;
	free(token);
}

int lifecheck(pid_t *childPids, int i) {
  clock_t start = clock();
  while (((clock() - start) < 1000)) {
    if (waitpid(childPids[i], 0, WNOHANG) != 0) {
      return 0;
    }
  }
  return 1;
}

void printproc(pid_t pid) {
  printf("\n=======================================================================================\n");
  char *saveproc = malloc(sizeof(char) * 100);
  sprintf(saveproc, "/proc/%d/status", pid);
  int proc = open(saveproc, 0);
  if (proc != -1) {
    char *buff = malloc(sizeof(char) * 100);
    for (int i = 0; i < 5; i++) {
      int readamount = read(proc, buff, 1);
      while (strncmp(buff, "\n", 1) != 0) {
        write(1, buff, readamount);
        readamount = read(proc, buff, 1);
      }
    }
    free(buff);
    close(proc);
  } else {
    printf("ERROR: No running process!\n");
  }
  free(saveproc);
  printf("\n=======================================================================================\n");
}

int main(int argc, char const *argv[]) {
  if (argc == 2) {
    int count = 0, size;
    FILE *ipf;
		ipf = fopen(argv[1], "r");
		if (ipf == NULL) {
			exit(EXIT_FAILURE);
		} else {
      fseek(ipf, 0, SEEK_END);
      size = ftell(ipf);
      fseek(ipf, 0, SEEK_SET);
    }
    size_t len = 0;
    size_t lineNum = 0;
    char *line = NULL;
    char ** tokens = malloc(size * sizeof(char*));

    sigset_t set;
    int ret;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    pid_t childPid;
    pid_t childPids[size];
    printf("\n=======================================================================================\n");
		while(getline(&line, &len, ipf) != EOF) {
      tokenize(line, tokens, SEPARATORS);

      childPid = fork();
      childPids[count++] = childPid;

      if (childPid < 0) {
        printf("ERROR running child process!\n");
        exit(-1);
      }
      if (childPid == 0) {
        printf("\n=======================================================================================\n");
        printf("Child Process: %d -Waiting for SIGUSR1...\n", getpid());
        printf("\n=======================================================================================\n");
        sigwait(&set, &ret);
        sleep(1);
        printf("\n=======================================================================================\n");
        printf("Child Process: %d -Received signal: SIGUSR1 - Calling exec()\n", getpid());
        printf("\n=======================================================================================\n");
        if (execvp(tokens[0], tokens) == -1) {
          printf("Error Invalid Argument\n");
          free(tokens);
          fclose(ipf);
          free(line);
          exit(EXIT_FAILURE);
        }
      }
    }

    sigset_t parentset;
    sigemptyset(&parentset);
    sigaddset(&parentset, SIGALRM);
    sigprocmask(SIG_BLOCK, &parentset, NULL);

    scheduler(childPids, count, SIGUSR1);
    scheduler(childPids, count, SIGSTOP);
    int using = 0, lim = 0;
    while(1) {
      if (using == count) using = 0;
      if (waitpid(childPids[using], 0, WNOHANG) == 0) {
        printf("\n=======================================================================================\n");
        lim = 0;
        alarm(1);
        murder(childPids[using], SIGCONT);
        if(lifecheck(childPids, using)) {
          printf("Parent Process: %d -Waiting for SIGALRM...\n", getpid());
          sigwait(&parentset, &ret);
        }
        murder(childPids[using], SIGSTOP);
      }
      if (++lim == count) break;
      using++;
      for (int i = 0; i < count; i++) {
        printf("Process : %d - Printing out PROC\n", childPids[i]);
        printproc(childPids[i]);
      }
    }
    for (int i = 0; i < count; i++) {
      printf("Process : %d - Printing out PROC\n", childPids[i]);
      printproc(childPids[i]);
    }

    free(tokens);
    fclose(ipf);
    free(line);
  } else {
    printf("Error: Run the program incorrectly!\n");
  }
  exit(EXIT_SUCCESS);
}
