#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void script_print (pid_t* pid_ary, int size) {
  FILE* fout;fout = fopen ("top_script.sh", "w");
  fprintf(fout, "#!/bin/bash\ntop");
  for (int i = 0; i < size; i++) {
    fprintf(fout, " -p %d", (int)(pid_ary[i]));
  }
  fprintf(fout, "\n");
  fclose (fout);
}

void signaler (pid_t *childPids, int size, int signum) {
  sleep(2);
  for (int i = 0; i < size; i++) {
    printf("Parent process: %d - Sending signal: %d to child process: %d\n", getpid(), signum, childPids[i]);
    kill(childPids[i], signum);
  }
}

int main(int argc, char const *argv[]) {
  int n = atoi(argv[1]);
  pid_t childPid;
  pid_t childPids[n];
  char *inp[] = { "./iobound", "-seconds", "5", NULL };

  sigset_t set;
  int ret;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigprocmask(SIG_BLOCK, &set, NULL);

  for (int i = 0; i < n; i++) {
    childPid = fork();
    childPids[i] = childPid;
    if (childPid == 0) {
      printf("Child Process: %d -Waiting for SIGUSR1...\n", getpid());
      sigwait(&set, &ret);
      sleep(1);
      printf("Child Process: %d -Received signal: SIGUSR1 - Calling exec()\n", getpid());
      execvp(inp[0], inp);
    }
  }
  script_print(childPids, n);

  signaler(childPids, n, SIGUSR1);
  signaler(childPids, n, SIGSTOP);
  sleep(5);
  signaler(childPids, n, SIGCONT);
  sleep(3);
  signaler(childPids, n, SIGINT);

  return 0;
}
