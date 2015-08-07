/*
 *  The scanner definition for NET.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */

%{

# include <tclap/CmdLine.h>
# include <net-parser.H>
# include <net-symtbl.H>
# include "test.tab.h"

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

SPACE           [ \f\r\t\v]

INTEGER         [[:digit:]]+
ID              {INTEGER}
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

extern int yydebug;

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

int main()
{
# ifdef YYDEBUG
  yydebug = 1;
# endif
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
	   cout << "line correctly parsed" << endl;
	   ++i;
	   if (line_commands != nullptr)
	     line_commands->for_each([] (auto c) { c->execute(); });
	   cout << endl;
	 }
       
       add_history(line);

       free(line);
       yy_delete_buffer(bp);
     }
}
