/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.hh"
#include "command.hh"

int yyparse(void);
void yyrestart(FILE * file);

void Shell::prompt() {
  if(isatty(0)) {
    if (getenv("PROMPT"))
      printf(getenv("PROMPT"));
    else
      printf("myshell>");
  }
  fflush(stdout);
}

// Handling Ctrl-C
extern "C" void dispCtrlC(int sig) {
  fprintf(stderr, "\nsig: %d     Ouch!\n", sig);
  if (sig == SIGCHLD)
    while(waitpid(-1, NULL, WNOHANG) > 0); 
  else
    Shell::prompt();
  //Shell::prompt();
}

// Zombie eliminations
extern "C" void dispZombies(int sig) {
  int pid = wait3(0, 0, NULL);
  //int flag = false;
  for (int i = 0; i < 1000; i++) {
    if (Shell::_currentCommand.bglist[i] == pid)
      printf("[%d] exited.\n", pid);
  }

  if (sig == SIGCHLD)
    while(waitpid(-1, NULL, WNOHANG) > 0); 
  else
    Shell::prompt();
}




int main(int argc, char ** argv) {
  // Ctrl-C Handle
  struct sigaction sa;
  sa.sa_handler = dispCtrlC;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &sa, NULL)) {
    perror("sigaction");
    exit(-1);
  }

  // Zombie Handle
  
  struct sigaction sz;
  sz.sa_handler = dispZombies;
  sigemptyset(&sz.sa_mask);
  sz.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sz, NULL)) {
    perror("sigaction");
    exit(-1);
  }
  

  char path[500];
  realpath(argv[0], path);
  setenv("SHELL", path, 1);

  // Extra credit for 2.7
  FILE *fp = fopen(".shellrc", "r");
  if (fp) {
    yyrestart(fp);
    yyparse();
    yyrestart(stdin);
    fclose(fp);
  }
  else
    Shell::prompt();

  yyparse();
}

Command Shell::_currentCommand;
std::string Shell::value_arg;
