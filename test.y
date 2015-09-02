%{

# define YYDEBUG 0

# include <iostream>  
# include <tpl_components.H>
# include <generate_graph.H>
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

%token LOAD SAVE EXIT ERROR INFO LS RM SEARCH PRODUCER PRODUCERS LIST APPEND
%token PRODUCT ID REGEX HELP COD TYPEINFO RIF NODE REACHABLE COVER DOT UPSTREAM
%token <symbol> STRCONST INTCONST VARNAME 

%type <expr> exp
%type <expr> rvalue
%type <expr> ref_exp cmd_unit help_exp
%type <node_list> cmd_list line
%type <rmexp> rm_list
%type <search_exp> search_cmd
%type <exp_list> item_list

%%

line: /* empty line */ { $$ = line_commands = nullptr; }
    | cmd_list '\n' { $$ = line_commands = $1; }
    | cmd_list      { $$ = line_commands = $1; }
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
        | INFO VARNAME                     { $$ = new Info($2); }
        | VARNAME                          { $$ = new Info($1); }
        | INFO VARNAME '[' ref_exp ']'     { $$ = new Info($2, $4); }
        | VARNAME '[' ref_exp ']'          { $$ = new Info($1, $3); }
        | TYPEINFO VARNAME                 { $$ = new TypeInfo($2); }
        | TYPEINFO VARNAME '[' ref_exp ']' { $$ = new TypeInfo($2, $4); }
        | VARNAME '=' exp                  { $$ = new Assign($1, $3); }
        | VARNAME '[' ref_exp ']' '=' exp  { $$ = new ListWrite($1, $3, $6); }
        | SAVE ref_exp ref_exp             { }
        | LS                               { $$ = new Ls(); }
        | RM rm_list                       { $$ = $2; }
        | APPEND VARNAME item_list { $$ = new Append($2, $3); delete $3; }
        | search_cmd { $$ = $1; }
        | help_exp { $$ = $1; }
        | REACHABLE VARNAME ref_exp ref_exp { $$ = new Connected($2, $3, $4); }
        | DOT VARNAME ref_exp { $$ = new Dot($2, $3); }
;

help_exp: HELP           { $$ = new Help; }
        | HELP LOAD      { $$ = new Help(Exp::Type::MAP); }
        | HELP SEARCH    { $$ = new Help(Exp::Type::SEARCHPRODUCER); }
        | HELP TYPEINFO  { $$ = new Help(Exp::Type::TYPEINFO); }
        | HELP INFO      { $$ = new Help(Exp::Type::INFO); }
        | HELP REACHABLE { $$ = new Help(Exp::Type::REACHABLE); }
        | HELP COVER     { $$ = new Help(Exp::Type::COVER); }
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
          | SEARCH PRODUCERS VARNAME ref_exp
	    {
	      $$ = new SearchProducerRegexCmd($3, $4);
	    }
          | SEARCH PRODUCT ID VARNAME ref_exp
            {
	      $$ = new SearchProductIdCmd($4, $5);
	    }
          | SEARCH PRODUCT REGEX VARNAME ref_exp
	    {
	      $$ = new SearchProductsRegexCmd($4, $5);
	    }
          | SEARCH PRODUCT COD VARNAME ref_exp
	    {
	      $$ = new SearchProductsCodCmd($4, $5);
	    }
          | SEARCH PRODUCT RIF VARNAME ref_exp
	    {
	      $$ = new SearchProductsRifCmd($4, $5);
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
    | SEARCH PRODUCERS VARNAME ref_exp
      {
	$$ = new SearchProducerRegex($3, $4);
      }
    | SEARCH PRODUCT ID VARNAME ref_exp
      {
	$$ = new SearchProductId($4, $5);
      }
    | SEARCH PRODUCT REGEX VARNAME ref_exp
      {
	$$ = new SearchProductsRegex($4, $5);
      }
    | SEARCH PRODUCT COD VARNAME ref_exp
      {
	$$ = new SearchProductsCod($4, $5);
      }
    | SEARCH PRODUCT RIF VARNAME ref_exp
      {
	$$ = new SearchProductsRif($4, $5);
      }
    | SEARCH NODE VARNAME ref_exp
      {
	$$ = new SearchNode($3, $4);
      }
    | COVER VARNAME VARNAME
      {
	$$ = new Cover($2, $3);
      }
    | UPSTREAM VARNAME VARNAME ref_exp
      {
	$$ = new Upstream($2, $3, $4);
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
// TODO: regla para leer listas y de ser posibe que sea recursiva
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
    return r;

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
    return result;

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
	auto prod_var = static_cast<VarProducer*>(var);
	auto prod_search = static_cast<SearchProducerRif*>(right_side);
	prod_var->mapa_ptr = prod_search->mapa_ptr;
	prod_var->productor = *prod_search->producer_ptr;
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::SEARCHPRODUCERREGEX:
      {
      again_search_regex:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarList;
	    left_side->set_value_ptr(var);
	  }
	else
	  {
	    left_side->free_value();
	    goto again_search_regex;
	  }
	auto search_exp = static_cast<SearchProducerRegex*>(right_side);
	search_exp->producers.for_each([var] (auto ptr)
          {
	    auto v = new VarProducer(*const_cast<Productor*>(ptr));
	    static_cast<VarList*>(var)->list.append(v);
	  });
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::SEARCHPRODUCTID:
      {
      again_search_product:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarProduct;
	    left_side->set_value_ptr(var);
	  }
	if (var->var_type != Var::VarType::Product)
	  {
	    left_side->free_value();
	    goto again_search_product;
	  }
	auto search_exp = static_cast<SearchProductId*>(right_side);
	static_cast<VarProduct*>(var)->product = search_exp->producto;
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::SEARCHPRODUCTREGEX:
    case Exp::Type::SEARCHPRODUCTCOD:
    case Exp::Type::SEARCHPRODUCTRIF:
      {
      again_search_products:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarList;
	    left_side->set_value_ptr(var);
	  }
	else
	  {
	    left_side->free_value();
	    goto again_search_products;
	  }
	auto search_exp = static_cast<SearchProducts*>(right_side);
	search_exp->productos.for_each([var] (auto ptr)
          {
	    auto v = new VarProduct(*ptr);
	    static_cast<VarList*>(var)->list.append(v);
	  });
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::SEARCHNODE:
      {
      again_search_node:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarNode;
	    left_side->set_value_ptr(var);
	  }
	else
	  {
	    left_side->free_value();
	    goto again_search_node;
	  }
	auto search_exp = static_cast<SearchNode*>(right_side);
	auto varnode = static_cast<VarNode*>(var);
	varnode->mapa_ptr = search_exp->mapa_ptr;
	varnode->net_ptr = &search_exp->mapa_ptr->net;
        varnode->node_ptr = search_exp->node_ptr;
	delete right_side;
	return make_pair(true, "");
      }
    case Exp::Type::COVER:
    case Exp::Type::UPSTREAM:
      {
	again_cover:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarCover;
	    left_side->set_value_ptr(var);
	  }
	else
	  {
	    left_side->free_value();
	    goto again_cover;
	  }
	auto cover_exp = static_cast<Cover*>(right_side);
	auto varcover = static_cast<VarCover*>(var);
	varcover->mapa_ptr = cover_exp->mapa_ptr;
	varcover->net = move(cover_exp->net);
	delete right_side;
	return make_pair(true, "");
	break;
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
  if (index_exp)
    {
      auto listread = new ListRead(name, index_exp);
      auto r = listread->execute(); // si es ok ==> index_exp es liberado
      if (not r.first)
	{
	  delete listread;
	  return r;
	}
      cout << (*listread->val)->info() << endl;
      delete listread;
      return make_pair(true, "");
    }

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

static pair<ExecStatus, MetaMapa*> semant_mapa(const string & mapa_name)
{
  stringstream s;
  Varname * mapa = var_tbl(mapa_name);
  if (mapa == nullptr)
    {
      s << "Map var " << mapa_name << " not found";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  VarMap * ptr = static_cast<VarMap*>(mapa->get_value_ptr());
  if (ptr== nullptr)
    {
      s << "Map var"  << mapa_name << " has not a associated value"
	<< "THIS PROBABLY IS A BUG. PLEASE REPORT IT";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  MetaMapa * mapa_ptr = &ptr->value;

  return make_pair(make_pair(true, ""), mapa_ptr);
}

ExecStatus Search::semant_mapa() 
{
  auto p = ::semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;

  mapa_ptr = p.second;

  return make_pair(true, "");
}

static pair<ExecStatus, string> semant_string(Exp * str_exp)
{
  auto r = str_exp->execute();
  if (not r.first)
    return make_pair(r, "");

  stringstream s;
  string str;
  switch (str_exp->type)
    {
    case Exp::STRCONST: 
      str = static_cast<StringExp*>(str_exp)->value;
      break;
    case Exp::VAR:
      {
	Varname * varname = static_cast<Varname*>(str_exp);
	VarString * value = static_cast<VarString*>(varname->get_value_ptr());
	if (value == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(make_pair(false, s.str()), "");
	  }      
	if (value->var_type != Var::VarType::String)
	  {
	    s << "var name " << varname->name 
	      << " does not correspond to an string";
	    return make_pair(make_pair(false, s.str()), "");
	  }
	str = value->value;
	break;
      }
    default:
      return make_pair(make_pair(false, "expression is not a string"), "");
    }
  return make_pair(make_pair(true, ""), str);
}

ExecStatus Search::semant_string() 
{
  auto r = semant_mapa();
  if (not r.first)
    return r;
  assert(mapa_ptr != nullptr);

  auto res = ::semant_string(exp);
  if (not res.first.first)
    return res.first;

  str = res.second;

  return make_pair(true, "");
}

ExecStatus SearchProducerRif::semant()
{
  auto r = semant_string();
  if (not r.first)
    return r;
  
  cout << "Searching " << str << endl; 
  producer_ptr = mapa_ptr->tabla_productores(str);
  if (producer_ptr == nullptr)
    return make_pair(false, "Rif " + str + " not found");
  free();
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
	return r;
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
    return r;

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
    return r;

  r = rexp->execute();
  if (not r.first)
    return r;

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
  auto r = semant();
  if (not r.first)
    return r;

  assert(producer_ptr != nullptr);

  cout << *producer_ptr << endl;

  return make_pair(true, "");
}

ExecStatus SearchProducerRegex::semant()
{
  auto r = semant_string();
  if (not r.first)
    return r;
  
  try
    {
      producers = mapa_ptr->producers_by_name(str);
    }
  catch (regex_error & e)
    {
      stringstream s;
      s << "Regular expression " << str << " " << e.what();
      return make_pair(false, s.str());
    }
  
  free();
  return make_pair(true, "");
}

ExecStatus SearchProducerRegexCmd::execute()
{
 auto r = semant();
 if (not r.first)
   return r;
 
 if (producers.is_empty())
   {
     cout << "Not matches found" << endl;
     return make_pair(true, "");
   }

 producers.for_each([] (auto p)
   {
     cout << *p << endl;
   });

 return make_pair(true, "");
}

ExecStatus Search::semant_int()
{
  auto r = semant_mapa();
  if (not r.first)
    return r;
  assert(mapa_ptr != nullptr);
  
  r = exp->execute();
  if (not r.first)
    return r;

  switch (exp->type)
    {
    case Exp::Type::INTCONST:
      id = static_cast<IntExp*>(exp)->value;
      break;
    case Exp::Type::VAR:
      {
	stringstream s;
	auto varname = static_cast<Varname*>(exp);
	auto var = varname->get_value_ptr();
	if (var == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }
	if (var->var_type != Var::VarType::Int)
	  {
	    s << "Var name " << varname->name << " is not an integer";
	    return make_pair(false, s.str());
	  }
	id = static_cast<VarInt*>(var)->value;
	break;
      }
    default: 
      ERROR("SearchProductId::execute() invalid type %ld", exp->type);
    }

  return make_pair(true, "");
}

ExecStatus SearchProductId::semant()
{
  auto r = semant_int();
  if (not r.first)
    return r;

  assert(mapa_ptr != nullptr);

  stringstream s;
  auto ptr = mapa_ptr->tabla_productos(id);
  if (ptr == nullptr)
    {
      s << "product id " << id << " not found";
      return make_pair(false, s.str());
    }
  
  producto = *ptr;
  free();
  return make_pair(true, "");
}

ExecStatus SearchProductIdCmd::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  cout << producto << endl;

  return make_pair(true, "");
}

ExecStatus SearchProductsRegex::semant()
{
  auto r = semant_string();
  if (not r.first)
    return r;

  try
    {
      productos = mapa_ptr->productos_by_nom(str);
    }
  catch (regex_error & e)
    {
      stringstream s;
      s << "Invalid regex " << str << " " << e.what() << endl;
      return make_pair(false, s.str());
    }
 
  free();
  return make_pair(true, "");
}

ExecStatus SearchProductsRegexCmd::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  if (productos.is_empty())
    cout << "Empty" << endl;
  else
    productos.for_each([] (auto p) { cout << *p << endl; });

  return make_pair(true, "");
}

ExecStatus SearchProductsCod::semant()
{
  auto r = semant_string();
  if (not r.first)
    return r;

  productos = mapa_ptr->productos_by_cod_aran(str);
  free();
  return make_pair(true, "");
}

ExecStatus SearchProductsCodCmd::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  if (productos.is_empty())
    cout << "Empty" << endl;
  else
    productos.for_each([] (auto p) { cout << *p << endl; });

  return make_pair(true, "");
}

ExecStatus SearchProductsRif::semant()
{
  auto r = semant_string();
  if (not r.first)
    return r;

  productos = mapa_ptr->productos_by_rif(str);
  free();
  return make_pair(true, "");
}

ExecStatus SearchProductsRifCmd::execute()
{
   auto r = semant();
  if (not r.first)
    return r;

  if (productos.is_empty())
    cout << "Empty" << endl;
  else
    productos.for_each([] (auto p) { cout << *p << endl; });

  return make_pair(true, "");
}

ExecStatus TypeInfo::execute()
{
  if (index_exp)
    {
      auto listread = new ListRead(name, index_exp);
      auto r = listread->execute();
      if (not r.first)
	{
	  delete listread;
	  return r;
	}
      cout << (*listread->val)->type_info() << endl;
      delete listread;
      delete index_exp;
      return make_pair(true, "");
    }

  Varname * varname = var_tbl(name);
  if (varname == nullptr)
    {
      cout << "Var name " << name << " not found" << endl;
      return make_pair(true, "");
    }

  auto var = varname->get_value_ptr();
  if (var == nullptr)
    {
      stringstream s;
      s << "var name " << varname->name << " has not a value" << endl
	<< "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
      return make_pair(false, s.str());
    }

  cout << var->type_info() << endl;

  return make_pair(true, "");
}

ExecStatus SearchNode::execute()
{
  auto r = semant_mapa();
  if (not r.first)
    return r;
  assert(mapa_ptr != nullptr);

  r = exp->execute();
  if (not r.first)
    return r;

  stringstream s;
  switch (exp->type)
    {
    case Exp::STRCONST:
      {
	const string & rif = static_cast<StringExp*>(exp)->value;
	node_ptr = mapa_ptr->search_node(rif);
	if (node_ptr == nullptr)
	  {
	    s << "Rif " << rif << " not found as node (maybe it is in the map)";
	    return make_pair(false, s.str());
	  }
	break;
      }
    case Exp::VAR:
      {
	Varname * varname = static_cast<Varname*>(exp);
	Var * value = varname->get_value_ptr();
	if (value == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }
	switch (value->var_type)
	  {
	  case Var::VarType::Producer: 
	    {
	      VarProducer * var_producer = static_cast<VarProducer*>(value);
	      node_ptr = mapa_ptr->search_node(var_producer->productor);
	      if (node_ptr == nullptr)
		return make_pair(false, "Producer not found");
	      break;
	    }
	  case Var::VarType::String: 
	    {
	      VarString * var_str = static_cast<VarString*>(value);
	      node_ptr = mapa_ptr->search_node(var_str->value);
	      if (node_ptr == nullptr)
		return make_pair(false, "Producer not found");
	      break;
	    }
	  default:
	    s << "Producer operand is not a producer var";
	    return make_pair(false, s.str());
	  }
	break;
      }
    default:
      s << "Producer operand is not a producer var";
      return make_pair(false, s.str());
    }

  free();  
  return make_pair(true, "");
}

ExecStatus Connected::semant_exp(Exp * exp, Net::Node *& ptr)
{
  assert(mapa_ptr);
  auto r = exp->execute();
  if (not r.first)
    return r;
  
  stringstream s;
  switch (exp->type)
    {
    case Exp::STRCONST:
      {
	const string & rif = static_cast<StringExp*>(exp)->value;
	ptr = mapa_ptr->search_node(rif);
	if (ptr == nullptr)
	  {
	    s << "Rif " << rif << " is not associated to a node" << endl
	      << "which it would not mean that it does not exists as" << endl
	      << "economical unit associated to this rif";
	    return make_pair(false, s.str());
	  }
	break;
      }
    case Exp::VAR:
      {
	auto varname = static_cast<Varname*>(exp);
	Var * var = varname->get_value_ptr();
	if (var == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }
	switch (var->var_type)
	  {
	  case Var::VarType::String:
	    {
	      const string & rif = static_cast<VarString*>(var)->value;
	      ptr = mapa_ptr->search_node(rif);
	      if (ptr == nullptr)
		{
		  s << "Rif " << rif << " is not associated to a node" << endl
		    << "which it would not mean that it does not exists" << endl
		    << "as economical unit associated to this rif";
		  return make_pair(false, s.str());
		}
	      break;
	    }
	  case Var::VarType::Node: 
	    {
	      auto varnode = static_cast<VarNode*>(var);
	      if (varnode->mapa_ptr != mapa_ptr)
		{
		  s << "Var node " << varname->name << " belongs to another map"
		    << endl;
		  return make_pair(false, s.str());
		}
	      ptr = varnode->node_ptr;
	      break;
	    }
	  case Var::VarType::Producer:
	    {
	      auto prod_var = static_cast<VarProducer*>(var);
	      if (prod_var->mapa_ptr != mapa_ptr)
		{
		  s << "Var node " << varname->name << " belongs to another map"
		    << endl;
		  return make_pair(false, s.str());
		}
	      const string & rif = prod_var->productor.rif;
	      ptr = mapa_ptr->search_node(rif);
	      if (ptr == nullptr)
		{
		  s << "Rif " << rif << " is not associated to a node" << endl
		    << "which it would not mean that it does not exists" << endl
		    << "as economical unit associated to this rif";
		  return make_pair(false, s.str());
		}
	      break;
	    }
	  default:
	    s << "var name " << varname->name << " is not a valid type" << endl
	      << "Here variables must be of type string, node or producer";
	    return make_pair(false, s.str());
	  }
	break;
      }
    default:
      s << "Invalid expression for node. It must be a node" << endl
	<< "producer var or a string var or constant containing a rif";
      return make_pair(false, s.str());
    }

  return make_pair(true, "");
}

ExecStatus Connected::execute()
{
  auto p = ::semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;
  
  mapa_ptr = p.second;

  auto r = semant_exp(src_exp, src);
  if (not r.first)
    return r;

  r = semant_exp(tgt_exp, tgt);
  if (not r.first)
    return r;

  if (mapa_ptr->reachable(src, tgt))
    cout << "Nodes are reachable" << endl;
  else
    cout << "Nodes are not reachable" << endl;

  if (src_exp->type != VAR)
    delete src_exp;
  if (tgt_exp->type != VAR)
    delete tgt_exp;
  return make_pair(true, "");
}

ExecStatus SubNet::semant()
{
  auto p = ::semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;
  
  mapa_ptr = p.second;
  
  stringstream s;
  auto varname = var_tbl(node_name);
  if (varname == nullptr)
    {
      s << "Var node: " << node_name << " not found";
      return make_pair(false, s.str());
    }
  auto var = static_cast<VarNode*>(varname->get_value_ptr());
  if (var->var_type != Var::VarType::Node)
    {
      s << "Var node: " << varname->name << " is not a node";
      return make_pair(false, s.str());
    }

  src = var->node_ptr;

  return make_pair(true, "");
}

ExecStatus Cover::execute()
{
  auto r = semant();
  if (not r.first)
    return r;
  
  cout << "Building cover graph from " << *src->get_info() << endl;
  net = Build_Subgraph<Net>()(mapa_ptr->net, src);

  return make_pair(true, "");
}

ExecStatus Upstream::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  stringstream s;
  r = productor_exp->execute();
  if (not r.first)
    return r;

  switch (productor_exp->type)
    {
    case INTCONST: 
      {
	Uid id = static_cast<IntExp*>(productor_exp)->value;
	producto_ptr = mapa_ptr->tabla_productos(id);
	if (producto_ptr == nullptr)
	  {
	    s << "Product id " << id << " not found";
	    return make_pair(false, s.str());
	  }
	break;
      }
    case STRCONST:
      {
	// puede ser un codigo arancelario
      }
    case VAR:
      {

      }
    default:
      s << "Upstream> productor exp is invalid. This is possibly a buf" << endl
	<< "Please report it";
      return make_pair(false, s.str());
    }

  auto productor_ptr = src->get_info();
  const string & cod_aran = producto_ptr->cod_aran;
  if (not productor_ptr->productos.exists([&cod_aran] (auto p)
					  {
					    return p.second == cod_aran;
					  }))
    {
      s << "Codigo arancelario de producto " << producto_ptr->id << " "
	<< cod_aran << " (" << producto_ptr->nombre << ")" << endl
	<< "no está entre los productos del productor " << node_name;
      return make_pair(false, s.str());
    }
  
  cout << "Building upstream net from " << *src->get_info() << endl;
  net = mapa_ptr->upstream(src, producto_ptr);

  free();
  return make_pair(true, "");
}

ExecStatus Dot::execute()
{
  stringstream s;
  auto varname = var_tbl(net_name);
  if (varname == nullptr)
    {
      s << "Var net " << net_name << " not found";
      return make_pair(false, s.str());
    }

  auto varcover = static_cast<VarCover*>(varname->get_value_ptr());
  if (varcover->var_type != Var::VarType::Cover)
    {
      s << "Var " << net_name << " is not a net type";
      return make_pair(false, s.str());
    }

  net_ptr = &varcover->net;

  auto res = ::semant_string(file_exp);
  if (not res.first.first)
    return res.first;

  file_name = res.second;

  ofstream out(file_name);
  if (out.fail())
    {
      s << "cannot create file " << file_name;
      return make_pair(false, s.str());
    }

  Write_Arc warc(varcover->mapa_ptr->tabla_insumos);

  To_Graphviz<Net, Write_Node, Write_Arc>().digraph(*net_ptr, out, 
						    Write_Node(), warc);

  free();
  return make_pair(true, "");
}
