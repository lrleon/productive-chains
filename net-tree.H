
# ifndef NET_TREE_H
# define NET_TREE_H

# include <tpl_tree_node.H>
# include <net.H>

using namespace std;

using ExecStatus = std::pair<bool, string>;

struct ASTNode : public Tree_Node_Vtl<Empty_Class>
{
  string error_msg;
  bool error = false;

  void set_error(string msg) 
  {
    error_msg = std::move(msg);
    error = true;
  }

  bool was_executed() const noexcept { return error; }
  bool on_error() const noexcept { return was_executed(); }

  virtual ExecStatus execute() = 0;
  virtual void free() = 0;
};

struct ASTList : public DynList<ASTNode*>
{
  ~ASTList() 
  {
    while (not this->is_empty())
      delete this->remove_first();
  }
};

struct Exp : public ASTNode
{
  enum Type { MAP, LIST, STRCONST, INTCONST, VAR, ASSIGN, INFO, LS, RM, 
	      SEARCHPRODUCER, APPEND, LISTREAD, LISTWRITE, SEARCHPRODUCERREGEX,
	      SEARCHPRODUCTID, SEARCHPRODUCTREGEX, HELP, ERROR } type;

  string type_string() const
  {
    switch (type)
      {
      case MAP: return "MAP";
      case LIST: return "LIST";
      case STRCONST: return "STRCONST";
      case INTCONST: return "INTCONST";
      case VAR: return "VAR";
      case ASSIGN: return "ASSIGN";
      case INFO: return "INFO";
      case LS: return "LS";
      case RM: return "RM";
      case SEARCHPRODUCER: return "SEARCHPRODUCER";
      case SEARCHPRODUCERREGEX: return "SEARCHPRODUCERREGEX";
      case SEARCHPRODUCTID: return "SEARCHPRODUCTID";
      case SEARCHPRODUCTREGEX: return "SEARCHPRODUCTREGEX";
      case HELP: return "HELP";
      case APPEND: return "APPEND";
      case LISTREAD: return "LISTREAD";
      case LISTWRITE: return "LISTWRITE";
      default:
	ERROR("Tipo de expresion invalido");
      }
    return string("INVALID");
  }

  Exp(Type t) : type(t) { }

  ExecStatus execute() 
  { 
    ERROR("Epx: execute method called"); return ExecStatus(); 
  }
  void free() { ERROR("Exp: free() method called"); }
};

struct ErrorExp : Exp
{
  string msg;

  ErrorExp(const string & m) : Exp(ERROR), msg(m) {}
  ExecStatus execute() { return make_pair(false, msg); }
  void free() {}
};

struct StringExp : public Exp
{
  const string value;
  StringExp(char * str) : Exp(STRCONST), value(str) {}
  ExecStatus execute() { return make_pair(true, ""); }
};

struct ListExp : public Exp
{
  ListExp() : Exp(LIST) {}
  ExecStatus execute() { return make_pair(true, ""); }
  void free() {}
};

struct Append : public Exp
{
  string list_name;
  DynList<Exp*> rexp_list;

  Append(const string & name, DynList<Exp*> * l_ptr) 
    : Exp(APPEND), list_name(name), rexp_list(move(*l_ptr)) { }
  ExecStatus execute();
  void free() { rexp_list.for_each([this] (auto e) { delete e; }); }
};

struct Var
{
  enum VarType { Map, List, String, Int, Producer, Product } var_type;

  Var(VarType type) : var_type(type) {}

  virtual ~Var() {} // virtual para poder liberar recursos de las derivadas

  virtual string info() const = 0;
  virtual Var * clone() const = 0;
  virtual void copy(Var * src) = 0;

  string type_string() const 
  {
    switch (var_type)
      {
      case Map: return string("Map");
      case List: return string("List");
      case String: return string("String");
      case Int: return string("Int");
      case Producer: return string("Producer");
      case Product: return string("Product");
      default: ERROR("Invalid type %ld", var_type);
      }
    return string("Var::type_string() var_type invalid");
  }
};

struct Varname : public Exp
{
  const string name;
  
private:

  Var * value_ptr = nullptr; // instancia la variable a la que esta asociada

public:

  bool has_value() const noexcept { return value_ptr != nullptr; }

  void free_value() noexcept
  {
    if (value_ptr == nullptr)
      return;

    cout << "**** Freing var " << name << endl;
    delete value_ptr;
    value_ptr = nullptr;
  }

  Varname(string nom) : Exp(VAR), name(move(nom)) {}

  ~Varname() { free_value(); }

  ExecStatus execute() 
  {
    if (value_ptr == nullptr)
      {
	stringstream s;
	s << "Var name " << name << " has not assigned value";
	return make_pair(false, s.str());
      }
    return make_pair(true, "");
  }
  void free() { free_value(); }

  Var * get_value_ptr() const noexcept { return value_ptr; }
  void set_value_ptr(Var * ptr) noexcept { value_ptr = ptr; }
};

struct Info : public Exp
{
  const string name;

  Info(char * nom) : Exp(INFO), name(nom) {}

  ExecStatus execute();
  void free() {}
};

struct Ls : public Exp
{
  Ls() : Exp(LS) {}

  ExecStatus execute();
  void free() {}
};

struct Help : public Exp
{
  Help(Type t = HELP) : Exp(t) {}
  ExecStatus execute();
  void free() {}
};

struct Rm : public Exp
{
  DynList<string> names;

  Rm() : Exp(RM) {}

  ExecStatus execute();
  void free() {}
};

struct VarMap : public Var
{
  MetaMapa value;
  VarMap() : Var(Map) {}

  VarMap * clone() const { return new VarMap(); }
  void copy(Var * src) { value = static_cast<VarMap*>(src)->value; }

  string info() const 
  {
    stringstream s;
    s << "Map var:" << endl
      << "    " << value.tabla_productores.size() << " producers" << endl
      << "    " << value.tabla_productos.size() << " products" << endl
      << "    " << value.net.vsize() << " nodes" << endl
      << "    " << value.net.esize() << " arcs" << endl;
    return s.str();
  }
};

struct VarList : public Var
{
  DynList<Var*> list;

  VarList() : Var(List) {}

  VarList * clone() const { return new VarList; }

  void copy(Var * src)
  {
    list.empty();
    static_cast<VarList*>(src)->list.for_each([this] (auto v)
      {
	auto vv = v->clone();
	vv->copy(v);
	list.append(vv);
      });
  }

  ~VarList() { list.for_each([] (auto v) { delete v; } ); }

  string info() const 
  {
    stringstream s;
    s << "List var: " << endl;
    size_t i = 0;
    list.for_each([&i, &s] (auto v) { s << i++ << ": " << v->info() << endl; });
    if (i == 0)
      s << "EMPTY";
    s << endl;
    return s.str();
  }
};

struct VarString : public Var
{
  string value;
  VarString() : Var(String) {}

  VarString * clone() const { return new VarString; }
  void copy(Var * src) { value = static_cast<VarString*>(src)->value; }

  string info() const { return "String var: \"" + value + "\"\n"; }
};

struct VarInt : public Var
{
  long value;
  VarInt() : Var(Int) {}

  VarInt * clone() const { return new VarInt; }
  void copy(Var * src) { value = static_cast<VarInt*>(src)->value; }
  string info() const { return "Int var: " + to_string(value) + '\n'; }
};

struct IntExp : public Exp
{
  const long value;

  IntExp(const string & val) : Exp(INTCONST), value(stol(val)) {}

  ExecStatus execute() { return make_pair(true, ""); }
  void free() {}
};

struct VarProducer : public Var
{
  Productor productor;

  VarProducer() : Var(Producer) {}
  VarProducer(const Productor & p) : VarProducer() { productor = p; }
  Var * clone() const { return new VarProducer; }
  void copy(Var * src) 
  {
    productor = static_cast<VarProducer*>(src)->productor; 
  }
  string info() const { return "Producer var: " + productor.to_string(); }
};

struct VarProduct : public Var
{
  MetaProducto product;
  
  VarProduct() : Var(Product) {}
  VarProduct(const MetaProducto & p) : VarProduct() { product = p; }
  Var * clone() const { return new VarProduct; }
  void copy(Var * src) { product = static_cast<VarProduct*>(src)->product; }
  string info() const { return "Product var: " + product.to_string(); }
};

struct ListAccess
{ 
  string list_name;
  Exp *  index_exp = nullptr;
  Var ** val = nullptr; // ** para poder mantener referencia en escritura

  ListAccess(const string & name, Exp * idx_exp)
    : list_name(name), index_exp(idx_exp) {}
  ExecStatus access();
};

struct ListRead : public Exp, public ListAccess
{
  ListRead(const string & name, Exp * idx_exp) 
    : Exp(LISTREAD), ListAccess(name, idx_exp) {}
  ExecStatus execute();
  void free() { delete index_exp; }
};

struct ListWrite : public Exp, public ListAccess
{
  Exp * rexp = nullptr;

  ListWrite(const string & name, Exp * idx_exp, Exp * exp) 
    : Exp(LISTWRITE), ListAccess(name, idx_exp), rexp(exp) {}
  ExecStatus execute();
  void free() 
  {
    delete index_exp; 
    delete rexp;
  }
};

struct Search : public Exp
{
  string mapa_name;
  Exp * exp = nullptr;
  const MetaMapa * mapa_ptr = nullptr;
  string str;
  Uid id;

  Search(Exp::Type type, const string & name, Exp * e) 
    : Exp(type), mapa_name(name), exp(e) {}
  ExecStatus semant_mapa();
  ExecStatus semant_string();
  ExecStatus semant_int();
  void free() { delete exp; }
};

struct SearchProducerRif : public Search
{
  Productor * producer_ptr = nullptr;

  SearchProducerRif(const string & name, Exp * exp) 
    : Search(SEARCHPRODUCER, name, exp) {}

  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProducerRifCmd : public SearchProducerRif
{
  using SearchProducerRif::SearchProducerRif;  
  ExecStatus execute();
};

struct SearchProducerRegex : public Search
{
  DynList<Productor*> producers;

  SearchProducerRegex(const string & name, Exp * exp)
    : Search(SEARCHPRODUCERREGEX, name, exp) {}

  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProducerRegexCmd : public SearchProducerRegex
{
  using SearchProducerRegex::SearchProducerRegex;
  ExecStatus execute();
};

struct SearchProductId : public Search
{
  MetaProducto producto;
  SearchProductId(const string & name, Exp * exp) 
    : Search(SEARCHPRODUCTID, name, exp) {}
  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProductIdCmd : public SearchProductId
{
  using SearchProductId::SearchProductId;
  ExecStatus execute();
};

struct SearchProducts : public Search
{
  DynList<MetaProducto*> productos;
  
  SearchProducts(Exp::Type type, const string & map_name, Exp * str_exp)
    : Search(type, map_name, str_exp) {}
};

struct SearchProductsRegex : public SearchProducts
{
  SearchProductsRegex(const string & map_name, Exp * str_exp)
    : SearchProducts(SEARCHPRODUCTREGEX, map_name, str_exp) {}
  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProductsRegexCmd : public SearchProductsRegex
{
  using SearchProductsRegex::SearchProductsRegex;
  ExecStatus execute();
};

struct Load : public Exp
{
  Exp * name_exp = nullptr;
  MetaMapa mapa;

  Load(Exp * exp) noexcept : Exp(MAP), name_exp(exp) {}

  ExecStatus execute();
  void free() {delete name_exp; }
};

struct Assign : public Exp
{
  string left_name;
  Exp * right_side = nullptr;

  Assign(string lname, Exp * rhs) 
    : Exp(ASSIGN), left_name(move(lname)), right_side(rhs) {}

  ~Assign()
  {
    // if (right_side->type != VAR)
    //   delete right_side; 
  }

  ExecStatus execute();
  void free() { delete right_side; }
};

struct Save : public ASTNode
{
  char * file_name;

  Save(const MetaMapa & mapa, char * name) : file_name(name) {}

  ExecStatus execute();
  void error() {}
};




# endif // NET_TREE_H
