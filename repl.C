

# include <net-lex.C>


extern int yyparse();

int main()
{
  // while (true)
  //   {
      char * line = // readline("> ");
	"Load \"name\"\n\n";

      YY_BUFFER_STATE bp = yy_scan_string(line);
      yy_switch_to_buffer(bp);

      yyparse();

      yy_delete_buffer(bp);
    // }
}
