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
#include <fcntl.h>
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
  pid_t p;
  int condition;
  int returnValue;
  int status;
  switch(c->type){
  	case SIMPLE_COMMAND: ;
  	p = fork();
  	if(p < 0)
  		error(1, 0, "error: unable to fork process");
  	if(p == 0){
  		//in child process
  		if(c->input){
  			int fd = open(c->input, O_RDWR);
  			if(fd < 0)
  				error(3, 0, "error: could not open file %s", c->input);
  			dup2(fd, 0);
  		}
  		if(c->output){
  			int fd = open(c->output, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
  			if(fd < 0)
  				error(3, 0, "error: could not open file %s", c->output);
  			dup2(fd, 1);
  		}
  		execvp(c->u.word[0], c->u.word);
   		error(2, 0, "error: unable to execute command"); 		
  	}
  	//in parent process
  	if(waitpid(p, &status, WNOHANG|WUNTRACED) < 0)
        error(2, 0, "error: child process terminate incorrectly");
    c->status = status;
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
  
  case IF_COMMAND: ;
  condition = execute_command(c->u.command[0], profiling);
  if(condition == 0)
  	 return execute_command(c->u.command[1], profiling);
  else{
  	if(c->u.command[2])
  		return execute_command(c->u.command[2], profiling);
  	return 0;
  }

  case WHILE_COMMAND:
  case UNTIL_COMMAND: ;
  while((condition = execute_command(c->u.command[0], profiling)) == ((c->type == WHILE_COMMAND) ? 0 : 1))
  {
  	returnValue = execute_command(c->u.command[1], profiling);
  }
  return returnValue;

  case SEQUENCE_COMMAND: ;
  execute_command(c->u.command[0], profiling);
  returnValue = execute_command(c->u.command[1], profiling);
  return returnValue;

  case SUBSHELL_COMMAND: ;
  return execute_command(c->u.command[0], profiling);

  case PIPE_COMMAND: ;
  int fd[2];
  pipe(fd);
  p = fork();
  	if(p < 0)
  		error(1, 0, "error: unable to fork process");
  	else if(p == 0){
  		close(fd[0]);
  		dup2(fd[1], 1);
  		execute_command(c->u.command[0], profiling);
  	}
  	else{
  		close(fd[1]);
  		dup2(fd[0], 0);
  		if(waitpid(p, &status, WNOHANG|WUNTRACED) < 0)
        	error(2, 0, "error: child process terminate incorrectly");
   		return execute_command(c->u.command[1], profiling);       
  	}

  default:
  return 0;
  }
}






