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
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
char right_buffer[MAX_BUFFER_LINE];
int right_length;
int curr_pos;



// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [] = {
  "ls -al | grep x", 
  "ps -e",
  "cat read-line-example.c",
  "vi hello.c",
  "make",
  "ls -al | grep xxx | grep yyy"
};
int history_length = sizeof(history)/sizeof(char *);

void read_line_print_usage()
{
  char * usage = "\n"
    " left arrow   moves cursor and allows insertion\n"
    " right arrow  moves cursor and allows insertion\n"
    " ctrl-?       Print usage\n"
    " ctrl-D       removes charter at the cursor\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

void backspace(int n) {
  int i = 0;
  char ch = 8;
  for (i = 0; i < n; i++)
    write(1, &ch, 1);
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  curr_pos = 0;

  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch != 127) {
      // It is a printable character. 

      // Do echo
      write(1,&ch,1);

      // If max number of character reached return.
      if (line_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      line_buffer[line_length]=ch;
      line_length++;
    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 1) {
      
      for (int i = curr_pos; i > 0; i--) {
        ch = 8;
        write(1, &ch, 1);
        curr_pos--;
      }
      
      for (int i = 0; i < line_length; i++) {
        char character = 8;
        write(1, &character, 1);
        right_buffer[right_length] = line_buffer[line_length - 1];
        line_length--;
        right_length++;

      }
    }
    else if (ch == 8 || ch == 127) {
      // <backspace> was typed. Remove previous character read.
      if (line_length != 0) {
      // Go back one character
      ch = 8;
      write(1,&ch,1);
      for (int i = right_length - 1; i >= 0; i--) {
        char temp = right_buffer[i];
        write(1,&temp,1);
      }

      // Write a space to erase the last character read
      ch = ' ';
      write(1,&ch,1);

      // Go back one character
      for (int i = 0; i < right_length + 1; i++) {
        char temp = 8;
        write(1,&temp,1);
      }

      // Remove one character from buffer
      line_length--;
      } 
      else
        continue;
    } else if (ch == 1) {
      for (int i = 0; i < line_length; i++) {
        char var = 8;
        write(1,"\033[1C", 5);
        int index = right_length - 1;
        line_buffer[line_length] = right_buffer[index];
        line_length++;
        right_length--;
      }
      /*
      while (curr_pos >= 0) {
        char ch1 = 8;
        write(1, &ch, 1);
        curr_pos--;
      }
      */
    } else if (ch == 5) {
        for (int i = right_length - 1; i >= 0; i--) {
          write(1,"\033[1C", 5);
          int index = right_length - 1;
          line_buffer[line_length] = right_buffer[index];
          line_length++;
          right_length--;
        }
      /*
      while (curr_pos < line_length) {
        char ch2 = line_buffer[curr_pos];
        write(1, &ch2, 1);
        curr_pos++;
      }
      */
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
	// Up arrow. Print next line in history.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	

	// Copy line from history
	strcpy(line_buffer, history[history_index]);
	line_length = strlen(line_buffer);
	history_index=(history_index+1)%history_length;

	// echo line
	write(1, line_buffer, line_length);
      }
      else if (ch1==91 && ch2==68) {
        //Left arrow
        if (line_length != 0) {
        ch = 8;
        write(1, &ch, 1);

        right_buffer[right_length] = line_buffer[line_length - 1];
        right_length++;
        line_length--;
        }
        else
          continue;
      }
      else if (ch1 == 91 && ch2 == 67) {
        if (right_length != 0) {
          //char ch2 = line_buffer[curr_pos];
          write(1, "\033[1C", 5);
          int last_index = right_length - 1;
          line_buffer[line_length] = right_buffer[last_index];
          right_length--;
          line_length++;
        }
        else
          continue;
      }
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  return line_buffer;
}

