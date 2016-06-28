%{

# define YYDEBUG 0

# include <iostream>
# include <tpl_graph_utils.H>
# include <tpl_components.H>
# include <generate_graph.H>
# include <tpl_find_path.H>
# include <topological_sort.H>
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
%token INPUTS ARCS OUTPUTS PATH INPUT OUTPUT RANKS SHAREHOLDER HOLDING DEMAND
%token <symbol> STRCONST INTCONST VARNAME ARC HEGEMONY PRODPLAN DOUBLECONST
%token PPDOT

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
	    cout << "Good bye!" << endl 
		 << endl;
	    exit(0);
	  }
        | DEMAND VARNAME ref_exp ref_exp   { $$ = new DemandCmd($2, $3, $4); }
        | PRODPLAN VARNAME ref_exp ref_exp ref_exp
	  {
            $$ = new ProdPlanCmd($2, $3, $4, $5);
          }
        | INFO VARNAME                     { $$ = new Info($2); }
        | VARNAME                          { $$ = new Info($1); }
        | INFO VARNAME '[' ref_exp ']'     { $$ = new Info($2, $4); }
        | VARNAME '[' ref_exp ']'          { $$ = new Info($1, $3); }
        | TYPEINFO VARNAME                 { $$ = new TypeInfo($2); }
        | TYPEINFO VARNAME '[' ref_exp ']' { $$ = new TypeInfo($2, $4); }
        | VARNAME '=' exp                  { $$ = new Assign($1, $3); }
        | VARNAME '[' ref_exp ']' '=' exp  { $$ = new ListWrite($1, $3, $6); }
        | SAVE VARNAME VARNAME ref_exp     { $$ = new Save($2, $3, $4); }
        | LS                               { $$ = new Ls(); }
        | RM rm_list                       { $$ = $2; }
        | APPEND VARNAME item_list { $$ = new Append($2, $3); delete $3; }
        | search_cmd { $$ = $1; }
        | help_exp { $$ = $1; }
        | REACHABLE VARNAME ref_exp ref_exp { $$ = new Connected($2, $3, $4); }
        | DOT VARNAME ref_exp { $$ = new Dot($2, $3); }
        | PPDOT VARNAME ref_exp { $$ = new PPDot($2, $3); }
        | INPUTS VARNAME VARNAME { $$ = new Inputs($2, $3); }
        | INPUTS VARNAME INTCONST 
	  {
	    auto id = atol(id_table($3));
	    $$ = new Inputs($2, id); 
	  }
        | ARCS INPUT ID VARNAME ref_exp { $$ = new ArcsInputId($4, $5); }
        | ARCS OUTPUT ID VARNAME ref_exp { $$ = new ArcsOutputId($4, $5); }
        | ARCS REGEX VARNAME ref_exp { $$ = new ArcsRegex($3, $4); }
        | ARCS VARNAME ref_exp { $$ = new Arcs($2, $3); }
        | ARCS INPUT VARNAME ref_exp ref_exp
	  {
	    $$ = new ArcsInputP($3, $4, $5); 
	  }
        | ARCS OUTPUT VARNAME ref_exp ref_exp
	  {
	    $$ = new ArcsOutputP($3,$4,$5); 
	  }
        | PATH VARNAME ref_exp ref_exp { $$ = new ComputePath($2, $3, $4); }
        | SEARCH SHAREHOLDER VARNAME ref_exp 
	  {
	    $$ = new ShareholderRif($3, $4); 
	  }
        | SEARCH SHAREHOLDER REGEX VARNAME ref_exp 
	  {
	    $$ = new ShareholderRegex($4,$5); 
	  }
        | SEARCH HOLDING VARNAME ref_exp { $$ = new HoldigRif($3, $4); }
        | SEARCH HOLDING REGEX VARNAME ref_exp 
	  {
	    $$ = new HoldingRegex($4, $5); 
	  }
        | SEARCH SHAREHOLDER HEGEMONY VARNAME ref_exp 
	  {
	    $$ = new Hegemony($4, $5);
	  }
        | RM ARC VARNAME ref_exp ref_exp 
	  {
	    $$ = new RmArcNodes($3, $4, $5); 
	  }
        | RM ARC VARNAME ref_exp { $$ = new RmArcId($3, $4); }
        | RM NODE VARNAME ref_exp { $$ = new RmNode($3, $4); }
;


help_exp: HELP           { $$ = new Help; }
        | HELP LOAD      { $$ = new Help(Exp::Type::MAP); }
        | HELP SEARCH    { $$ = new Help(Exp::Type::SEARCHPRODUCER); }
        | HELP TYPEINFO  { $$ = new Help(Exp::Type::TYPEINFO); }
        | HELP INFO      { $$ = new Help(Exp::Type::INFO); }
        | HELP REACHABLE { $$ = new Help(Exp::Type::REACHABLE); }
        | HELP COVER     { $$ = new Help(Exp::Type::COVER); }
        | HELP UPSTREAM  { $$ = new Help(Exp::Type::UPSTREAM); }
        | HELP INPUTS    { $$ = new Help(Exp::Type::INPUTS); }
        | HELP PATH      { $$  = new Help(Exp::Type::PATH); }
        | HELP DEMAND    { $$  = new Help(Exp::Type::DEMAND); }
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

exp : LOAD ref_exp { $$ = new Load(static_cast<StringExp*>($2)); }
    | LIST { $$ = new ListExp; }
    | VARNAME '[' ref_exp ']' { $$ = new ListRead($1, $3); }
    | ref_exp { $$ = $1; }
    | SEARCH PRODUCER VARNAME ref_exp { $$ = new SearchProducerRif($3, $4); }
    | SEARCH PRODUCERS VARNAME ref_exp { $$ = new SearchProducerRegex($3, $4); }
    | SEARCH PRODUCT ID VARNAME ref_exp { $$ = new SearchProductId($4, $5); }
    | SEARCH PRODUCT REGEX VARNAME ref_exp
      {
	$$ = new SearchProductsRegex($4, $5);
      }
    | SEARCH PRODUCT COD VARNAME ref_exp { $$ = new SearchProductsCod($4, $5); }
    | SEARCH PRODUCT RIF VARNAME ref_exp { $$ = new SearchProductsRif($4, $5); }
    | SEARCH NODE VARNAME ref_exp { $$ = new SearchNode($3, $4); }
    | COVER VARNAME ref_exp { $$ = new Cover($2, $3); }
    | UPSTREAM VARNAME ref_exp ref_exp ref_exp
      {
	$$ = new UpstreamB($2, $3, $4, $5);
      }
    | RANKS VARNAME VARNAME { $$ = new RanksExp($2, $3); }
    | DEMAND VARNAME ref_exp ref_exp   { $$ = new Demand($2, $3, $4); }
    | PRODPLAN VARNAME ref_exp ref_exp ref_exp
      {
	$$ = new ProdPlan($2, $3, $4, $5);
      }
;

ref_exp : STRCONST
            {
	      assert(string_table($1));
	      auto symbol = string_table($1);
	      $$ = new StringExp(symbol);
	    }
          | INTCONST
	    {
	      assert(id_table($1) == $1);
	      auto symbol = id_table($1);
	      $$ = new IntExp(symbol);
	    }
          | DOUBLECONST
	  {
	    assert(id_table($1) == $1);
	    auto symbol = id_table($1);
	    $$ = new DoubleExp(symbol);
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
  cout << "Sintax error" << endl
       << endl
       << "Make sure that you are not using as identifier any of the" << endl
       << "following reserved words:" << endl
       << "LOAD SAVE EXIT INFO LS RM SEARCH PRODUCER PRODUCERS PRODUCT" << endl
       << "ID REGEX LIST APPEND HELP COD TYPE RIF NODE REACHABLE COVER" << endl
       << "DOT UPSTREAM INPUTS OUTPUTS INPUT OUTPUT ARCS PATH RANKS" << endl
       << "SHAREHOLDER DEMAND PRODPLAN PPDOT" << endl
       << endl;
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
	    << "THIS PROBABLY IS AN REPL AH_ERROR. PLEASE REPORT IT!";
	  return make_pair(false, s.str());
	}

      if (var->var_type != Var::VarType::String)
	{
	  s << "Current type of var " << varname->name << " is not string";
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

ExecStatus Assign::execute()
{
  auto result = right_side->execute();
  if (not result.first)
    {
      right_side->free();
      return result;
    }

  Varname * left_side = var_tbl(left_name);
  if (left_side == nullptr)
    left_side = var_tbl.addvar(left_name, new Varname(left_name));

  stringstream s;
  switch (right_side->type)
    {
    case Exp::Type::DEMAND:
      {
	auto var = new VarDemandResult;
	left_side->set_value_ptr(var);
	return make_pair(true, "");
      }
    case Exp::Type::PRODPLAN:
      {
	auto var = left_side->get_value_ptr();

	if (var == nullptr)
	  {
	    var = new VarProdPlan;
	    left_side->set_value_ptr(var);
	  }

	var->copy(&static_cast<ProdPlan*>(right_side)->result);
	delete right_side;
	return make_pair(true, "");
      }
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
	    var = new VarNet;
	    left_side->set_value_ptr(var);
	  }
	else
	  {
	    left_side->free_value();
	    goto again_cover;
	  }
	auto cover_exp = static_cast<Cover*>(right_side);
	auto varnet = static_cast<VarNet*>(var);
	varnet->mapa_ptr = cover_exp->mapa_ptr;
	varnet->net = move(cover_exp->net);
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
    case Exp::Type::RANKS:
      {
	again_ranks:
	auto var = left_side->get_value_ptr();
	if (var == nullptr)
	  {
	    var = new VarRanks;
	    left_side->set_value_ptr(var);
	  }
	if (var->var_type != Var::VarType::Ranks)
	  {
	    left_side->free_value();
	    goto again_ranks;
	  }
	auto & rvalue = static_cast<RanksExp*>(right_side)->ranks;
	static_cast<VarRanks*>(var)->ranks = move(rvalue);
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

static ExecStatus semant_producer(MetaMapa * mapa_ptr, 
				  Exp * producer_exp, 
				  Productor *& producer_ptr)
{
  auto r = producer_exp->execute();
  if (not r.first)
    return r;

  stringstream s;
  switch (producer_exp->type)
    {
    case Exp::STRCONST:
      {
	const auto & rif = static_cast<StringExp*>(producer_exp)->value;
	producer_ptr = mapa_ptr->tabla_productores(rif);
	if (producer_ptr == nullptr)
	  {
	    s << "producer rif " << rif << " not found";
	    return make_pair(false, s.str());
	  }
	break;
      }
    case Exp::VAR:
      {
	auto varname = static_cast<Varname*>(producer_exp);
	auto var = varname->get_value_ptr();
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
	      const auto & rif = static_cast<VarString*>(var)->value;
	      producer_ptr = mapa_ptr->tabla_productores(rif);
	      if (producer_ptr == nullptr)
		{
		  s << "producer rif " << rif << " not found";
		  return make_pair(false, s.str());
		}
	      break;
	    }
	  case Var::VarType::Producer:
	    producer_ptr = &static_cast<VarProducer*>(var)->productor;
	    break;
	  default:
	    s << "var " << varname->name << " is not a producer or string type";
	    return make_pair(false, s.str());
	  }
	break;
      }
    default:
      s << "Producer expression is not a string with a rif neither a "
	<< "producer var";
    return make_pair(false, s.str());
    }

  return make_pair(true, "");
}

static ExecStatus semant_product(MetaMapa * mapa_ptr, 
				 Exp * product_exp, 
				 MetaProducto *& product_ptr)
{
  auto r = product_exp->execute();
  if (not r.first)
    return r;

  stringstream s;
  switch (product_exp->type)
    {
    case Exp::INTCONST:
      {
	const auto id = static_cast<IntExp*>(product_exp)->value;
	product_ptr = mapa_ptr->tabla_productos(id);
	if (product_ptr == nullptr)
	  {
	    s << "product id " << id << " not found";
	    return make_pair(false, s.str());
	  }
	break;
      }
    case Exp::VAR:
      {
	auto varname = static_cast<Varname*>(product_exp);
	auto var = varname->get_value_ptr();
	if (var == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(false, s.str());
	  }
	switch (var->var_type)
	  {
	  case Var::VarType::Int:
	    {
	      const auto id = static_cast<VarInt*>(var)->value;
	      product_ptr = mapa_ptr->tabla_productos(id);
	      if (product_ptr == nullptr)
		{
		  s << "product id " << id << " not found";
		  return make_pair(false, s.str());
		}
	    break;
	    }
	  case Var::VarType::Product:
	    product_ptr = &static_cast<VarProduct*>(var)->product;
	    break;
	  default:
	    s << "var " << varname->name << " is not a product or integer type";
	    return make_pair(false, s.str());
	  }
	break;
      }
    default:
      s << "Producer expression is not a string with a rif neither a "
	<< "producer var";
    return make_pair(false, s.str());
    }

  return make_pair(true, "");
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

static pair<ExecStatus, VarNet*> semant_net(const string & net_name)
{
  stringstream s;
  auto varname = var_tbl(net_name);
  if (varname == nullptr)
    {
      s << "Var net " << net_name << " not found";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  auto varnet = static_cast<VarNet*>(varname->get_value_ptr());
  if (varnet->var_type != Var::VarType::Cover)
    {
      s << "Var " << net_name << " is not a net type";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  return make_pair(make_pair(true, ""), varnet);
}

static ExecStatus semant_mapa_or_net(const string & name, 
				     MetaMapa *& mapa_ptr, Net *& net_ptr)
{
  stringstream s;
  auto p = ::semant_mapa(name);
  if (p.first.first)
    {
      mapa_ptr = p.second;
      net_ptr = &mapa_ptr->net;
    }
  else 
    {
      auto rnet = semant_net(name);
      if (not rnet.first.first)
	{
	  s << name << " is not a name of map or net variable";
	  return make_pair(false, s.str());
	}
      net_ptr = &rnet.second->net;
      mapa_ptr = rnet.second->mapa_ptr;
    }
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
	  AH_ERROR("Append::execute(): invalid expression type");
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
      AH_ERROR("ListWrite::execute() invalid rvalue type");
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

 producers.for_each([] (auto p) { cout << *p << endl; });

 return make_pair(true, "");
}

static pair<ExecStatus, long> semant_int(Exp * int_exp)
{
  long id = -1;

  auto r = int_exp->execute();
  if (not r.first)
    return make_pair(r, id);

  switch (int_exp->type)
    {
    case Exp::Type::INTCONST:
      id = static_cast<IntExp*>(int_exp)->value;
      break;
    case Exp::Type::VAR:
      {
	stringstream s;
	auto varname = static_cast<Varname*>(int_exp);
	auto var = varname->get_value_ptr();
	if (var == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(make_pair(false, s.str()), id);
	  }
	if (var->var_type != Var::VarType::Int)
	  {
	    s << "Var name " << varname->name << " is not an integer";
	    return make_pair(make_pair(false, s.str()), id);
	  }
	id = static_cast<VarInt*>(var)->value;
	break;
      }
    default: 
      return make_pair(make_pair(false, 
				 "Expression type if not a integer type"), id);
    }

  return make_pair(make_pair(true, ""), id);
}

static pair<ExecStatus, double> semant_double(Exp * double_exp)
{
  double val = 0.0;

  auto r = double_exp->execute();
  if (not r.first)
    return make_pair(r, val);

  switch (double_exp->type)
    {
    case Exp::Type::DOUBLECONST:
      val = static_cast<DoubleExp*>(double_exp)->value;
      break;
    case Exp::Type::VAR:
      {
	stringstream s;
	auto varname = static_cast<Varname*>(double_exp);
	auto var = varname->get_value_ptr();
	if (var == nullptr)
	  {
	    s << "var name " << varname->name << " has not a value" << endl
	      << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	    return make_pair(make_pair(false, s.str()), val);
	  }
	if (var->var_type != Var::VarType::Double)
	  {
	    s << "Var name " << varname->name << " is not an double type";
	    return make_pair(make_pair(false, s.str()), val);
	  }
	val = static_cast<VarDouble*>(var)->value;
	break;
      }
    default: 
      return make_pair(make_pair(false, 
				 "Expression type if not a double type"), val);
    }

  return make_pair(make_pair(true, ""), val);
}

ExecStatus Demand::semant()
{
  auto r = ::semant_mapa_or_net(map_name, map_ptr, net_ptr);
  
  if (not r.first)
    return r;

  assert(map_ptr != nullptr);

  auto pres = semant_product(map_ptr, exp_id, product);
  
  if (not pres.first)
    return pres;
  
  auto qres = semant_int(exp_quantity);

  if (not qres.first.first)
    return qres.first;

  
  /* En este punto debo obtener el valor con el cual comparar la demanda
     para decidir si satisface o no para terminar el resto. */

  return r;
}

ExecStatus DemandCmd::execute()
{
  auto r = semant();
  
  if (not r.first)
    return r;
  
  cout << result.to_str() << endl;
  
  return make_pair(true, "");
}

ExecStatus ProdPlan::semant()
{
  auto r = ::semant_mapa_or_net(map_name, map_ptr, net_ptr);
  
  if (not r.first)
    return r;

  assert(map_ptr != nullptr);

  auto pres = semant_product(map_ptr, exp_id, product);
  
  if (not pres.first)
    return pres;
  
  auto qres = semant_int(exp_quantity);

  if (not qres.first.first)
    return qres.first;

  auto tres = semant_int(exp_threshold);

  if (not tres.first.first)
    return tres.first;

  assert(product != nullptr);
  result.pp = new ProdPlanGraph(map_ptr);

  try
    {
      result.pp->build_pp(product, qres.second, tres.second);
    }
  catch (const exception & e)
    {
      cout << "Exception caught with error message: " << endl
	   << e.what() << endl;
      return make_pair(false, "");
    }

  return make_pair(true, "");
};

ExecStatus ProdPlanCmd::execute()
{
  auto r = semant();
  
  if (not r.first)
    return r;

  cout << result.info() << endl;

  return make_pair(true, "");
}

ExecStatus Search::semant_int()
{
  auto r = semant_mapa();
  if (not r.first)
    return r;
  assert(mapa_ptr != nullptr);
  
  auto res = ::semant_int(exp);
  if (not res.first.first)
    return res.first;
  
  id = res.second;

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

static ExecStatus 
semant_node_exp(Exp * exp, MetaMapa * mapa_ptr, Net::Node *& ptr)
{
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

  auto r = semant_node_exp(src_exp, mapa_ptr, src);
  if (not r.first)
    return r;

  r = semant_node_exp(tgt_exp, mapa_ptr, tgt);
  if (not r.first)
    return r;

  if (mapa_ptr->reachable(src, tgt))
    cout << "Nodes are reachable" << endl;
  else
    cout << "Nodes are not reachable" << endl;

  free();
  return make_pair(true, "");
}

ExecStatus SubNet::semant()
{
  auto p = ::semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;
  
  mapa_ptr = p.second;
  
  auto rnode = semant_node_exp(node_exp, mapa_ptr, src);
  if (not rnode.first)
    return rnode;

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

ExecStatus UpstreamB::execute()
{
  auto r = SubNet::semant();
  if (not r.first)
    return r;

  auto rprod = semant_product(mapa_ptr, product_exp, product_ptr);
  if (not rprod.first)
    return rprod;

  auto rid = semant_int(threshold_exp);
  if (not rid.first.first)
    return rid.first;

  threshold = rid.second;

  cout << "Building upstream net from " << *src->get_info() << endl;
  net = mapa_ptr->upstream_best(src, product_ptr, threshold);
  cout << "done!";

  free();
  return make_pair(true, "");
}

static pair<ExecStatus, ProdPlanGraph*> semant_pp(const string & pp_name)
{
  stringstream s;
  Varname * pp = var_tbl(pp_name);
  if (pp == nullptr)
    {
      s << "Production plan var " << pp_name << " not found";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  VarProdPlan * ptr = static_cast<VarProdPlan*>(pp->get_value_ptr());
  if (ptr== nullptr)
    {
      s << "Prod plan"  << pp_name << " has not a associated value"
	<< "THIS PROBABLY IS A BUG. PLEASE REPORT IT";
      return make_pair(make_pair(false, s.str()), nullptr);
    }

  ProdPlanGraph * pp_ptr = ptr->pp;

  return make_pair(make_pair(true, ""), pp_ptr);
}

ExecStatus Dot::execute()
{
  auto r = semant_mapa_or_net(net_name, mapa_ptr, net_ptr);
  
  if (not r.first)
    return r;

  auto res = ::semant_string(file_exp);
  if (not res.first.first)
    return res.first;

  file_name = res.second;

  ofstream out(file_name);
  if (out.fail())
    {
      stringstream s;
      s << "cannot create file " << file_name;
      return make_pair(false, s.str());
    }

  Write_Arc warc(mapa_ptr->tabla_insumos);
  To_Graphviz<Net, Write_Node, Write_Arc>().digraph(*net_ptr, out, 
						    Write_Node(), warc, "LR");

  free();
  return make_pair(true, "");
}

ExecStatus PPDot::execute()
{
  auto r = semant_pp(prod_plan_name);
  
  if (not r.first.first)
    return r.first;

  auto res = ::semant_string(file_exp);
  if (not res.first.first)
    return res.first;

  file_name = res.second;

  ofstream out(file_name);
  if (out.fail())
    {
      stringstream s;
      s << "cannot create file " << file_name;
      return make_pair(false, s.str());
    }

  To_Graphviz<ProdPlanGraph>().digraph(*r.second, out);

  free();
  return make_pair(true, "");
}

ExecStatus Inputs::execute()
{
  auto res = semant_mapa(mapa_name);
  if (not res.first.first)
    return res.first;
  
  mapa_ptr = res.second;

  stringstream s;

  if (producto_id != -1)
    {
      assert(name == "");
      
      producto_ptr = mapa_ptr->tabla_productos(producto_id);
      if (producto_ptr == nullptr)
	{
	  s << "Product " << producto_id << " not found";
	  return make_pair(false, s.str());
	}      
      report_product();
      return make_pair(true, "");
    }

  auto varname = var_tbl(name);
  if (varname == nullptr)
    {
      s << "var " << name << " not found";
      return make_pair(false, s.str());
    }
  auto var = varname->get_value_ptr();
  if (var == nullptr)
    {
      s << "var name " << name << " has not a value" << endl
	<< "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
      return make_pair(false, s.str());
    }
  switch (var->var_type)
    {
    case Var::VarType::Node:
      node_ptr = static_cast<VarNode*>(var)->node_ptr;
      report_node();
      break;
    case Var::VarType::Int:
      producto_id = static_cast<VarInt*>(var)->value;
      producto_ptr = mapa_ptr->tabla_productos(producto_id);
      if (producto_ptr == nullptr)
	{
	  s << "Product " << producto_id << " not found";
	  return make_pair(false, s.str());
	}
      report_product();
      break;
    case Var::VarType::Product:
      producto_ptr = &static_cast<VarProduct*>(var)->product;
      report_product();
      break;
    default:
      s << "var " << name << " is not a product, node or integer type";
      return make_pair(false, s.str());
    }

  return make_pair(true, "");
}

void Inputs::report_product()
{
  assert(producto_ptr);

  in_place_sort(producto_ptr->comb, [] (auto p1, auto p2)
		{
		  return get<0>(p1) < get<0>(p2);
		});

  const string col0 = "insumo_id";
  const string col1 = "cod_aran";
  const string col2 = "cantidad";
  const string col3 = "arco_id";

  using Lens = tuple<size_t, size_t, size_t, size_t>;
  using Line = tuple<string, string, string, string>;

  DynList<Line> lines = producto_ptr->comb.map<Line>([] (auto c)
    {
      return make_tuple(to_string(get<0>(c)), get<1>(c), 
			to_string(get<2>(c)), to_string(get<3>(c)));
    });

  Lens lens = lines.foldl<Lens>(make_tuple(col0.size(), col1.size(), 
					   col2.size(), col3.size()),
				[] (auto acu, auto l)
    {
      return make_tuple(max(get<0>(acu), get<0>(l).size()),
			max(get<1>(acu), get<1>(l).size()),
			max(get<2>(acu), get<2>(l).size()),
			max(get<3>(acu), get<3>(l).size()));
    });

  cout << "Producto id = " << producto_ptr->id << endl
       << "cod_aran    = " << producto_ptr->cod_aran << endl
       << "Nombre      = " << producto_ptr->nombre << endl
       << "Inputs:" << endl
       << "    " << string(get<0>(lens) - col0.size(), ' ') << col0
       << " " << string(get<1>(lens) - col1.size(), ' ') << col1
       << " " << string(get<2>(lens) - col2.size(), ' ') << col2 
       << " " << string(get<3>(lens) - col3.size(), ' ') << col3 << endl; 
  lines.for_each([&lens] (auto l)
  {
    const string blanks0(get<0>(lens) - get<0>(l).size(), ' ');
    const string blanks1(get<1>(lens) - get<1>(l).size(), ' ');
    const string blanks2(get<2>(lens) - get<2>(l).size(), ' ');
    const string blanks3(get<3>(lens) - get<3>(l).size(), ' ');
    cout << "    " << blanks0 << get<0>(l) << blanks1 << " " << get<1>(l)
	 << blanks2 << " " << get<2>(l) << blanks3 << " " << get<3>(l) << endl;
  });
}

void Inputs::report_node()
{
  assert(node_ptr);

  using Line = tuple<string, string, string, string, string, string, string>;
  
  const string col0 = "arco_id";
  const string col1 = "cod_aran";
  const string col2 = "producto_id";
  const string col3 = "insumo_id";
  const string col4 = "factor";
  const string col5 = "cantidad";
  const string col6 = "coste";

  auto lines = mapa_ptr->net.in_arcs(node_ptr).map<Line>([] (auto a)
    {
      const auto & info = a->get_info();
      return make_tuple(to_string(info.arco_id), info.cod_aran, 
			to_string(info.producto_id), to_string(info.insumo_id),
			to_string(info.factor), to_string(info.cantidad),
			to_string(info.coste));
    });
  auto lens = lines.foldl(make_tuple(col0.size(), col1.size(), col2.size(),
				     col3.size(),col4.size(),col5.size(),
				     col6.size()), [] (auto acu, auto l)
    {
      return make_tuple(max(get<0>(acu), get<0>(l).size()),
			max(get<1>(acu), get<1>(l).size()),
			max(get<2>(acu), get<2>(l).size()),
			max(get<3>(acu), get<3>(l).size()),
			max(get<4>(acu), get<4>(l).size()),
			max(get<5>(acu), get<5>(l).size()),
			max(get<6>(acu), get<6>(l).size()));
    });

  auto productor_ptr = node_ptr->get_info();
  cout << "Node inputs reports" << endl
       << "Rif    = " << productor_ptr->rif << endl
       << "Nombre = " << productor_ptr->nombre << endl
       << "Input arcs:" << endl
       << "  " << string(get<0>(lens) - col0.size(), ' ') << col0
       << " " << string(get<1>(lens) - col1.size(), ' ') << col1
       << " " << string(get<2>(lens) - col2.size(), ' ') << col2
       << " " << string(get<3>(lens) - col3.size(), ' ') << col3
       << " " << string(get<4>(lens) - col4.size(), ' ') << col4
       << " " << string(get<5>(lens) - col5.size(), ' ') << col5
       << " " << string(get<6>(lens) - col6.size(), ' ') << col6 << endl;
  lines.for_each([&lens] (auto l)
    {
      const string blanks0(get<0>(lens) - get<0>(l).size(), ' ');
      const string blanks1(get<1>(lens) - get<1>(l).size(), ' ');
      const string blanks2(get<2>(lens) - get<2>(l).size(), ' ');
      const string blanks3(get<3>(lens) - get<3>(l).size(), ' ');
      const string blanks4(get<4>(lens) - get<4>(l).size(), ' ');
      const string blanks5(get<5>(lens) - get<5>(l).size(), ' ');
      const string blanks6(get<6>(lens) - get<6>(l).size(), ' ');
      cout << "  " << blanks0 << get<0>(l)
	   << " " << blanks1 << get<1>(l)
	   << " " << blanks2 << get<2>(l)
	   << " " << blanks3 << get<3>(l)
	   << " " << blanks4 << get<4>(l)
	   << " " << blanks5 << get<5>(l)
	   << " " << blanks6 << get<6>(l) << endl;
    });
}

ExecStatus Arc::semant_mapa()
{
  auto res = ::semant_mapa(mapa_name);
  if (not res.first.first)
    return res.first;
  
  mapa_ptr = res.second;

  return make_pair(true, "");
}

ExecStatus ArcsInt::semant()
{
  auto r = semant_mapa();
  if (not r.first)
    return r;

  auto res = ::semant_int(int_exp);
  if (not res.first.first)
    return res.first;

  id = res.second;

  free();
  return make_pair(true, "");
}

ExecStatus ArcsInputId::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  cout << "Searching arcs whose insumo_id == " << id << endl;
  arc_list = mapa_ptr->net.filter_arcs([this] (auto a)
    {
      return a->get_info().insumo_id == id; 
    });

  report();

  return make_pair(true, "");
}

ExecStatus Arcs::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  cout << "Searching arc whose arco_id == " << id << endl;
  arc_list = mapa_ptr->net.filter_arcs([this] (auto a)
    {
      return a->get_info().arco_id == id; 
    });

  report();

  return make_pair(true, "");
}

ExecStatus ArcsOutputId::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  cout << "Searching arc whose producto_id == " << id << endl;
  arc_list = mapa_ptr->net.filter_arcs([this] (auto a)
    {
      return a->get_info().producto_id == id; 
    });

  report();

  return make_pair(true, "");
}

ExecStatus ArcsProducer::semant()
{
  auto r = semant_mapa();
  if (not r.first)
    return r;

  r = semant_producer(mapa_ptr, producer_exp, producer_ptr);
  if (not r.first)
    return r;
  
  r = semant_product(mapa_ptr, product_exp, product_ptr);
  if (not r.first)
    return r;

  node_ptr = mapa_ptr->search_node(producer_ptr->rif);
  if (node_ptr == nullptr)
    {
      stringstream s;
      s << "No node is associated to producer " << producer_ptr->rif;
      return make_pair(false, s.str());
    }

  free();
  return make_pair(true, "");
}

ExecStatus ArcsInputP::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  arc_list = mapa_ptr->net.in_arcs(node_ptr);
  report();
  return make_pair(true, "");
}

ExecStatus ArcsOutputP::execute()
{
  auto r = semant();
  if (not r.first)
    return r;

  arc_list = mapa_ptr->net.out_arcs(node_ptr);
  report();
  return make_pair(true, "");
}

ExecStatus ArcsRegex::execute()
{
  auto res = semant_mapa();
  if (not res.first)
    return res;

  auto r = ::semant_string(str_exp);
  if (not r.first.first)
    return r.first;

  str = r.second;
  try
    {
      regex reg(str);
      arc_list = 
	mapa_ptr->net.filter_arcs([&reg, tbl = mapa_ptr->tabla_insumos] (auto a)
          {
	    return regex_search(tbl(a->get_info().insumo_id)->nombre, reg);
	  });
      free();
      return make_pair(true, "");
    }
  catch (regex_error & e)
    {
      return make_pair(false, e.what());
    }
}

static void arcs_report(MetaMapa * mapa_ptr, const DynList<Net::Arc*> & l)
{
  const string col0 = "rif";
  const string col1 = "ins_id";
  const string col2 = "arco_id";
  const string col3 = "nombre-insumo";
  const string col4 = "prod_id";
  const string col5 = "rif";

  using Line = tuple<string,string,string,string,string,string>;

  auto lines = l.map<Line>([mapa_ptr, tbl = mapa_ptr->tabla_insumos] (auto a)
    {
      auto src = mapa_ptr->net.get_src_node(a);
      auto tgt = mapa_ptr->net.get_tgt_node(a);
      auto prod_src = src->get_info();
      auto prod_tgt = tgt->get_info();
      const auto & info = a->get_info();
      return make_tuple(prod_src->rif,
			to_string(info.insumo_id), to_string(info.arco_id),
			tbl(info.insumo_id)->nombre, 
			to_string(info.producto_id), prod_tgt->rif);
    });

  auto lens = lines.foldl(make_tuple(col0.size(), col1.size(), 
				     col2.size(), col3.size(),
				     col4.size(), col5.size()),
			  [] (auto acu, auto l)
    {
      return make_tuple(max(get<0>(acu), get<0>(l).size()),
			max(get<1>(acu), get<1>(l).size()),
			max(get<2>(acu), get<2>(l).size()),
			max(get<3>(acu), get<3>(l).size()),
			max(get<4>(acu), get<4>(l).size()),
			max(get<4>(acu), get<5>(l).size()));
    });

  cout << string(get<0>(lens) - col0.size(), ' ') << col0 << " ["
       << string(get<1>(lens) - col1.size(), ' ') << col1 
       << " " << string(get<2>(lens) - col2.size(), ' ') << col2 
       << " " << string(get<3>(lens) - col3.size(), ' ') << col3
       << " " << string(get<4>(lens) - col4.size(), ' ') << col4 << "]" 
       << " " << string(get<5>(lens) - col5.size(), ' ') << col5 << endl;
  lines.for_each([&lens] (auto l)
    {
      const string blanks0(get<0>(lens) - get<0>(l).size(), ' ');
      const string blanks1(get<1>(lens) - get<1>(l).size(), ' ');
      const string blanks2(get<2>(lens) - get<2>(l).size(), ' ');
      const string blanks3(get<3>(lens) - get<3>(l).size(), ' ');
      const string blanks4(get<4>(lens) - get<4>(l).size(), ' ');
      const string blanks5(get<5>(lens) - get<5>(l).size(), ' ');
      cout << blanks0 << get<0>(l)
	   << " [" << blanks1 << get<1>(l)
	   << " " << blanks2 << get<2>(l)
	   << " " << blanks3 << get<3>(l) 
	   << " " << blanks4 << get<4>(l) 
	   << "] " << blanks5 << get<5>(l) << endl;
    });
}

void Arc::report()
{
  arcs_report(mapa_ptr, arc_list);
}

ExecStatus Save::execute()
{
  auto p = ::semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;

  mapa_ptr = p.second;

  auto n = semant_net(net_name);
  if (not n.first.first)
    return n.first;

  stringstream s;
  auto varnet = n.second;
  if (varnet->mapa_ptr != mapa_ptr)
    {
      s << "Var net " << net_name << " is not associated to map " << mapa_name;
      return make_pair(false, s.str());
    }

  auto r = semant_string(str_exp);
  if (not r.first.first)
    return r.first;

  const string & file_name = r.second;

  ofstream out(file_name);
  if (out.fail())
    {
      s << "cannot create file " << file_name;
      return make_pair(false, s.str());
    }

  cout << "Saving net " << net_name << " to " << file_name << "..." << endl;
  mapa_ptr->save(varnet->net, out);
  cout << "done!" << endl;

  free();
  return make_pair(true, "");
}

ExecStatus ComputePath::execute()
{
  auto r = ::semant_mapa(mapa_name);
  if (not r.first.first)
    return r.first;
  
  mapa_ptr = r.second;

  auto n = semant_node_exp(src_exp, mapa_ptr, src);
  if (not n.first)
    return n;

  n = semant_node_exp(tgt_exp, mapa_ptr, tgt);
  if (not n.first)
    return n;

  cout << "Computing a min path from " << src->get_info()->rif << " to "
       << tgt->get_info()->rif << endl;
  auto p = Directed_Find_Path<Net>(mapa_ptr->net).bfs(src, tgt);
  if (p.is_empty())
    {
      cout << "Not found, what does not necessarily mean that it does not exist"
	   << " the inverse" << endl;
      return make_pair(true, "");
    }

  cout << endl;
  arcs_report(mapa_ptr, p.arcs());
  cout << endl;

  free();
  return make_pair(true, "");
}

string VarRanks::info() const
{
  size_t diameter = 0;
  size_t ratio = 0;
  stringstream s;
  ranks.for_each([&s, &ratio, &diameter] (auto r)
    {
      size_t ratio_i = 0;
      s << "Rank "  << diameter++ << endl;
      r.for_each([&s, &ratio_i] (auto ptr)
        {
	  s << "    " << ptr->get_info()->to_string() << endl;
	  ++ratio_i;
	});
      s << "    " << ratio_i << " nodes" << endl;
      ratio = max(ratio, ratio_i);
    });
  s << diameter << " ranks" << endl;
  return s.str();
}

ExecStatus RanksExp::execute()
{
  auto rmapa = semant_mapa(mapa_name);
  if (not rmapa.first.first)
    return rmapa.first;

  auto mapa_ptr = rmapa.second;

  auto rnet = semant_net(net_name);
  if (not rnet.first.first)
    return rnet.first;
  
  auto varnet = rnet.second;
  if (varnet->mapa_ptr != mapa_ptr)
    {
      stringstream s;
      s << "var net " << net_name << " is not related to map" << mapa_name;
      return make_pair(false, s.str());
    }

  ranks = Q_Topological_Sort<Net>().ranks(varnet->net);

  return make_pair(true, "");
}

void Shareholder::report()
{
  using Line = tuple<string, string, string>;
  auto l = lista.map<Line>([this] (auto p)
       {
	 auto socio = mapa_ptr->tabla_socios(get<0>(p));
	 return make_tuple(get<0>(p), socio->nombre, to_string(get<2>(p)));
       });

  using Lens = tuple<size_t, size_t, size_t>;
  auto lens = l.foldl<Lens>(make_tuple(0, 0, 0), [] (auto acu, auto p)
    {
      return make_tuple(max(get<0>(acu), get<0>(p).size()),
			max(get<1>(acu), get<1>(p).size()),
			max(get<2>(acu), get<2>(p).size()));
    });
  
  l.for_each([&lens] (auto p)
    {
      const string blanks0(get<0>(lens) - get<0>(p).size(), ' ');
      const string blanks1(get<1>(lens) - get<1>(p).size(), ' ');
      const string blanks2(get<2>(lens) - get<2>(p).size(), ' ');
      cout << blanks0 << get<0>(p) << " " << blanks1 << get<1>(p) 
	   << " " << blanks2 << get<2>(p) << endl;
    });
}

ExecStatus ShareholderRif::execute()
{
  {
    auto p = semant_mapa(mapa_name);
    if (not p.first.first)
      return p.first;
    mapa_ptr = p.second;
  }

  Productor * producer_ptr = nullptr;
  auto p = semant_producer(mapa_ptr, producer_exp, producer_ptr);
  if (not p.first)
    return p;

  lista = producer_ptr->socios.map<Desc>([this] (auto p)
      {
	auto socio = mapa_ptr->tabla_socios(p.first);
	return make_tuple(p.first, socio->nombre, p.second);
      });

  cout << producer_ptr->rif << " " << producer_ptr->nombre << endl
       << "Shareholders:" << endl;
  report();

  return make_pair(true, "");
}

ExecStatus ShareholderRegex::execute()
{
  {
    auto p = semant_mapa(mapa_name);
    if (not p.first.first)
      return p.first;
    mapa_ptr = p.second;
  }

  auto p = semant_string(regex_exp);
  if (not p.first.first)
    return p.first;

  const string & str = p.second;
  try
    {
      auto producers = mapa_ptr->producers_by_name(str);
      producers.for_each([this] (auto producer_ptr)
        {
	  cout << "Shareholder " << producer_ptr->rif << " " 
	       << producer_ptr->nombre << " :" << endl;
	  producer_ptr->socios.for_each([this] (auto p)
            {
	      auto socio = mapa_ptr->tabla_socios(p.first);
	      lista.append(make_tuple(p.first, socio->nombre, p.second));
	    });
	  report();
	  cout << endl;
	  lista.empty();
	});
      return make_pair(true, "");
    }
  catch (regex_error & e)
    {
      return make_pair(false, e.what());
    }
  
  return make_pair(true, "");
}

void Holder::report()
{
  using Line = tuple<string, string, string>;
  auto l = lista.map<Line>([this] (auto p)
       {
	 auto prod_ptr = mapa_ptr->tabla_productores(get<0>(p));
	 return make_tuple(get<0>(p), prod_ptr->nombre, to_string(get<2>(p)));
       });

  using Lens = tuple<size_t, size_t, size_t>;
  auto lens = l.foldl<Lens>(make_tuple(0, 0, 0), [] (auto acu, auto p)
    {
      return make_tuple(max(get<0>(acu), get<0>(p).size()),
			max(get<1>(acu), get<1>(p).size()),
			max(get<2>(acu), get<2>(p).size()));
    });
  
  l.for_each([&lens] (auto p)
    {
      const string blanks0(get<0>(lens) - get<0>(p).size(), ' ');
      const string blanks1(get<1>(lens) - get<1>(p).size(), ' ');
      const string blanks2(get<2>(lens) - get<2>(p).size(), ' ');
      cout << blanks0 << get<0>(p) << " " << blanks1 << get<1>(p) 
	   << " " << blanks2 << get<2>(p) << endl;
    });
}

ExecStatus HoldigRif::execute()
{
  {
    auto p = semant_mapa(mapa_name);
    if (not p.first.first)
      return p.first;
    mapa_ptr = p.second;
  }

  auto p = semant_string(rif_exp);
  if (not p.first.first)
    return p.first;

  const string & rif = p.second;

  stringstream s;
  auto ptr = mapa_ptr->tabla_socios(rif);
  if (ptr == nullptr)
    {
      s << "Rif " << rif << " not found";
      return make_pair(false, s.str());
    }
  cout << "Holdings for " << rif << " " << ptr->nombre << ":" << endl;
  lista = ptr->empresas.map<Desc>([this] (auto p)
    {
      auto producer_ptr = mapa_ptr->tabla_productores(p.first);
      assert(producer_ptr);
      return make_tuple(p.first, producer_ptr->nombre, p.second);
    });
  report();

  return make_pair(true, "");
}

ExecStatus HoldingRegex::execute()
{
  {
    auto p = semant_mapa(mapa_name);
    if (not p.first.first)
      return p.first;
    mapa_ptr = p.second;
  }

  auto p = semant_string(str_exp);
  if (not p.first.first)
    return p.first;

  const string & str = p.second;

  try
    {
      regex reg(str);
      mapa_ptr->tabla_socios.for_each([&reg, this] (auto socio)
        {
	  if (not regex_search(socio.nombre, reg))
	    return;
	  cout << "Holdings for " << socio.rif << " " << socio.nombre
	       << " :" << endl;
	  socio.empresas.for_each([this] (auto p)
            {
	      lista.append(make_tuple(p.first, "", p.second));
	    });
	  report();
	  cout << endl;
	  lista.empty();
	});
    }
  catch (regex_error & e)
    {
      stringstream s;
      s << "Regular expression " << str << " " << e.what();
      return make_pair(false, s.str());
    }

  cout << "Holdings reports for regex " << str << " :" << endl;
  report();

  return make_pair(true, "");
}

ExecStatus Hegemony::execute()
{
  auto p = semant_mapa(mapa_name);
  if (not p.first.first)
    return p.first;
  mapa_ptr = p.second;

  auto ri = semant_int(int_exp);
  if (not ri.first.first)
    return ri.first;
  n = ri.second;

  lista = sort(mapa_ptr->tabla_socios.filter([this] (auto s)
    {
      return s.empresas.size() >= n;
    }), [] (const auto & s1, const auto & s2) 
	       { return s1.empresas.size() > s2.empresas.size(); });

  using Producer = tuple<string, string, float>;
  using Line = tuple<string, string, string, DynList<Producer>>;
  auto lines = lista.map<Line>([this] (const auto & s)
  {
    auto l = s.empresas.template map<Producer>([this] (auto p)
      { 
       	return make_tuple(p.first, 
			  mapa_ptr->tabla_productores(p.first)->nombre,
			  p.second);
      });
    return make_tuple(s.rif, s.nombre, to_string(s.empresas.size()), move(l));
  });

  using Lens = tuple<size_t, size_t, size_t>;
  auto lens = lines.foldl<Lens>(make_tuple(0, 0, 0), [] (auto acu, auto t)
    {
      return make_tuple(max(get<0>(acu), get<0>(t).size()),
			max(get<1>(acu), get<1>(t).size()),
			max(get<2>(acu), get<2>(t).size()));
    });

  lines.for_each([&lens] (auto l)
    {
      const string blanks0(get<0>(lens) - get<0>(l).size(), ' ');
      const string blanks1(get<1>(lens) - get<1>(l).size(), ' ');
      const string blanks2(get<2>(lens) - get<2>(l).size(), ' ');
      cout << " " << blanks0 << get<0>(l) << " " << blanks1 << get<1>(l)
	   << " " << blanks2 << get<2>(l) << " : " << endl;
      using Line = tuple<string, string, string, string>;
      size_t i = 0;
      auto lines = get<3>(l).template map<Line>([&i] (auto t)
        {
	  return make_tuple(to_string(++i), get<0>(t), 
			    get<1>(t), to_string(get<2>(t)));
	});
      using Lens = tuple<size_t, size_t, size_t, size_t>;
      auto lens = lines.template foldl<Lens>(make_tuple(0, 0, 0, 0), 
					     [] (auto acu, auto t)
       {
	 return make_tuple(max(get<0>(acu), get<0>(t).size()),
			   max(get<1>(acu), get<1>(t).size()),
			   max(get<2>(acu), get<2>(t).size()),
			   max(get<3>(acu), get<3>(t).size()));
       });
      lines.for_each([&lens] (auto t)
        {
	  const string blanks0(get<0>(lens) - get<0>(t).size(), ' ');
	  const string blanks1(get<1>(lens) - get<1>(t).size(), ' ');
	  const string blanks2(get<2>(lens) - get<2>(t).size(), ' ');
	  const string blanks3(get<3>(lens) - get<3>(t).size(), ' ');
	  cout << "   " << blanks0 << get<0>(t) << " " << blanks1 << get<1>(t)
	       << " " << blanks2 << get<2>(t) << " " << blanks3 << get<3>(t)
	       << " %" << endl;
	});
      cout << endl;
    });

  return make_pair(true, "");
}

ExecStatus RmArcNodes::execute()
{
  auto p = semant_mapa_or_net(vname, mapa_ptr, net_ptr);
  if (not p.first)
    return p;

  auto r = semant_node_exp(src_exp, mapa_ptr, src);
  if (not r.first)
    return r;

  r = semant_node_exp(tgt_exp, mapa_ptr, tgt);
  if (not r.first)
    return r;

  stringstream s;
  auto arc = net_ptr->search_directed_arc(src, tgt);
  if (arc == nullptr)
    {
      s << "Arc " << src->get_info()->rif << " --> " << tgt->get_info()->rif
	<< " not found";
      return make_pair(false, s.str());
    }
  
  cout << "Removing arc " << src->get_info()->rif << " [" << arc->get_info() 
       << "] " << tgt->get_info()->rif << endl;
  net_ptr->remove_arc(arc);

  free();
  return make_pair(true, "");
}

ExecStatus RmArcId::execute()
{
  auto p = semant_mapa_or_net(vname, mapa_ptr, net_ptr);
  if (not p.first)
    return p;  

  auto pi = semant_int(id_exp);
  if (not pi.first.first)
    return pi.first;

  arc_id = pi.second;

  cout << "Searching arc with id = " << arc_id << endl;
  auto arc = net_ptr->search_arc([this] (auto a) 
    {
      return a->get_info().arco_id == arc_id;
    });
  
  stringstream s;
  if (arc == nullptr)
    {
      s << "arc id " << arc_id << " not found";
      return make_pair(false, s.str());
    }

  cout << "Removing arc " << net_ptr->get_src_node(arc)->get_info()->rif 
       << " [" << arc->get_info() << "] " 
       << net_ptr->get_tgt_node(arc)->get_info()->rif << endl;
  net_ptr->remove_arc(arc);
  
  return make_pair(true, "");
}

ExecStatus RmNode::execute()
{
  auto p = semant_mapa_or_net(vname, mapa_ptr, net_ptr);
  if (not p.first)
    return p;

  auto r = semant_node_exp(node_exp, mapa_ptr, node_ptr);
  if (not r.first)
    return r;

  cout << "Removing node " << node_ptr->get_info()->rif << endl;
  net_ptr->remove_node(node_ptr);

  return make_pair(true, "");
}
