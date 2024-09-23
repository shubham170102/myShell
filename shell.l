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
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <cstring>
#include "y.tab.hh"
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <fstream>
#include "shell.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}


extern "C" char * read_line();

int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
    return getc(f);
  }

  if (p == NULL || *p == 0) {
    char *s = read_line();
    p = s;
  }

  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)


%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}

"2>" {
  return TWOGREAT;
}

">>" {
  return GREATGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

\"[^\n\"]*\" {
  // Quotes
  std::string str = std::string(yytext);
  str = str.substr(1, str.size() - 2);
  yylval.cpp_string = new std::string(str);;
  return WORD;
}

\`[^\n\`]*\`|$\([^\n]*\) {
  // Subshell
  std::string string =  yytext;
  // Extracting the string b/w $(...) or `...`
  if (string.at(0) == '$')
    string = string.substr(2, string.size() - 3);
  else
    string = string.substr(1, string.size() - 2); 

  // setup input and output
  int tempin;
  int tempout;
  tempin = dup(0);
  tempout = dup(1);

  // Parent creates 2 pipes
  int pin[2];
  int pout[2];
  pipe(pin);
  pipe(pout);

  // Parent writes to pin[1]
  write(pin[1], string.c_str(), string.size());
  write(pin[1], "\n", 1);
  write(pin[1], "exit", 4);
  write(pin[1], "\n", 1);
  close(pin[1]);

  // Child redirect from pin[0] and redirect output to pout[1]
  // Child process runs subshell command and writes output to pout[1]
  dup2(pin[0], 0);
  dup2(pout[1], 1);
  close(pin[0]);
  close(pout[1]);

  // Create child process
  int ret = fork();
  if (ret == 0) {
    // Child process
    char ** args = new char* [2];
    args[0] = (char*) "/proc/self/exe";
    args[1] = NULL;
    execvp(args[0], args);
    perror("execvp for subshell");
    exit(1);
  }
  else if (ret < 0) {
    // error in fork
    perror("fork");
    exit(1);
  }
  else {
    //waitpid(ret, NULL, 0);

    dup2(tempin, 0);
    dup2(tempout, 1);
    close(tempin);
    close(tempout);

    // Read the output from the subshell
    char c;
    char * buffer = (char *) malloc (4000);
    int i = 0;

    // Parent closes pout[1], reads the output from pout[0] 
    // and writes into buffer
    while(read(pout[0], &c, 1)) {
      // Read result into buffer
      if (c == '\n')
        buffer[i++] = ' ';
      else
        buffer[i++] = c;
    }
    close(pout[0]);

    // Terminate
    buffer[i - 1] = '\0';

    // Parents puts chars in the buffer back into lex, in reverse order
    int j = 0;
    for (j = strlen(buffer); j >= 0; j--) {
      myunputc(buffer[j]);
    }
  }
}

"source "[^ \t\n|><]+ {
  std::string *str = new std::string(yytext + strlen("source "));
  std::ifstream fp;
  fp.open(*str);
  std::vector<std::string> lines;
  std::string line;

  while(std::getline(fp, line)) {
    line += "\n";
    lines.push_back(line);
  }
  fp.close();
  int index = lines.size() - 1;
  for(; index >= 0; index--) {
    for (int j = lines[index].length() - 1; j >= 0; j--)
      myunputc(lines[index][j]);
  }
}



[^ \t\n|><&]*\\[^ \t\n]* {
  // Escaping
  int i = 0;
  char *escape = (char *) malloc(100);
  char *string = yytext;

  while(*string) {
    // Evaluating everthing after "\"
    if (*string == '\\') {
      if (*(string + 1) != '\\') {
        escape[i++] = *(++string);
      }
      else {
        string = string + 1;
        escape[i++] = '\\';
      }
    }
    else {
      escape[i++] = *string;
    }
    string++;
  }
  // Null pointer
  escape[i] = '\0';
  yylval.cpp_string = new std::string(escape);
  return WORD;
}


\$\{\$\} {
  std::string str = yytext;
  std::string target = "";
  std::size_t found_dollar = str.find('$');
  while(found_dollar != -1) {
    std::string var;
    target = target + str.substr(0, found_dollar);
    if (str[found_dollar + 1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        var = str.substr(found_dollar + 2, end - found_dollar - 2);
        int pid = getpid();
        std::string s = std::to_string(pid);
        target = target + s;
        str = str.substr(end + 1, -1);
      }
    }
    found_dollar = str.find('$');
  }
  target = target + str;
  yylval.cpp_string = new std::string(target);
  return WORD;
}


\$\{\?\} {
  std::string str = yytext;
  std::string target = "";
  std::size_t found_dollar = str.find('$');
  while(found_dollar != -1) {
    std::string var;
    target = target + str.substr(0, found_dollar);
    if (str[found_dollar + 1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        var = str.substr(found_dollar + 2, end - found_dollar - 2);
        target += target + std::string(getenv(var.c_str()));
        str = str.substr(end + 1, -1);
      }
    }
    found_dollar = str.find('$');
  }
  target = target + str;
  yylval.cpp_string = new std::string(target);
  return WORD;
}

\$\{\!\} {
  std::string str = yytext;
  std::string target = "";
  std::size_t found_dollar = str.find('$');
  while(found_dollar != -1) {
    std::string var;
    target = target + str.substr(0, found_dollar);
    if (str[found_dollar + 1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        var = str.substr(found_dollar + 2, end - found_dollar - 2);
        target += target + std::string(getenv(var.c_str()));
        str = str.substr(end + 1, -1);
      }
    }
    found_dollar = str.find('$');
  }
  target = target + str;
  yylval.cpp_string = new std::string(target);
  return WORD;
}

\$\{\_\} {
  std::string str = yytext;
  std::string target = "";
  std::size_t found_dollar = str.find('$');
  while(found_dollar != -1) {
    std::string var;
    target = target + str.substr(0, found_dollar);
    if (str[found_dollar + 1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        var = str.substr(found_dollar + 2, end - found_dollar - 2);
        std::string var_new = Shell::value_arg;
        target += target + var_new;
        //target += target + std::string(getenv(var.c_str()));
        str = str.substr(end + 1, -1);
      }
    }
    found_dollar = str.find('$');
  }
  target = target + str;
  yylval.cpp_string = new std::string(target);
  return WORD;
}



$\{[^ \}\n]*\}([^ \n]*|$\{[^ \}\n]*\})* {
  //Environment variable expansion
  std::string str = yytext;
  std::string target = "";
  std::size_t found_dollar = str.find('$');
  while(found_dollar != -1) {
    std::string var;
    target = target + str.substr(0, found_dollar);
    if (str[found_dollar + 1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        var = str.substr(found_dollar + 2, end - found_dollar - 2);
        /*
        if (var == '$') {
          int pid = getpid();
          std::string s = std::to_string(pid);
          target = target + s;
        }
        */
        if (getenv(var.c_str()))
          target = target + getenv(var.c_str());
        else
          target = target + var;
        str = str.substr(end + 1, -1);
      }
    }
    found_dollar = str.find('$');
  }
  target = target + str;
  yylval.cpp_string = new std::string(target);
  return WORD;
}


~[^ \n\t]* {
  // Tilde Expansion
  std::string str = yytext;
  std::string home_dir = getenv("HOME");
  int length = str.size();
  if (length != 1) {
    if (str[1] == '/')
      str = home_dir + str.substr(1, -1);
    else
      str = std::string("/homes/") + str.substr(1, -1);
    yylval.cpp_string = new std::string(str);
  }
  else
    yylval.cpp_string = new std::string(home_dir);
  return WORD;
}

[^ \|>\t\n][^ \|>\t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}
