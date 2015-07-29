
# include <readline/readline.h>
# include <readline/history.h>
# include <string>
# include <iostream>

# include <net-tree.H>

using namespace std;

enum TokenType
  {
    LOAD = 256, SAVE, EXIT, RIF, COD, VARNAME, ID, STRCONST, ERROR
  };

typedef union YYSTYPE
{
  char * symbol;
  char * error_msg;
} YYSTYPE;

inline int get_input(char *buf, int size)
{
  char *line;
  int len = 0;
  if (feof(yyin))
    return YY_NULL;

  
  string prompt;
  line = readline("> ");
  if (!line)
    return YY_NULL;

  strncpy(buf, line, size - 2);
  len=strlen(line);
  len= (len) > (size - 2) ? (size - 2) : len;
  buf[len]='\0';
  add_history(buf);

  buf[len]='\n';
  buf[len+1]='\0';
  free(line);
  return strlen(buf);
}
