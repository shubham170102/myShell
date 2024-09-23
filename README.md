# myShell Interpreter Project

## Overview
This project aims to build a shell interpreter that combines behaviors from common shells, such as Bash and csh. The shell includes functionalities such as command execution, IO redirection, piping, and process management. It also features signal handling, environment variable expansion, wildcard expansion, and subshells. Skeleton code is provided, and the shell is implemented using Flex (Lex) and Bison (Yacc).

## Features Implemented

### Part 1: Parsing and Executing Commands
- **Lex and Yacc Integration**: The shell supports a complex grammar with command arguments, pipes, and file redirection for stdin, stdout, and stderr.
- **Simple Command Execution**: Commands are executed using `fork()` and `execvp()`. The shell waits for foreground commands and allows background execution with `&`.
- **File Redirection**: Redirection is handled with `dup2()` for stdin, stdout, and stderr.
- **Pipes**: Command output can be piped using `pipe()` and `dup2()`, allowing the chaining of multiple commands.
- **Interactive/Non-interactive Mode**: The shell detects if it's running interactively using `isatty()` and adjusts its behavior accordingly.

### Part 2: Signal Handling, Subshells, and More
- **Ctrl-C Handling**: The shell ignores `SIGINT` when no process is running, discarding the current prompt and providing a new one.
- **Zombie Process Elimination**: The shell cleans up zombie processes by handling `SIGCHLD` signals and using `waitpid()`.
- **Exit Command**: The `exit` command terminates the shell and prints a goodbye message.
- **Quote Support**: Arguments with spaces can be passed using quotes.
- **Escaping Special Characters**: Escape characters (`\`) are handled for special characters like quotes and `&`.
- **Builtin Functions**:
  - `printenv`: Prints environment variables.
  - `setenv`: Sets environment variables.
  - `unsetenv`: Unsets environment variables.
  - `source`: Executes commands from a specified file.
  - `cd`: Changes the current working directory.
- **Subshells**: Subshells are implemented using `$(command)` to execute commands and use their output within another command.

### Part 3: Expansions, Wildcards, and Line Editing
- **Environment Variable Expansion**: Variables like `${var}`, `${$}`, `${?}`, `${!}`, `${_}`, and `${SHELL}` are expanded.
- **Tilde Expansion**: The `~` character expands to the current user's home directory, and `~user` expands to the specified user's home directory.
- **Wildcard Expansion**: The shell supports wildcard characters (`*` and `?`) for file and directory name matching.
- **Line Editor**: The shell includes basic line editing features such as cursor movement, backspace, and delete.
- **Command History**: The shell tracks command history and allows users to navigate through previous commands using the arrow keys.
- **Ctrl-R for Reverse Search**: The shell allows reverse searching through command history with `Ctrl-R`.


