/*
 *  The scanner definition for NET.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */

%{

# include <signal.h>
# include <stdlib.h>
# include <unistd.h>
# include <tclap/CmdLine.h>
# include <net-parser.H>
# include <net-symtbl.H>
# include "test.tab.h"

# define YYDEBUG 1

  size_t curr_lineno = 0;

  StringTable string_table;
  IdTable id_table;

/* Max size of string constants */
# define MAX_STR_CONST 4097
# define MAX_CWD_SIZE 4097
# define YY_NO_UNPUT   /* keep g++ happy */


  char string_buf[MAX_STR_CONST]; /* to assemble string constants */
  char *string_buf_ptr = string_buf;

  bool string_error = false;

  inline bool put_char_in_buf(char c)
  {
    if (string_buf_ptr == &string_buf[MAX_STR_CONST - 1])
      {
	yylval.error_msg = "String constant too long";
	string_error = true;
	return false;
      }
    *string_buf_ptr++ = c;
    return true;
  }

%}

%x STRING

/*
 * Define names for regular expressions here.
 */
/* Keywords */
LOAD         [lL][oO][aA][dD]
SAVE         [sS][aA][vV][eE]
EXIT         [eE][xX][iI][tT]
INFO         [iI][nN][fF][oO]
LS           [lL][sS]
RM           [rR][mM]
SEARCH       [sS][eE][aA][rR][cC][hH]
PRODUCER     [pP][rR][oO][dD][uU][cC][eE][rR]
PRODUCERS    [pP][rR][oO][dD][uU][cC][eE][rR][sS]
PRODUCT      [pP][rR][oO][dD][uU][cC][tT]
ID           [iI][dD]
REGEX        [rR][eE][gG][eE][xX]
LIST         [lL][iI][sS][tT]
APPEND       [aA][pP][pP][eE][nN][dD]
HELP         [hH][eE][lL][pP]
COD          [cC][oO][dD]
TYPE         [tT][yY][pP][eE]
RIF          [rR][iI][fF]
NODE         [nN][oO][dD][eE]
REACHABLE    [rR][eE][aA][cC][hH][aA][bB][lL][eE]
COVER        [cC][oO][vV][eE][rR]
DOT          [dD][oO][tT]
UPSTREAM     [uU][pP][sS][tT][rR][eE][aA][mM]
INPUTS       [iI][nN][pP][uU][tT][sS]
OUTPUTS      [oO][uU][tT][pP][uU][tT][sS]
INPUT        [iI][nN][pP][uU][tT]
OUTPUT       [oO][uU][tT][pP][uU][tT]
ARCS         [aa][rR][cC][sS]
PATH         [pP][aA][tT][hH]
RANKS        [rR][aA][nN][kK][sS]
SHAREHOLDER  [sS][hH][aA][rR][eE][hH][oO][lL][dD][eE][rR]
HOLDING      [hH][oO][lL][dD][iI][nN][gG]

SPACE           [ \f\r\t\v]

INTEGER         [[:digit:]]+
VARNAME         [[:alpha:]][[:alnum:]_.-]*

%%

{SPACE}  /* Ignore spaces */ 

\n { ++curr_lineno; return '\n'; }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{LOAD}       return LOAD; 
{SAVE}       return SAVE;
{EXIT}       return EXIT;
{INFO}       return INFO;
{LS}         return LS;
{RM}         return RM;
{SEARCH}     return SEARCH;
{PRODUCER}   return PRODUCER;
{PRODUCERS}  return PRODUCERS;
{PRODUCT}    return PRODUCT;
{REGEX}      return REGEX;
{ID}         return ID;
{LIST}       return LIST;
{APPEND}     return APPEND;
{COD}        return COD;
{TYPE}       return TYPEINFO;
({HELP}|\?)  return HELP;
{RIF}        return RIF;
{NODE}       return NODE;
{REACHABLE}  return REACHABLE;
{COVER}      return COVER;
{DOT}        return DOT;
{UPSTREAM}   return UPSTREAM;
{INPUT}      return INPUT;
{OUTPUT}     return OUTPUT;
{INPUTS}     return INPUTS;
{OUTPUTS}    return OUTPUTS;
{ARCS}       return ARCS;
{PATH}       return PATH;
{RANKS}      return RANKS;
{SHAREHOLDER} return SHAREHOLDER;
{HOLDING}    return HOLDING;

 /*
  * The single-characters tokens 
  */
[=;]          return *yytext;
"["           return *yytext;
"]"           return *yytext;


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

\" { /* start of string */
  string_buf_ptr = &string_buf[0];
  string_error = false;
  BEGIN(STRING); 
} 
<STRING>[^\\\"\n\0] {
  if (not put_char_in_buf(*yytext))
    return ERROR;
 }
<STRING>\\\n {  // escaped string
  if (not put_char_in_buf('\n'))
    return ERROR;
  ++curr_lineno;
 } 
<STRING>\\n {
  if (not put_char_in_buf('\n'))
    return ERROR;
 }
<STRING>\\t {
  if (not put_char_in_buf('\t'))
    return ERROR;
 }
<STRING>\\b {
  if (not put_char_in_buf('\b'))
    return ERROR;
 }
<STRING>\\f {
  if (not put_char_in_buf('\f'))
    return ERROR; 
}
<STRING>\\\0 {
  yylval.error_msg = "String contains escaped null character.";
  string_error = true;
  return ERROR;
 }
<STRING>'\n' {
  BEGIN(INITIAL);
  ++curr_lineno;
  yylval.error_msg = "Unterminated string constant";
  return ERROR;
 }
<STRING>\" { /* end of string */
  *string_buf_ptr = '\0';
  BEGIN(INITIAL);  
  if (not string_error)
    {
      yylval.symbol = string_table.addstring(string_buf);
      return STRCONST;
    }
 }
<STRING>\\[^\n\0ntbf] {
  if (not put_char_in_buf(yytext[1]))
    return ERROR; 
 }
<STRING>'\0' {
  yylval.error_msg = "String contains escaped null character.";
  string_error = true;
  return ERROR;
 }
<STRING><<EOF>> {
  yylval.error_msg = "EOF in string constant";
  BEGIN(INITIAL);
  return ERROR;
 }

{INTEGER} { // matches integer constant 
  yylval.symbol = id_table.addstring(yytext);
  assert(yylval.symbol);
  return INTCONST;  
}

{VARNAME} {
  yylval.symbol = string_table.addstring(yytext);
  assert(yylval.symbol);
  return VARNAME;
}

. {
  yylval.error_msg = yytext;
  return ERROR; 
 }
  

%%

int yywrap()
{
  return 1;
}

extern int yyparse();

string get_prompt(size_t i)
{
  stringstream s;
  s << i << " > ";
  return s.str();
}

# ifdef YYDEBUG
int yydebug;
# endif

bool verbose = true;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("repl", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", true);
  cmd.add(verbose);

  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();
}

extern ASTList * line_commands;

bool exit_by_ctr_c = false;

void my_handler(int s)
{
  if (s == 15)
    exit_by_ctr_c = true;
}

int main()
{
# ifdef YYDEBUG
  yydebug = 1;
# endif

  /* signal (SIGINT,my_handler); */
  // struct sigaction sigIntHandler;
  // sigIntHandler.sa_handler = my_handler;
  // sigemptyset(&sigIntHandler.sa_mask);
  // sigIntHandler.sa_flags = 0;
  // sigaction(SIGINT, &sigIntHandler, NULL);

  if (not resize_process_stack(128*1024*1024))
    cout << "Warning: cannot resize process stack" << endl
	 << endl;

  for (size_t i = 0; true;) 
     {
       string prompt = get_prompt(i);
       char * line =  readline(prompt.c_str());
       if (line == nullptr)
	 break;

       YY_BUFFER_STATE bp = yy_scan_string(line);
       yy_switch_to_buffer(bp);

       int status = yyparse();

       if (status == 0)
	 {
	   /* cout << "line correctly parsed" << endl; */
	   ++i;
	   if (line_commands != nullptr)
	     {
	       line_commands->for_each([] (auto c) 
                 {
		   auto status = c->execute();
		   if (not status.first)
		     {
		       cout << "ERROR: " << status.second << endl;
		       c->free();
		     }
		 });
	       delete line_commands;
	       line_commands = nullptr;
	     }
	   cout << endl;
	 }

       if (exit_by_ctr_c)
	 cout << "ctr-c pressed" << endl;
       
       add_history(line);

       free(line);
       yy_delete_buffer(bp);
     }
}
