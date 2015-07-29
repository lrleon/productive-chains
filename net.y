%{

extern int get_input(char *buf, int size);
# undef YY_INPUT
# define YY_INPUT(buf,result,max_size) result = get_input(buf, max_size);

 bool error_found = false;

%}

%%

%token LOAD SAVE RIF COD EXIT VARNAME ID STRCONST ERROR

execute: cmd_list '\n' {
  $$=NULL;
  errfnd=false;
  YYACCEPT
    }
;


cmd_list: cmd_list1 {
  $$=NULL;
 }
| cmd_list1 ';' {
  $$=NULL;
  }
;

cmd_list1: cmd_list1 ';' cmd_unit {
  $$=NULL;
 }
| cmd_list1 ';' error {
  error_found = true;
  $$ = NULL;
  yyclearin;
  YYRECOVERING();
  }
| cmd_unit {
  $$=NULL;
  }
| error  {
  if(not error_found)
    {
      errfnd=true;
    }
  yyclearin;
  YYRECOVERING();
  $$=NULL;
  }
;

cmd_unit: LOAD STRCONST {
  $$ = new Load($1);
 }
| SAVE STRCONST {
  $$ = new Save($1);
 }

%%
