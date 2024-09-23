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

/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <regex.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>


#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
#define MAXFILENAME 1024

}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE GREATGREAT LESS AMPERSAND PIPE GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"
#include <cstring>
#include <cassert>
#include <algorithm>


void yyerror(const char * s);
void expandWildcardsIfNecessary(std::string * arg);
void expandWildcard(char * prefix, char * suffix);
bool cmpfunction (char * i, char * j);

int yylex();
static std::vector<char *> sort = std::vector<char *>();

void yyerror(const char * s);
int yylex();

%}

%%

goal: command_list;

arg_list:
  arg_list WORD {
      //printf("   Yacc: insert argument \"%s\"\n", $2->c_str());
      //Command::_currentSimpleCommand->insertArgument( $2 );
      expandWildcardsIfNecessary( $2 );
    }

  | /*empty string*/
  ;

cmd_and_args:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  arg_list  {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }

  ;

pipe_list:
  cmd_and_args
  | pipe_list PIPE cmd_and_args
  ;

io_modifier:
  GREATGREAT WORD {
    if (Shell::_currentCommand._outFile != NULL) {
      printf("Ambiguous output redirect.\n");
      exit(0);
    }
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._append = true;
  }
  | GREAT WORD {
      if (Shell::_currentCommand._outFile != NULL) {
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      Shell::_currentCommand._outFile = $2; 
  }
  | GREATGREATAMPERSAND WORD {
      if (Shell::_currentCommand._outFile != NULL) {
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._append = true;
      Shell::_currentCommand._errFile = $2;
  }

  | GREATAMPERSAND WORD {
      if (Shell::_currentCommand._outFile != NULL) {
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = $2;
  }
  | LESS WORD {
      if (Shell::_currentCommand._inFile != NULL) {
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      Shell::_currentCommand._inFile = $2;
  }
  | TWOGREAT WORD {
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      Shell::_currentCommand._errFile = $2;
  }
  ;

io_modifier_list:
  io_modifier_list io_modifier
  | /*empty*/
  ;

background_optional:
  AMPERSAND {
    Shell::_currentCommand._background = true;
  }
  | /*empty*/
  ;

command_line:
  pipe_list io_modifier_list background_optional NEWLINE{
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE /*accept empty cmd line*/
  | error NEWLINE { yyerrok; }
  ; /*error recovery*/

command_list :
  command_line |
  command_list command_line
  ;/* command loop*/



%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

bool compare(char * i, char * j) {
  bool flag = strcmp(i, j) < 0;
  return flag;
}

void expandWildcardsIfNecessary(std::string * arg) {
  // Return if arg doesn't contain '*' or '?'
  char *c_arg = (char *)arg->c_str();
  if ((strchr(c_arg, '*') == NULL && strchr(c_arg, '?') == NULL) || strcmp(c_arg, "${?}") == 0) {
    Command::_currentSimpleCommand->insertArgument(arg);
    return;
  }

  expandWildcard((char *) "", c_arg);

  std::sort(sort.begin(), sort.end(), compare);

  for (const auto i: sort) {
    std::string *temp = new std::string(i);
    if (strchr(temp->c_str(), '*') == NULL)
      Command::_currentSimpleCommand->insertArgument(temp);
  }

  for (const auto i: sort) {
    free(i);
  }
  sort.clear();
}


void expandWildcard(char * prefix, char * suffix) {
  if (suffix[0] == 0) {
    sort.push_back(strdup(prefix));
    prefix = NULL;
    return;
  }
  char Prefix[MAXFILENAME];
  if (prefix[0] == 0) {
    if (suffix[0] == '/') {
      suffix = suffix + 1;
      sprintf(Prefix, "%s/", prefix);
    }
    else
      strcpy(Prefix, prefix);
  }
  else
    sprintf(Prefix, "%s/", prefix);

  char * s = strchr(suffix, '/');
  char component[MAXFILENAME];
  if (s != NULL) {
    strncpy(component, suffix, s - suffix);
    component[s - suffix] = 0;
    suffix = s + 1;
  }
  else {
    strcpy(component, suffix);
    suffix = suffix + strlen(suffix);
  }
  char newPrefix[MAXFILENAME];
  if (strchr(component, '?') == NULL && strchr(component, '*') == NULL) {
    if (Prefix[0] == 0 )
      strcpy(newPrefix, component);
    else
      sprintf(newPrefix, "%s/%s", prefix, component);
    expandWildcard(newPrefix, suffix);
    return;
  }

  char * reg = (char *) malloc (2 * strlen(component) + 10);
  //char *a = c_arg;
  char *r = reg;
  *r = '^';
  int i = 0;
  r++; // match beginning of line
  while (component[i]) {
    if (component[i] == '*') {*r = '.'; r++; *r = '*'; r++;}
    else if (component[i] == '?') {*r = '.'; r++;}
    else if (component[i] == '.') {*r = '\\'; r++; *r = '.'; r++;}
    else {*r = component[i]; r++;}
    i++;
  }
  *r = '$'; r++; *r = 0;

  regex_t re;
  int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
  if (expbuf != 0) {
    perror("regcomp");
    return;
  }

  char *dir;
  if (Prefix[0] == 0)
    dir = (char *)".";
  else
    dir = Prefix;
  DIR * d = opendir(dir);
  if (d == NULL)
    return;

  struct dirent * ent;
  bool flag = false;
  while ((ent = readdir(d)) != NULL) {
    if (regexec(&re, ent->d_name, 1, NULL, 0) == 0) {
      flag = true;
      if (Prefix[0] == 0)
        strcpy(newPrefix, ent->d_name);
      else
        sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
      if (reg[1] == '.') {
        if (ent->d_name[0] != '.')
          expandWildcard(newPrefix, suffix);
      }
    else
      expandWildcard(newPrefix, suffix);
    }
  }
  if (!flag) {
    if (Prefix[0] != 0)
      sprintf(newPrefix, "%s/%s", prefix, component);
    else
      strcpy(newPrefix, component);
    expandWildcard(newPrefix, suffix);
  }
  closedir(d);
  regfree(&re);
  free(reg);
  reg = NULL;
}


#if 0
main()
{
  yyparse();
}
#endif
