// UCLA CS 111 Lab 1 command execution

// Copyright 2012-2014 Paul Eggert.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
prepare_profiling (char const *name)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (0, 0, "warning: profiling not yet implemented");
  return -1;
}

int
command_status (command_t c)
{
  return c->status;
}

int
execute_command (command_t c, int profiling)
{
  //ignore profiling for now
  switch(c->type){
  	case SIMPLE_COMMAND: ;
  	pid_t p = fork();
  	if(p < 0)
  		error(1, 0, "error: unable to fork process");
  	if(p == 0){
  		//in child process
  		/*char cwd[1024];
   		getcwd(cwd, sizeof(cwd));
   		char* envPath;
  		envPath = getenv("PATH");
  		char newPath[1024];
  		strcpy(newPath, envPath);
  		strcat(newPath, ":");
  		strcat(newPath, cwd);
  		setenv("PATH", newPath, 1);
  		printf("%s\n", newPath);*/
  		execvp(c->u.word[0], c->u.word);
   		error(2, 0, "error: unable to execute command"); 		
  	}
  	//in parent process
  	int status;
  	if(waitpid(p, &status, WNOHANG|WUNTRACED) < 0)
        error(2, 0, "error: child process terminate incorrectly");
    c->status = status;
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
  
  default:
  return 0;
  }
}






