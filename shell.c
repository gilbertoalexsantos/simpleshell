#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LENGTH 1024
#define LIM " \t\r\n"

const char *path = "/bin/";

int count_args(char *arg) {
  char flag[strlen(arg) + 1];
  strcpy(flag,arg);

  int ret = 0;
  char *f = strtok(flag,LIM);
  while(f != NULL) {
    ret++;
    f = strtok(NULL,LIM);
  }

  return ret;
}

void parse(char *source, int size, char *sink[]) {
  int ct = 0;
  char *f = strtok(source,LIM);
  while(f != NULL) {
    sink[ct] = malloc(sizeof(char[strlen(f) + 1]));
    strcpy(sink[ct],f);
    f = strtok(NULL,LIM);
    ct++;
  }
}

void run(char *cmds) {
  int qt_args = count_args(cmds);
  char *args[qt_args + 1];
  parse(cmds,qt_args,args);
  args[qt_args] = NULL;

  if(strcmp(args[0],"exit") == 0)
    exit(0);

  int pip = fork();

  if(pip != 0) {
    wait(NULL);
  } else {
    char program[MAX_LENGTH];
    strcpy(program,path);
    strcat(program,args[0]);
    int rv = execv(program,args);
    exit(0);
  }
}

static int qt_calls = 0;

int main() {
  char line[MAX_LENGTH];
  while(1) {
    printf("$> ");

    if(!fgets(line,MAX_LENGTH,stdin))
      exit(0);

    run(line);
  }
  return 0;
}
