%{

# define YYDEBUG 0

# include <iostream>  
# include <net-tree.H>
# include <net-symtbl.H>

  using namespace std;

 int yylex(void);
 void yyerror(char const *);

 SymbolTable var_tbl;

 ASTList * line_commands = nullptr;

%}

%union {
  ASTNode * node;
  ASTList * node_list;
  Exp * expr;
  char * symbol;
  char * error_msg;
};

%token LOAD SAVE EXIT ERROR INFO
%token <symbol> STRCONST INTCONST VARNAME 

%type <expr> exp
%type <expr> var 
%type <expr> ref_exp cmd_unit
%type <node_list> cmd_list line


%%

line: /* empty line */ { $$ = line_commands = nullptr; }
    | cmd_list '\n'
      {
	$$ = line_commands = $1;
      }
    | cmd_list
      {
	$$ = line_commands = $1;
      }
;

cmd_list: cmd_unit
          {
	    auto l = new ASTList;
	    l->append($1);
	    $$ = l;
	  }
        | cmd_unit ';' cmd_list
  	  {
	    auto l = $3;
	    l->append($1);
	  }
;

cmd_unit: EXIT
          {

	  }
        | INFO VARNAME
	  {
	    $$ = new Info($2);
	  }
        | VARNAME '=' exp
	  {
	    auto name = $1;
	    Varname * var = var_tbl(name);
	    if (var == nullptr)
	      {
		var = new Varname(name);
		var_tbl.addvar(name, var);
	      }
	    auto ptr = new Assign(var, $3);
	    $$ = ptr;
	  }
        | SAVE ref_exp
	  {

	  }
;


// Cmd_list: cmd_unit 

exp : LOAD ref_exp
      {
	$$ = new Load(static_cast<StringExp*>($2));
      }
    | VARNAME '[' ref_exp ']'
      {
	
      }
    | ref_exp
      {

      }
;

ref_exp : STRCONST
            {
	      assert(string_table($1));
	      auto symbol = string_table($1);
	      StringExp * str_exp = new StringExp(symbol);
	      $$ = str_exp;
	    }
          | INTCONST
	    {
	      assert(id_table($1) == $1);
	      // auto SymbolTable = id_table($1);
	      // TODO
	    }
          | var
	    {
	      $$ = $1;
	    }
;

var: VARNAME
    {
      assert(string_table($1));
      auto symbol = string_table($1);
      Varname * var = new Varname(symbol);
      $$ = var;
    }
;

%%

void yyerror(char const * s) 
{
  cout << "ERROR " << s << " " << endl;
}

ExecStatus Load::execute()
{
  const string & name = file_name->value;
  if (file_name->type != STRCONST)
    {
      stringstream s;
      s << name << " is not a string";
      return make_pair(false, s.str());
    }

  ifstream in(name);
  if (in.fail())
    {
      stringstream s;
      s << "cannot open file " << name;
      return make_pair(false, s.str());
    }

  new (&mapa) MetaMapa(in);
  
  return make_pair(true, "");
}

ExecStatus Save::execute()
{
  return ExecStatus();
}

ExecStatus Assign::execute()
{
  right_side->execute();
  switch (right_side->type)
    {
    case Exp::Type::GRAPH:
      {
	cout << "LOAD ASSIGN" << endl;
      again:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarMap;
	    left_side->set_value_ptr(var);
	  }
	
	if (var->var_type != Var::VarType::Map)
	  {
	    left_side->free_var();
	    goto again;
	  }
	static_cast<VarMap*>(var)->value = 
	  move(static_cast<Load*>(right_side)->mapa);
	return make_pair(true, "");
      }
    default:
      cout << "Assign " << right_side->type_string() << " = "
	   << left_side->type_string() << " not yet implemented" << endl;
      return make_pair(false, "Assignation type not yet implemented");
    };
  return ExecStatus();
}

ExecStatus Info::execute() 
{
  stringstream s;
  s << "var name: ";
  Varname * varname = var_tbl(name);
  if (varname == nullptr)
    {
      s << name <<  " not found";
      string str = s.str();
      cout << str << endl;
      return make_pair(false, move(str));
    }

  auto var = varname->get_value_ptr();
  assert(var);
  s << var->info();
  cout << s.str();
  return make_pair(true, "");
}
