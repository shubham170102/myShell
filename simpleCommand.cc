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
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>

#include <iostream>
#include <unistd.h>


#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  _arguments.push_back(argument);

  /*
  char *c_arg = strdup(argument->c_str());
  char *dollar_found = strchr(c_arg, '$');
  if (dollar_found == NULL) {
    return;
  }

  char *str1 = dollar_found + 1;
  char *str2 = str1 + 1;
  char *name = (char *) malloc(sizeof(char *) * 1024);
  char *temp = name;
  while(*str2 != '}' && *str2) {
    *temp = *str2;
    temp++;
    str2++;
  }
  *temp = '\0';

  char *value;
  if (!strcmp(name, "$")) {
    pid_t pid = getpid();
    value = (char *) malloc(16);
    sprintf(value, "%d", pid);
  }
  else if (!strcmp(name, "?")) {
    int code = Shell::_returnCode;
    value = (char *) malloc(16);
    sprintf(value, "%d", code);
  }
  else if (!strcmp(name, "!")) {
    int pid = Shell::_lastBackground;
    value = (char *) malloc(16);
    sprintf(value, "%d", pid);
  }
  */

}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
