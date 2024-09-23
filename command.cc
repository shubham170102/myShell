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

#include <iostream>

#include "command.hh"
#include "shell.hh"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>

extern char ** environ;
//int bglist[1000];

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    int flag = 0;
    if ( _outFile == _errFile)
      flag = 1;

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile && flag == 0 ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");
  
    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
      if (isatty(0))
        Shell::prompt();
      return;
    }

    // Print contents of Command data structure
    //print();

    const char *cmd = _simpleCommands[0]->_arguments[0]->c_str();
    // Exit myshell
    if (!strcmp(cmd, "exit")) {
      if (isatty(0)) {
        printf("Good bye!!\n");
      }
      exit(1);
    }

    // Builtin function - setenv
    if (strcmp(cmd, "setenv") == 0) {
      const char *cmd1 =_simpleCommands[0]->_arguments[1]->c_str();
      const char *cmd2 =_simpleCommands[0]->_arguments[2]->c_str();
      int env = setenv(cmd1, cmd2, 1);
      if (env != 0)
        perror("setenv");
      clear();
      Shell::prompt();
      return;
    }

    //Builtin function - unsetenv
    if (strcmp(cmd, "unsetenv") == 0) {
      const char *cmd1 =_simpleCommands[0]->_arguments[1]->c_str();
      int env = unsetenv(cmd1);
      if (env != 0)
        perror("unsetenv");
      clear();
      Shell::prompt();
      return;
    }

    //Builtin function - cd
    if (strcmp(cmd, "cd") == 0) {
      std::string *cmd1 =_simpleCommands[0]->_arguments[1];
      int error;
      if (_simpleCommands[0]->_arguments.size() == 1)
        error = chdir(getenv("HOME"));
      else {
        error = chdir(cmd1->c_str());
      }
      if (error != 0)
        fprintf(stderr, "cd: can't cd to %s\n", cmd1->c_str()); 
      clear();
      Shell::prompt();
      return;
    }
    // Execution and IO Redirection
    //  Save in/out
    int tempin = dup(0);
    int tempout = dup(1);
    int temperr = dup(2);

    // Initial input, output, error declaration
    int fdin = 0;
    int fdout = 0;
    int fderr = 0;

    // Input file ridection
    if (_inFile)
      fdin = open(_inFile->c_str(), O_RDONLY);
    else
      // Use default input
      fdin = dup(tempin);

    int ret; // will be used for fork()
    for (unsigned int i = 0; i < _simpleCommands.size(); i++) {
      // Redirect stdin/input to file
      dup2(fdin, 0);
      close(fdin);

      unsigned int size = _simpleCommands.size();
      // setup output and error
      if (i == size - 1) {
        // Error file redirection
        if (_errFile) {
          if (_append)
            fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0664);
          else
            fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
        } else {
          // Use default error/stderr
          fderr = dup(temperr);
        }
        //  Output file redirection
        if (_outFile) {
          if (_append)
            fdout = open(_outFile->c_str(), O_CREAT | O_WRONLY | O_APPEND, 0664);
          else
            fdout = open(_outFile->c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0664);
        } else {
            // Use default output/stdout
            fdout = dup(tempout);
        }
        // Redirect stderr to file

         dup2(fderr, 2);
        close(fderr);
      }
      else {
        // Create Pipe
        int fdpipe[2];
        pipe(fdpipe);
        fdout = fdpipe[1];
        fdin = fdpipe[0];
      }
      // Redirect stdout to file
      dup2(fdout, 1);
      close(fdout);

      // Creation of Child Process
      ret = fork();
      if (ret == 0) {
        // Builtin function - printenv
        if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
          char **p = environ;
          while(*p != NULL) {
            printf("%s\n", *p);
            p++;
          }
          exit(0);
        }

        // Builtin function - source
        if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source")) {
          const char *cmd_arg1 = _simpleCommands[i]->_arguments[1]->c_str();
          FILE *fp = fopen(cmd_arg1, "r");
          char command_line[1024];
          fgets(command_line, 1023, fp);
          fclose(fp);

          // pipe system call and save in/out redirection
          int pin[2];
          int pout[2];
          int tempin;
          int tempout;
          tempin = dup(0);
          tempout = dup(1);
          pipe(pin);
          pipe(pout);

          // Writing into command line
          write(pin[1], command_line, strlen(command_line));
          write(pin[1], "\n", 1);
          close(pin[1]);

          // Redirect stdin and stdout
          dup2(pin[0], 0);
          dup2(pout[1], 1);
          close(pin[0]);
          close(pout[1]);

          // Child Process for source
          int pid = fork();
          if (pid == 0) {
            char ** progname = new char* [2];
            progname[0] = (char*) "/proc/self/exe";
            progname[1] = NULL;
            execvp(progname[0], progname);
            perror("execvp for source");
            exit(1);
          }
          else if (pid < 0) {
            // Error in fork
            perror("fork");
            exit(1);
          }

          // Restore in/out defaults
          dup2(tempin, 0);
          dup2(tempout, 1);
          close(tempin);
          close(tempout);

          // Read the output
          char ch;
          char * buffer = (char *) malloc (100);
          int i = 0;

          while(read(pout[0], &ch, 1)) {
            // Read result into buffer
            if (ch != '\n')
              buffer[i++] = ch;
            else
              continue;
          }
          // Terminate
          buffer[i] = '\0';
          printf("%s\n", buffer);
        }
        else {
          // Child Process
          const char *command = _simpleCommands[i]->_arguments[0]->c_str();
          int num = _simpleCommands[i]->_arguments.size();
          char ** args = new char* [num + 1];
          for (unsigned int j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
            args[j] = (char *)_simpleCommands[i]->_arguments[j]->c_str();
          }
          args[num] = NULL;
          execvp(command, args);
          perror("execvp");
          exit(1);
        }
      }
      else if (ret < 0) {
        // Error in fork
        perror("fork");
        return;
      }
    }

    // Restore in/out/error defaults
    dup2(tempin, 0);
    dup2(tempout, 1);
    dup2(temperr, 2);
    close(tempin);
    close(tempout);
    close(temperr);

    int newarg = (_simpleCommands[_simpleCommands.size()-1]->_arguments.size()) - 1;
    Shell::value_arg = *(_simpleCommands[_simpleCommands.size()-1]->_arguments[newarg]);

    if (!_background) {
      // Parent Process
      // Wait for last command
      int i;
      waitpid(ret, &i, 0);
      setenv("?", std::to_string(WEXITSTATUS(i)).c_str(), 1);
      if (WEXITSTATUS(i) != 0) {
        if (getenv("ON_ERROR")) {
          printf(getenv("ON_ERROR"));
          printf("\n");
        }
      }
    }
    else {
      // addtion to background list
      std::string x = std::to_string(ret).c_str();
      setenv("!", x.c_str(), 1);
      /*
      for (int i = 0; i < 1000; i++) {
        if (bglist[i] == 0) {
          bglist[i] = ret;
          break;
        }
      }
      */
    }

    //printf("\n");
    // Clear to prepare for next command
    clear();

    // Print new prompt
    if (isatty(0))
      Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
//CommandShell
