#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_LENGTH_COMMAND 1024
#define MAX_LENGTH_PATH 1024
#define CLEAR_SCREEN printf("\e[1;1H\e[2J")
#define SPLITER " \t\r\n"
#define SHELL_PATH ".shell_path"

char **directoriesFromShellPath;

/*
  Return a pointer to a copy of the source
*/
char *copy(const char *source) {
  char *sink = malloc((sizeof (char)) * strlen(source) + 1);
  strcpy(sink, source);
  
  return sink;
}

/*
  Return a pointer with the source appended with 'append'
  The join is with a '/' (if the final character of source is
  not a '/')

*/
char *appendPath(const char *source,
                 const char *append) {
  int offset = 0;
  int lenSource = strlen(source);
  if (lenSource) {
    offset = (source[lenSource - 1] != '/');
  }
  
  char *sink = malloc((sizeof (char)) * (lenSource + strlen(append) + offset));
  
  strcpy(sink, source);
  if (offset) {
    strcat(sink, "/");    
  }
  strcat(sink, append);
  
  return sink;
}

/*
  Return the number of substrings separated by the spliter
*/
int lenSubstrings(const char *source,
                  const char *spliter) {
  char *flag = copy(source);

  int ret = 0;
  
  char *f = strtok(flag, spliter);
  while(f != NULL) {
    ret++;
    f = strtok(NULL, spliter);
  }
  free(flag);
  
  return ret;
}

/*
  Return a array of strings with all substrings that are
  generated from a split in source, using the spliter
  OBS: The last position is filled with NULL
*/
char **splitSubstrings(const char *source,
                       const char *spliter) {
  char *flag = copy(source);

  int lenSubstrings_ = lenSubstrings(source, spliter);
  char **sink = malloc((sizeof (char*)) * lenSubstrings_ + 1);
  
  int counter = 0;
  
  char *f = strtok(flag, spliter);
  while(f != NULL) {
    sink[counter++] = copy(f);
    f = strtok(NULL, spliter);
  }
  free(flag);

  sink[lenSubstrings_] = NULL;
  
  return sink;
}

/*
  Return 1 if the directory has the file, 0 if not
*/
int findFileInDirectory(const char *file,
                        const char *directory) {
  DIR *directory_ = opendir(directory);
  if (directory_ == NULL) {
    return 0;
  }

  struct dirent *dir;
  while ((dir = readdir(directory_)) != NULL) {
    if (strcmp(dir->d_name, file) == 0) {
      return 1;
    }
  }
  
  return 0;
}

/*
  Return the index of the directoryPath, if the file if found,
  -1 if not
*/
int findFileInDirectories(const char *file,
                          char **directoriesPath) {
  int counter = 0;
  while (directoriesPath[counter] != NULL) {
    int found = findFileInDirectory(file, directoriesPath[counter]);
    if (found) {
      return counter;
    }
    counter++;
  }
  return -1;
}

/*
  Return the number of lines in the file
  OBS: Preserves the state of the file
*/
int lenLines(FILE *file) {
  long preservedPosition = ftell(file);
  fseek(file, 0, SEEK_SET);
  
  int ret = 0;
  
  char c;
  for(c = getc(file); c != EOF; c = getc(file)) {
    ret += (c == '\n');
  }
  fseek(file, preservedPosition, SEEK_SET);
  
  return ret;
}

/*
  Return a array o chars containing all lines from source
*/
char **getLines(const char *source) {
  FILE *file = fopen(source, "r");

  int lenLines_ = lenLines(file);
  char **sink = malloc((sizeof (char*)) * lenLines_);

  fseek(file, 0, SEEK_SET);  
  char *flag = malloc((sizeof (char)) * MAX_LENGTH_PATH);
  int line;
  for (line = 0; line < lenLines_; line++) {
    fscanf(file, "%s", flag);
    sink[line] = copy(flag);
  }
  free(flag);

  fclose(file);

  return sink;
}

/*
  Parse the commands
*/
void run(const char *commands) {
  const int quantityArguments = lenSubstrings(commands, SPLITER);

  if (!quantityArguments) {
    return;
  }
  
  char **argsSeparated = splitSubstrings(commands, SPLITER);
  
  if(strcmp(argsSeparated[0], "exit") == 0) {
    printf("Bye!\n");
    exit(0);
  }
  else if(strcmp(argsSeparated[0], "cd") == 0) {
    if(argsSeparated[1] == NULL) {
      fprintf(stderr, "cd missing argument\n");
    }
    else {
      chdir(argsSeparated[1]);
    }
  } else if(strcmp(argsSeparated[0], "clear") == 0) {
    CLEAR_SCREEN;
  } else {
    int forkId = fork();
    if(forkId != 0) {
      wait(NULL);
    }
    else {
      int indexDirectory = findFileInDirectories(argsSeparated[0],
                                                 directoriesFromShellPath);
      if(indexDirectory == -1) {
        fprintf(stderr, "command not found\n");
        exit(0);
      }
      char *absolutePathCommand = appendPath(directoriesFromShellPath[indexDirectory],
                                             argsSeparated[0]);
      execv(absolutePathCommand, argsSeparated);
    }
  }
  
  free(argsSeparated);
}

/*
  Create a file with the name 'SHELL_PATH' and put some
  paths inside
*/
void createShellPath() {
  FILE *file = fopen(SHELL_PATH, "w");
  if(file == NULL) {
    fprintf(stderr, "error when creating file '%s'\n", SHELL_PATH);
    exit(1);
  }

  fputs("/bin/\n", file);
  fputs("/usr/bin/\n", file);
  
  fclose(file);
}

/*
  Create the file SHELL_PATH, if doesn't exists
*/
void init() {
  FILE *file = fopen(SHELL_PATH,"r");

  if(file == NULL) {
    createShellPath();
  }

  fclose(file);

  directoriesFromShellPath = getLines(SHELL_PATH);
}

int main() {
  init();
  
  char line[MAX_LENGTH_COMMAND];
  while(1) {
    printf("$> ");

    if(!fgets(line, MAX_LENGTH_COMMAND, stdin)) {
      exit(0);
    }

    run(line);
  }
  
  return 0;
}
