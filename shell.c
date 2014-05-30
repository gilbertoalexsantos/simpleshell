#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_LENGTH 1024
#define MAX_LENGTH_PATH 100
#define CLEAR_SCREEN printf("\e[1;1H\e[2J");
#define LIM " \t\r\n"

char **path;
int size_path;

// Count the number of substrings separeted by the constant LIM
int count_args(const char *arg) {
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

// Put the arguments on sink
void parse(const char *source, int size, char *sink[]) {
  char flag[strlen(source) + 1];
  strcpy(flag,source);

  int ct = 0;
  char *f = strtok(flag,LIM);
  while(f != NULL) {
    sink[ct] = (char*) malloc(sizeof(char[strlen(f) + 1]));
    strcpy(sink[ct],f);
    f = strtok(NULL,LIM);
    ct++;
  }

  sink[size] = NULL;
}

// Return the index of the correspondent path to the cmd, if doesn't exist,
// return -1
int get_file_path(const char *cmd) {
  int i = 0;
  for(i = 0; i < size_path; i++) {
    DIR *d;
    struct dirent *dir;
    d = opendir(path[i]);
    if(d == NULL)
      continue;
    while((dir = readdir(d)) != NULL) {
      if(strcmp(dir->d_name,cmd) == 0)
        return i;
    }
  }
  return -1;
}

void run(const char *cmds) {
  int qt_args = count_args(cmds);
  char *args[qt_args + 1];
  parse(cmds,qt_args,args);
  if(qt_args == 0)
    return;

  if(strcmp(args[0],"exit") == 0)
    exit(0);
  else if(strcmp(args[0],"cd") == 0) {
    if(args[1] == NULL)
      fprintf(stderr,"cd missing argument.\n");
    else
      chdir(args[1]);
  } else if(strcmp(args[0],"clear") == 0) {
    CLEAR_SCREEN;
  } else {
    int pid = fork();
    if(pid != 0)
      wait(NULL);
    else {
      char program[MAX_LENGTH];
      int p = get_file_path(args[0]);
      if(p == -1) {
        fprintf(stderr,"command not found!\n");
        exit(0);
      }
      strcpy(program,path[p]);
      strcat(program,args[0]);
      execv(program,args);
    }
  }
}

// Create the file .shell_path
void create_shell_path() {
  FILE *fp;
  fp = fopen(".shell_path","w");
  if(fp == NULL) {
    fprintf(stderr,"error when creating file '.shell_path'\n");
    exit(1);
  }
  fputs("/bin/\n",fp);
  fputs("/usr/bin/\n",fp); 
  fclose(fp);
}

// Count the unmber of lines in a file
int count_lines(FILE *f) {
  char c;
  int ret = 0;
  for(c = getc(f); c != EOF; c = getc(f))
    ret += (c == '\n');
  return ret;
}

// Transfer the directorys in .shell_path to the matrix path
void init() {
  FILE *fp;
  fp = fopen(".shell_path","r");

  if(fp == NULL)
    create_shell_path();

  size_path = count_lines(fp);
  fseek(fp,0,SEEK_SET);
  path = (char**) malloc(size_path * sizeof(char**));
  for(int i = 0; i < size_path; i++) {
    path[i] = (char*) malloc(sizeof(char[MAX_LENGTH_PATH]));
    fscanf(fp,"%s",path[i]);
  }

  fclose(fp);
}

int main() {
  init();
  char line[MAX_LENGTH];
  while(1) {
    printf("$> ");

    if(!fgets(line,MAX_LENGTH,stdin))
      exit(0);

    run(line);
  }
  return 0;
}
