// UCLA CS 111 Lab 1 command reading

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
#include <stdio.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */



command_t parse_command(int (*get_next_byte) (void *),
         void *get_next_byte_argument)
{
  char c;
  char token[256];
  int i = 0;
  int word_count = 0; //for simple command;
  char prev_white_space = '\n';
  command_t cmd = (command_t ) checked_malloc (sizeof(struct command));
  cmd->type = UNKNOWN;

  while((c = get_next_byte(get_next_byte_argument)) != EOF)
  {
    if((i < 255) && (c != '\n') && (c != ' ') && (c != '\t') && (c != '<') && (c != '>') && (c != ';') && (c != '|') && (c != '(') && (c != ')'))
    {
      token[i++] = c;
    }
    else{
      if(i == 0 && (c == '\n' || c == '\t' || c == ' ')){ //several white spaces in sequence
        if(c == '\n'){
          prev_white_space = c;
          if(cmd->type == SIMPLE_COMMAND || cmd->type == SUBSHELL_COMMAND)
            return cmd;
        }
        continue;
      }
      token[i] = '\0';
      //A new token is fetched
      if(strcmp(token, "if") == 0 && cmd->type == UNKNOWN){
        cmd->type = IF_COMMAND;
        cmd->u.command[0] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }
      if(strcmp(token, "until") == 0 && cmd->type == UNKNOWN){
        cmd->type = UNTIL_COMMAND;
        cmd->u.command[0] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }
      if(strcmp(token, "while") == 0 && cmd->type == UNKNOWN){
        cmd->type = WHILE_COMMAND;
        cmd->u.command[0] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }


      if(strcmp(token, "then") == 0 && cmd->type == IF_COMMAND){
        cmd->u.command[1] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }
      if(strcmp(token, "else") == 0 && cmd->type == IF_COMMAND){
        cmd->u.command[2] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }
      if(strcmp(token, "do") == 0 && (cmd->type == WHILE_COMMAND || cmd->type == UNTIL_COMMAND)){
        cmd->u.command[1] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }

      if(strcmp(token, "fi") == 0 && cmd->type == IF_COMMAND){
        return cmd;
      }
      if(strcmp(token, "done") == 0 && (cmd->type == WHILE_COMMAND || cmd->type == UNTIL_COMMAND)){
        return cmd;
      }

      //subshell command
      if(cmd->type == UNKNOWN && c == '('){
        cmd->type = SUBSHELL_COMMAND;
        cmd->u.command[0] = parse_command(get_next_byte, get_next_byte_argument);
        i = 0;
        continue;
      }


      //simple command
      if(cmd->type == UNKNOWN){
        cmd->type = SIMPLE_COMMAND;
      }

      if(cmd->type == SIMPLE_COMMAND && i != 0){
        cmd->u.word = (char **) checked_realloc(cmd->u.word, (word_count + 1) * sizeof(char *));
        cmd->u.word[word_count] = (char *) checked_malloc(strlen(token) + 1);
        strcpy(cmd->u.word[word_count++], token);
      }
      if(c == '\n' && cmd->type == SIMPLE_COMMAND){
        cmd->u.word = (char **) checked_realloc(cmd->u.word, (word_count + 1) * sizeof(char *));
        //cmd->u.word[word_count] = (char *) checked_malloc(1);
        cmd->u.word[word_count] = NULL;        
        return cmd;
      }

      //pipeline & sequence
      if((c == '|' || c == ';') && cmd->type == SIMPLE_COMMAND){
        cmd->u.word = (char **) checked_realloc(cmd->u.word, (word_count + 1) * sizeof(char *));
        cmd->u.word[word_count] = NULL; 
        command_t sub_cmd = cmd;
        cmd = (command_t ) checked_malloc (sizeof(struct command));
        cmd->type = (c == '|') ? PIPE_COMMAND : SEQUENCE_COMMAND;
        cmd->u.command[0] = sub_cmd;
        cmd->u.command[1] = parse_command(get_next_byte, get_next_byte_argument);
        return cmd;
      }

      //redirection
      if(c == '<' || c == '>'){
        cmd->u.word = (char **) checked_realloc(cmd->u.word, (word_count + 1) * sizeof(char *));
        cmd->u.word[word_count] = NULL; 
        char io[256];
        int j = 0;
        char c2;
        while(((c2 = get_next_byte(get_next_byte_argument)) != ' ' && c2 != '\t' && c2 != '\n' && c2 != ')') || j == 0){
          if(c2 != ' ' && c2 != '\t')
            io[j++] = c2;
        }
        io[j] = '\0';
        if(c == '<'){
          cmd->input = (char *) checked_malloc(strlen(io) + 1);
          strcpy(cmd->input, io);
        }
        else{
          cmd->output = (char *) checked_malloc(strlen(io) + 1);
          strcpy(cmd->output, io);
        }
        if(c2 == '\n' || c2 == ')') 
          return cmd;
          
      }

      //subshell command return
      if(c == ')'){
        if(cmd->type == SIMPLE_COMMAND){
          cmd->u.word = (char **) checked_realloc(cmd->u.word, (word_count + 1) * sizeof(char *));
          cmd->u.word[word_count] = NULL;
        } 
        return cmd;
      }

      //reset for next token
      prev_white_space = c;
      i = 0;
    }
  }

  //end of file
  return NULL;  
}
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t cmd_stream = NULL, cmd_stream_prev = NULL, head = NULL;
  command_t cmd;
  while((cmd = parse_command(get_next_byte, get_next_byte_argument)) != NULL){

    cmd_stream = (command_stream_t) checked_malloc (sizeof(struct command_stream));
    cmd_stream->cmd = cmd;
    cmd_stream->next = NULL;
    if(cmd_stream_prev == NULL)
      head = cmd_stream;
    else
      cmd_stream_prev->next = cmd_stream;
    cmd_stream_prev = cmd_stream;
  }

  return head;
}

command_t
read_command_stream (command_stream_t s)
{
  if(s->cmd == NULL)
    return NULL;
  command_t cmd_t = s->cmd;
  command_stream_t s_prev = s;

  if(s->next){
    s->cmd = s->next->cmd;
    s->next = s->next->next;
  }
  else{
    s->cmd = NULL;
    s->next = NULL;
  }

 //s_prev to be freed: command.u.word[0, 1, 2, ...], command, command_stream

  return cmd_t;
}
