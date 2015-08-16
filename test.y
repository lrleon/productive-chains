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
  Search * search_exp;
  char * symbol;
  char * error_msg;
  Append * append;
  DynList<Exp*> * exp_list;
};

%token LOAD SAVE EXIT ERROR INFO LS RM SEARCH PRODUCER LIST APPEND
%token <symbol> STRCONST INTCONST VARNAME 

%type <expr> exp
%type <expr> rvalue
%type <expr> ref_exp cmd_unit
%type <node_list> cmd_list line
%type <rmexp> rm_list
%type <search_exp> search_cmd
%type <exp_list> item_list

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
	    // TODO: limpiar todo el ambiente. Podria ser desde una
	    // rutina. MEJOR: poner una variable bool en true para salir
	    // desde main()
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
	    $$ = new Assign($1, $3);
	  }
        | VARNAME '[' ref_exp ']' '=' exp
	  {
	    $$ = new ListWrite($1, $3, $6);
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
        | APPEND VARNAME item_list
	  {
	    $$ = new Append($2, $3);
	    delete $3;
	  }
        | search_cmd
	  {
	    $$ = $1;
	  }
;

item_list: ref_exp 
             {
	       auto l = new DynList<Exp*>;
	       l->append($1);
	       $$ = l;
	     }
         | item_list ref_exp 
	   {
	     auto l = $1;
	     l->append($2);
	     $$ = l;
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
;

search_cmd: SEARCH PRODUCER VARNAME ref_exp
            {
	      $$ = new SearchProducerRifCmd($3, $4);
	    }
;

exp : LOAD ref_exp
      {
	$$ = new Load(static_cast<StringExp*>($2));
      }
    | LIST
      {
	$$ = new ListExp;
      }
    | VARNAME '[' ref_exp ']'
      {
	$$ = new ListRead($1, $3);
      }
    | ref_exp
      {
	$$ = $1;
      }
    | SEARCH PRODUCER VARNAME ref_exp
      {
	$$ = new SearchProducerRif($3, $4);
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

  stringstream s;
  switch (right_side->type)
    {
    case Exp::Type::MAP:
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
	delete right_side;
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
	delete right_side;
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
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::SEARCHPRODUCER:
      {
      again_search:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarProducer;
	    left_side->set_value_ptr(var);
	  }

	if (var->var_type != Var::VarType::Producer)
	  {
	    left_side->free_value();
	    goto again_search;
	  }
	static_cast<VarProducer*>(var)->productor = 
	  move(*static_cast<SearchProducerRif*>(right_side)->producer_ptr);
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::VAR:
      {
	auto rvalue = static_cast<Varname*>(right_side)->get_value_ptr();
	if (rvalue == nullptr)
	  {
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
	return make_pair(true, "");
      }
    case Exp::Type::LIST:
      {
      again_list:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarList;
	    left_side->set_value_ptr(var);
	  }

	if (var->var_type != Var::VarType::List)
	  {
	    left_side->free_value();
	    goto again_list;
	  }
	// Originalmente la lista es vacia. Por eso aqui no hay asignacion
	return make_pair(true, "");
      }
    case Exp::Type::LISTREAD:
      {
	Var * rvalue = *static_cast<ListRead*>(right_side)->val;
	if (rvalue == nullptr)
	  {
	    s << "Entry list has not associated a value"
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
	return make_pair(true, "");
      }      
    default:
      s << "Assign " << right_side->type_string() << " = "
	<< left_side->type_string() << " not yet implemented";
      cout << s.str() << endl;
      return make_pair(false, s.str());
    };
  return make_pair(true, "");
}

ExecStatus Info::execute() 
{
  stringstream s;
  s << "var " << name << ": ";
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

ExecStatus Search::set_mapa() 
{
  stringstream s;
  Varname * mapa = var_tbl(mapa_name);
  if (mapa == nullptr)
    {
      s << "Map var " << mapa_name << " not found";
      return make_pair(false, s.str());
    }

  VarMap * ptr = static_cast<VarMap*>(mapa->get_value_ptr());
  if (ptr== nullptr)
    {
      s << "Map var"  << mapa_name << " has not a associated value"
	<< "THIS PROBABLY IS A BUG. PLEASE REPORT IT";
      return make_pair(false, s.str());
    }

  mapa_ptr = &ptr->value;

  return make_pair(true, "");
}

ExecStatus Search::semant() 
{
  auto r = set_mapa();
  if (not r.first)
    return make_pair(false, r.second);
  assert(mapa_ptr != nullptr);

  auto res = exp->execute();
  if (not res.first)
    return make_pair(false, res.second);

  stringstream s;

  switch (exp->type)
    {
    case STRCONST: 
      str = static_cast<StringExp*>(exp)->value;
      break;
    case VAR:
      {
	Varname * varname = static_cast<Varname*>(exp);
	VarString * value = static_cast<VarString*>(varname->get_value_ptr());
	if (value == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }      
	if (value->var_type != Var::VarType::String)
	  {
	    s << "var name " << varname->name 
	      << " does not correspond to an interger";
	    return make_pair(false, s.str());
	  }
	str = value->value;
	break;
      }
    default:
      return make_pair(false, "id in search producer id is not an integer");
    }
}

ExecStatus SearchProducerRif::compute()
{
  auto r = semant();
  if (not r.first)
    return make_pair(false, r.second);
  
  producer_ptr = mapa_ptr->tabla_productores(str);
  if (producer_ptr == nullptr)
    return make_pair(false, "Rif " + str + " not found");
  return make_pair(true, "");
}

ExecStatus Append::execute()
{
  stringstream s;
  Varname * varname = var_tbl(list_name);
  if (varname == nullptr)
    {
      s << "var name " << list_name << " not found";
      return make_pair(false, s.str());
    }

  VarList * varlist = static_cast<VarList*>(varname->get_value_ptr());
  if (varlist == nullptr)
    {
      s << "var name " << varname->name << " has not a value" << endl
	<< "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
      return make_pair(false, s.str());
    }

  if (varlist->var_type != Var::List)
    {
      s << "var name " << list_name << " is not a list";
      return make_pair(false, s.str());
    }

  for (auto it = rexp_list.get_it(); it.has_curr(); it.next())
    {
      auto rexp = it.get_curr();
      auto r = rexp->execute();
      if (not r.first)
	return make_pair(false, r.second);      
    }
  
  for (auto it = rexp_list.get_it(); it.has_curr(); it.next())
    {
      auto rexp = it.get_curr();
      switch (rexp->type)
	{
	case Exp::Type::STRCONST:
	  {
	    VarString * var = new VarString;
	    var->value = move(static_cast<StringExp*>(rexp)->value);
	    varlist->list.append(var);
	    delete rexp;
	    break;
	  }
	case Exp::Type::INTCONST:
	  {
	    VarInt * var = new VarInt;
	    var->value = move(static_cast<IntExp*>(rexp)->value);
	    varlist->list.append(var);
	    delete rexp;
	    break;
	  }
	case Exp::Type::VAR:
	  {
	    auto vname = static_cast<Varname*>(rexp);
	    auto rvalue = vname->get_value_ptr();
	    if (rvalue == nullptr)
	      {
		s << "right var name " << vname->name << " has not a value" 
		  << endl
		  << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
		return make_pair(false, s.str());
	      }
	    auto val = rvalue->clone();
	    val->copy(rvalue);
	    varlist->list.append(val);
	    break;
	  }
	default:
	  ERROR("Append::execute(): invalid expression type");
	}
    }
  
  return make_pair(true, "");
}

ExecStatus ListAccess::access()
{
  stringstream s;
  auto varname = var_tbl(list_name);
  if (varname == nullptr)
    {
      s << "Var name " << list_name << " not found";
      return make_pair(false, s.str());
    }

  auto var = static_cast<VarList*>(varname->get_value_ptr());
  if (var == nullptr)
    {
      s << "var name " << varname->name << " has not a value" << endl
	<< "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
      return make_pair(false, s.str());
    }
  if (var->var_type != Var::VarType::List)
    {
      s << "Var name " << list_name << " is not a list type";
      return make_pair(false, s.str());
    }

  auto r = index_exp->execute();
  if (not r.first)
    return make_pair(false, r.second);

  size_t i = 0;
  switch (index_exp->type)
    {
    case Exp::Type::INTCONST:
      i = static_cast<IntExp*>(index_exp)->value;
      break;
    case Exp::Type::VAR:
      {
	auto rval = static_cast<Varname*>(index_exp)->get_value_ptr();;
	if (rval->var_type != Var::VarType::Int)
	  {
	    s << "list index var name " << list_name << " is not an integer";
	    return make_pair(false, s.str());
	  }
	i = static_cast<VarInt*>(rval)->value;
	break;
      }
    default:
      s << "In " << list_name << "[exp]: exp is not an integer type";
      return make_pair(false, s.str());
    }
  
  try
    {
      val = &var->list.nth(i);
    }
  catch (out_of_range & e)
    {
      return make_pair(false, to_string(i) + " " + e.what());
    }

  delete index_exp;
  return make_pair(true, "");
}

ExecStatus ListRead::execute()
{
  return access();
}

ExecStatus ListWrite::execute()
{
  auto r = access();
  if (not r.first)
    return make_pair(false, r.second);

  r = rexp->execute();
  if (not r.first)
    return make_pair(false, r.second);

  assert(val != nullptr);
  delete *val;

  switch (rexp->type)
    {
    case Exp::Type::INTCONST:
      *val = new VarInt;
      static_cast<VarInt*>(*val)->value = static_cast<IntExp*>(rexp)->value;
      break;
    case Exp::Type::STRCONST:
      *val = new VarString;
      static_cast<VarString*>(*val)->value = 
	static_cast<StringExp*>(rexp)->value;
      break;
    case Exp::Type::VAR:
      {
	auto rval = static_cast<Varname*>(rexp)->get_value_ptr();
	auto new_val = rval->clone();
	new_val->copy(rval);
	*val = new_val;
      break;
      }
    default:
      ERROR("ListWrite::execute() invalid rvalue type");
    }

  delete rexp;
  return make_pair(true, "");
}

ExecStatus SearchProducerRifCmd::execute()
{
  auto r = compute();
  if (not r.first)
    return make_pair(false, r.second);

  assert(producer_ptr != nullptr);

  cout << *producer_ptr << endl;

  return make_pair(true, "");
}

ExecStatus SearchProducerRegex::compute()
{
  auto r = semant();
  if (not r.first)
    return make_pair(false, r.second);
  
  for (auto it = mapa_ptr->tabla_productores.get_it(); it.has_curr(); it.next())
    {

    }
  // producers = mapa_ptr->tabla_productores.filter([this] (auto p)
  //   {

  //   });
  // producer_ptr = mapa_ptr->tabla_productores(str);
  // if (producer_ptr == nullptr)
  //   return make_pair(false, "Rif " + str + " not found");
  return make_pair(true, "");
}
