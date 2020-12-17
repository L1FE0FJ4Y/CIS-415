#include "command-1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX 1024

void listDir(){
  char cwd[MAX];
  DIR* dirPtr = NULL;
  struct dirent *dePtr;
  getcwd(cwd, MAX);
  if (cwd) {
    dirPtr = opendir(cwd);
    if (dirPtr) {
      while ((dePtr = readdir(dirPtr)) != NULL) {
        write(1, dePtr->d_name, strlen(dePtr->d_name));
        write(1, " ", 1);
      }
      closedir(dirPtr);
    } else {write(1, "Error!", strlen("Error!"));}
  }
  write(1, "\n", 1);
  }

void showCurrentDir(){
  char cwd[MAX];
  getcwd(cwd,MAX);
  if (cwd) {
    write(1, cwd, strlen(cwd));
  } else   write(1, "Error!", strlen("Error!"));
  write(1, "\n", 1);
}

void makeDir(char *dirName){
  if(mkdir(dirName, 0755)){write(1, "Error!", strlen("Error!"));}
  write(1, "\n", 1);
}

void changeDir(char *dirName){
    if (dirName == NULL) {
      write(1, "Error!", strlen("Error!"));
    } else chdir(dirName);
    write(1, "\n", 1);
}

void copyFile(char *sourcePath, char *destinationPath){
  int in, out, ret;
  char buf[MAX];

  in = open(sourcePath, O_RDONLY);
  out = open(destinationPath, O_WRONLY|O_CREAT, 0666);

  while (ret = read(in,buf, MAX)) {
    write(out, buf, ret);
  }
  close(in);
  close(out);
  write(1, "\n", 1);
}

void moveFile(char *sourcePath, char *destinationPath){
  int in, out, ret;
  char buf[MAX];

  in = open(sourcePath, O_RDONLY);
  out = open(destinationPath, O_RDONLY|O_CREAT, 0666);

  while (ret = read(in,buf, MAX)) {
    write(out, buf, ret);
  }
  unlink(sourcePath);
  close(in);
  close(out);
  write(1, "\n", 1);
}

void deleteFile(char *filename){
  if(unlink(filename)){
    write(1, "Error!", strlen("Error!"));
    write(1, "\n", 1);
  }
}

void displayFile(char *filename){
  char buf[MAX];
  int ipf = 0;
  int ret = 0;

  ipf = open(filename, 0);
  while ((ret = read(ipf, buf, MAX)) != 0) {
    write(1, buf, MAX);
  }
  close(ipf);
  write(1, "\n", 1);
}
