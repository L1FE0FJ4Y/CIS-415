#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
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

int main(int argc, char const *argv[]) {
  int n = atoi(argv[1]);
  pid_t childPid;
  char *inp[] = { "./iobound", "-seconds", "5", NULL };
  pid_t childPids[n];
  int pidNum;

  for (int i = 0; i < n; i++) {
    childPid = fork();
    childPids[i] = childPid;
    if (childPid == 0) {
      execvp(inp[0], inp);
    }
  }
  script_print(childPids, n);
  wait(0);
  sleep(1);
  return 0;
}
