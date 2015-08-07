%{

# include <net-parser.H> 

/* # include <iostream> */
/*   using namespace std; */

extern int get_input(char *buf, int size);
# undef YY_INPUT
# define YY_INPUT(buf,result,max_size) result = get_input(buf, max_size);

 int yylex(void);
 void yyerror(char const *);

 /* bool error_found = false; */

 /* CommandList * command_list; */

 /* using CommandListPtr = CommandList*; */
 /* using CommandPtr = Command*; */

%}

/* %union { */
/*   char * symbol; */
/*   char * error_msg; */
/* }; */

%token LOAD SAVE RIF COD EXIT STRCONST VARNAME
/* %token <symbol> STRCONST */
/* %token <symbol> VARNAME  */

/* %type <symbol> cmd_unit */

%%

input: line
;

line: cmd_unit '\n'
  {
    cout << "PARSED LINE" << endl; 
  }
;  

cmd_unit: LOAD STRCONST
{
  cout << "LOAD" << endl;
}
        | SAVE STRCONST 
{
  cout << "SAVE" << endl;
}
;

%%

void yyerror(char const * s) 
{
  cout << "ERROR " << s << " " << endl;
}


int yylex()
{
  static int count = 0;
  switch (count++)
    {
    case 0: cout << "LOAD" << endl; return LOAD;
    case 1: cout << "STRCONST" << endl; return STRCONST;
    case 2: cout << "EOL" << endl; return '\n';
    default: cout << "EOF" << endl; return EOF;
    }
}

int main()
{
  
   /* while (true) */
   /*   { */
      /* char * line = // readline("> "); */
      /* 	"Load \"name\"\n\n"; */

      /* YY_BUFFER_STATE bp = yy_scan_string(line); */
      /* yy_switch_to_buffer(bp); */

       int status = yyparse();

       cout << "STATUS = " << status << endl;

      /* yy_delete_buffer(bp); */
    // }
}
