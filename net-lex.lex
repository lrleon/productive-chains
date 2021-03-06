 /*
 *  The scanner definition for NET.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */

%{

# include <net-parser.H> 
# include "net.tab.h"


  YYSTYPE netyylval;
  size_t curr_lineno = 0;

# define yylval netyylval

/* Max size of string constants */
# define MAX_STR_CONST 4097
# define MAX_CWD_SIZE 4097
# define YY_NO_UNPUT   /* keep g++ happy */



/* define YY_INPUT so we read thorugh readline */
/* # undef YY_INPUT */
/* # define YY_INPUT(buf, result, max_size) result = get_input(buf, max_size); */


char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr = string_buf;



/*
 *  Add Your own definitions here
 */

 long nested_comment_counter = 0;

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

%x NESTED_COMMENT SINGLE_COMMENT STRING

/*
 * Define names for regular expressions here.
 */
/* Keywords */
LOAD         [lL][oO][aA][dD]
SAVE         [sS][aA][vV][eE]
RIF          [rR][iI][fF]
COD          [cC][oO][dD]
EXIT         [eE][xX][iI][tT]

DIGIT           [0-9]
UPPER_LETTER    [A-Z]
LOWER_LETTER    [a-z]
ANY_LETTER      ({UPPER_LETTER}|{LOWER_LETTER})
SPACE           [ \f\r\t\v]
NEWLINE         \n

INTEGER         {DIGIT}+
ID              {INTEGER}
VARNAME         {ANY_LETTER}([_\.-]|{ANY_LETTER}|{DIGIT})*

%%

{SPACE}  { cout << "SPACE" << endl; /* Ignore spaces */ }

{NEWLINE} { ++curr_lineno; cout << "MATCH NEWLINE" << endl; return NEWLINE; }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{LOAD}       { cout << "MATCH LOAD" << endl; return LOAD; }
{SAVE}       return SAVE;
{RIF}        return RIF;
{COD}        return COD;
{EXIT}       return EXIT;

 /*
  * The single-characters tokens 
  */
[=;]          return *yytext;


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
<STRING>{NEWLINE} {
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
      yylval.symbol = strdup(string_buf); // TODO: ojo con este memory leak
      cout << "MATCH STRCONST" << endl;
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

{ID} { // matches integer constant 
  yylval.symbol = yytext;
  return ID;  
}

{VARNAME} {
  yylval.symbol = yytext;
  return VARNAME;
}

"*)" {
  yylval.error_msg = "Unmatched *)";
  return ERROR;
}

. {
  cout << "LEX ERROR" << endl;
  yylval.error_msg = yytext;
  return ERROR; 
 }
  

%%

int yywrap()
{
  return 1;
}
