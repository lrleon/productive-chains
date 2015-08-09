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
  Rm  * rmexp;
  char * symbol;
  char * error_msg;
};

%token LOAD SAVE EXIT ERROR INFO LS RM
%token <symbol> STRCONST INTCONST VARNAME 

%type <expr> exp
%type <expr> rvalue
%type <expr> ref_exp cmd_unit
%type <node_list> cmd_list line
%type <rmexp> rm_list


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
	    // TODO: limpiar todo el ambiente. Podria ser desde una rutina
	    cout << "Bye ;-)" << endl 
		 << endl;
	    exit(0);
	  }
        | INFO VARNAME
	  {
	    $$ = new Info($2);
	  }
        | VARNAME '=' exp
 	  { 
	    auto ptr = new Assign($1, $3);
	    $$ = ptr;
	  }
        | SAVE ref_exp ref_exp
	  {
	    
	  }
        | VARNAME
	  {
	    $$ = new Info($1);
	  }
        | LS
	  {
	    $$ = new Ls();
	  }
        | RM rm_list
  	  {
	    $$ = $2;
	  }
;

rm_list : VARNAME
          {
	    auto rm = new Rm;
	    rm->names.append($1);
	    $$ = rm;
	  }
        | VARNAME rm_list
  	  {
	    auto rm = $2;
	    rm->names.append($1);
	    $$ = rm;
	  }

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
	      auto symbol = id_table($1);
	      IntExp * int_exp = new IntExp(symbol);
	      $$ = int_exp;
	    }
          | rvalue
	    {
	      $$ = $1;
	    }
;

rvalue: VARNAME 
        {
	  auto varname = var_tbl($1);
	  if (varname == nullptr)
	    {
	      stringstream s;
	      s << "var name " << $1 << " not found";
	      $$ = new ErrorExp(s.str());
	    }
	  else
	    $$ = varname;
	}
;

%%

void yyerror(char const * s) 
{
  cout << "ERROR " << s << " " << endl;
}


ExecStatus Load::execute()
{
  stringstream s;

  auto r = name_exp->execute();
  if (not r.first)
    return make_pair(false, r.second);

  string file_name;
  if (name_exp->type == Exp::STRCONST)
    file_name = static_cast<StringExp*>(name_exp)->value;
  else if (name_exp->type == Exp::VAR)
    {
      Varname * varname = static_cast<Varname*>(name_exp);
      Var * var = varname->get_value_ptr();
      if (var == nullptr)
	{
	  s << "Var " << varname->name << " has not a type associated" << endl
	    << "THIS PROBABLY IS AN REPL ERROR. PLEASE REPORT IT!";
	  return make_pair(false, s.str());
	}

      if (var->var_type != Var::VarType::String)
	{
	  s << s << "Current type of var " << varname->name << " is not string";
	  return make_pair(false, s.str());
	}
      file_name = static_cast<VarString*>(var)->value;
    }

  ifstream in(file_name);
  if (in.fail())
    {
      s << "cannot open file " << file_name;
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
  auto result = right_side->execute();
  if (not result.first)
    {
      cout << result.second << endl;
      return make_pair(false, result.second);
    }

  Varname * left_side = var_tbl(left_name);
  if (left_side == nullptr)
    left_side = var_tbl.addvar(left_name, new Varname(left_name));
  switch (right_side->type)
    {
    case Exp::Type::GRAPH:
      {
      again_graph:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarMap;
	    left_side->set_value_ptr(var);
	  }
	
	if (var->var_type != Var::VarType::Map)
	  {
	    left_side->free_value();
	    goto again_graph;
	  }
	static_cast<VarMap*>(var)->value = 
	  move(static_cast<Load*>(right_side)->mapa);
	return make_pair(true, "");
      }
    case Exp::Type::STRCONST:
      {
      again_string:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarString;
	    left_side->set_value_ptr(var);
	  }

	if (var->var_type != Var::VarType::String)
	  {
	    left_side->free_value();
	    goto again_string;
	  }
	static_cast<VarString*>(var)->value = 
	  move(static_cast<StringExp*>(right_side)->value);
	return make_pair(true, "");
      }
    case Exp::Type::INTCONST:
      {
      again_int:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarInt;
	    left_side->set_value_ptr(var);
	  }

	if (var->var_type != Var::VarType::Int)
	  {
	    left_side->free_value();
	    goto again_int;
	  }
	static_cast<VarInt*>(var)->value = 
	  move(static_cast<IntExp*>(right_side)->value);
	return make_pair(true, "");
      }
    case Exp::Type::VAR:
      {
	auto rvalue = static_cast<Varname*>(right_side)->get_value_ptr();
	if (rvalue == nullptr)
	  {
	    stringstream s;
	    s << "rvalue name " << static_cast<Varname*>(right_side)->name 
	      << " has not associated a value" << endl
	      << "THIS PROBABLY IS A BUG. SO, PLEASE REPORT IT";
	    return make_pair(false, s.str());
	  }

	
	auto lvalue = left_side->get_value_ptr();
	if (lvalue == nullptr)
	  left_side->set_value_ptr(lvalue = rvalue->clone());
	else if (lvalue->var_type != rvalue->var_type)
	  {
	    left_side->free_value();
	    left_side->set_value_ptr(lvalue = rvalue->clone());
	  }
	assert(lvalue->var_type == rvalue->var_type);
	lvalue->copy(rvalue);
	break;
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

ExecStatus Ls::execute()
{
  var_tbl.for_each([] (auto p) { cout << p.first << " "; });
  cout << endl
       << endl
       << "    " << var_tbl.size() << " vars " << endl;
  return make_pair(true, "");
}

ExecStatus Rm::execute()
{
  names.for_each([] (auto name) 
		 {
		   try
		     {
		       var_tbl.rmvar(name);
		     }
		   catch (domain_error)
		     {
		       cout << "var name " << name << " not found" << endl;
		     }
		 });
  return make_pair(true, "");
}

ExecStatus SearchProducer::set_mapa() 
{
  Varname * mapa = var_tbl(mapa_name);
  if (mapa == nullptr)
    {
      stringstream s;
      s << "Map var " << mapa_name << " not found";
      return make_pair(false, s.str());
    }

  mapa_ptr = &static_cast<VarMap*>(mapa->get_value_ptr())->value;

  return make_pair(true, "");
}


ExecStatus SearchProducerId::execute()
{
  auto r = set_mapa();
  if (not r.first)
    return make_pair(false, r.second);

  stringstream s;

  Uid id;
  switch (int_exp->type)
    {
    case INTCONST: 
      id = static_cast<IntExp*>(int_exp)->value;
      break;
    case VAR:
      {
	Varname * varname = static_cast<Varname*>(int_exp);
	VarInt * value = static_cast<VarInt*>(varname->get_value_ptr());
	if (value == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }      
	if (value->var_type != Var::VarType::Int)
	  {
	    s << "var name " << varname->name 
	      << " does not correspond to an interger";
	    return make_pair(false, s.str());
	  }
	id = value->value;
	break;
      }
    default:
      return make_pair(false, "id in search producer id is not an integer");
    }
  
  // TODO: terminar, no esta lista
}
