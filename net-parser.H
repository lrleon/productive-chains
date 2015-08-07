# ifndef NET_PARSER_H
# define NET_PARSER_H

# include <readline/readline.h>
# include <readline/history.h>
# include <string>
# include <iostream>

# ifndef YY_NULL
# define YY_NULL 0
# endif

using namespace std;

# include <net-tree.H>

extern FILE *yyin;


// # if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
// typedef union YYSTYPE
// {
//   char * symbol;
//   char * error_msg;
// } YYSTYPE;

// # define YYSTYPE_IS_TRIVIAL 1
// # define YYSTYPE_IS_DECLARED 1
// # endif



# endif // NET_PARSER_H
