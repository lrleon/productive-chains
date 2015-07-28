 /*
 *  The scanner definition for NET.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{


/* Max size of string constants */
#define MAX_STR_CONST 4097
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr = string_buf;

extern int curr_lineno;
extern int verbose_flag;

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
RIF          [rR][iI][fF]
COD          [cC][oO][dD]

DIGIT           [0-9]
UPPER_LETTER    [A-Z]
LOWER_LETTER    [a-z]
ANY_LETTER      ({UPPER_LETTER}|{LOWER_LETTER})
SPACE           [ \f\r\t\v]
NEWLINE         \n

INTEGER         {DIGIT}+
ID              INTEGER

%%

{SPACE}  /* Ignore spaces */

{NEWLINE} { ++curr_lineno; }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{LOAD}       return LOAD;
{RIF}        return RIF;
{COD}        return COD;

 /*
  * The single-characters tokens 
  */
"("          return '(';
")"          return ')';
"="          return '=';
","          return ',';

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
      yylval.symbol = stringtable.add_string(string_buf);
      return STR_CONST;
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
  yylval.symbol = inttable.add_string(yytext);
  return INT_CONST;  
}

"*)" {
  yylval.error_msg = "Unmatched *)";
  return ERROR;
}

. {
  yylval.error_msg = yytext;
  return ERROR; 
 }
  

%%

