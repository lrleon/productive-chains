
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

  virtual ostream & trace(ostream & out) const
  {
    return out << "ASTNode"<< endl;
  }
};

struct ASTList : public DynList<ASTNode*>
{
  ~ASTList() 
  {
    while (not this->is_empty())
      delete this->remove_first();
  }

  ostream & trace(ostream & out) const
  {
    for_each([&out] (auto p) 
	     {
	       out << "Comando ";
	       p->trace(out); 
	       out << endl;
	     });
    return out;
  }
};

struct Exp : public ASTNode
{
  enum Type { GRAPH, LIST, STRCONST, INTCONST, VAR, ASSIGN, INFO, LS, RM, 
	      SEARCH, ERROR } type;

  string type_string() const
  {
    switch (type)
      {
      case GRAPH: return string("GRAPH");
      case LIST: return string("LIST");
      case STRCONST: return string("STRCONST");
      case INTCONST: return string("INTCONST");
      case VAR: return string("VAR");
      case ASSIGN: return string("ASSIGN");
      case INFO: return string("INFO");
      case LS: return string("LS");
      case RM: return string("RM");
      case SEARCH: return string("SEARCH");
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

  ostream & trace(ostream & out) const { return out << "Exp "; }
};

struct ErrorExp : Exp
{
  string msg;

  ErrorExp(const string & m) : Exp(ERROR), msg(m) {}

  ExecStatus execute()
  {
    return make_pair(false, msg);
  }

  ostream & trace(ostream & out) const { return out << "Exp "; }
};

struct StringExp : public Exp
{
  const string value;

  StringExp(char * str) : Exp(STRCONST), value(str) {}

  ExecStatus execute() 
  {
    return make_pair(true, "");
  }

  ostream & trace(ostream & out) const { return out << "StringExp "; }
};

struct Var
{
  enum VarType { Map, List, String, Int, Report } var_type;

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
    cout << "**** Freing var " << name << endl;
    if (value_ptr == nullptr)
      return;

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

  ostream & trace(ostream & out) const { return out << "Varname "; }

  Var * get_value_ptr() const noexcept { return value_ptr; }

  void set_value_ptr(Var * ptr) noexcept { value_ptr = ptr; }
};

struct Info : public Exp
{
  const string name;

  Info(char * nom) : Exp(INFO), name(nom) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Info "; }
};

struct Ls : public Exp
{
  Ls() : Exp(LS) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Ls "; }
};

struct Rm : public Exp
{
  DynList<string> names;

  Rm() : Exp(RM) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Rm "; }
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
  void copy(Var * src) { list = static_cast<VarList*>(src)->list; }

  ~VarList()
  {
    list.for_each([] (auto v) { delete v; } );
  }

  string info() const 
  {
    stringstream s;
    s << "List var: ";
    size_t i = 0;
    list.for_each([&i, &s] (auto v) { s << i++ << ": " << v->info() << endl; });
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

  ostream & trace(ostream & out) const { return out << "IntExp "; }
};

struct VarReport : public Var
{
  DynList<string> entries;

  VarReport() : Var(Report) {}
  Var * clone() const { return new VarReport; }
  void copy(Var * src) { entries = static_cast<VarReport*>(src)->entries; }
  string info() const 
  {
    stringstream s;
    s << "Report var with " << entries.size() << " entries";
    return s.str();
  }
};

struct Search : public Exp
{
  string mapa_name;
  MetaMapa * mapa_ptr = nullptr;

  SearchProducer(Exp::Type type, const string & name) 
    : Exp(type), mapa_name(name) {}

  ExecStatus set_mapa();
};

struct SearchProducerId : public SearchProducer
{
  string mapa_name;
  Exp * int_exp = nullptr;

  SearchProducerId(const string & name, Exp * exp) 
    : Search(SEARCH, mapa_name), int_exp(exp) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "SearchProducerId "; }
};

// struct SearchProducerRegex : public Exp
// {

// };

// struct SearchProducerRif : public Exp
// {

// };

// struct SearchProducerExt : public Exp
// {

// };

struct Load : public Exp
{
  Exp * name_exp = nullptr;
  MetaMapa mapa;

  Load(Exp * exp) noexcept : Exp(GRAPH), name_exp(exp) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Load "; }
};

struct Assign : public Exp
{
  string left_name;
  Exp * right_side;

  Assign(string lname, Exp * rhs) 
    : Exp(ASSIGN), left_name(move(lname)), right_side(rhs) {}

  ~Assign()
  {
    if (right_side->type != VAR)
      delete right_side; 
  }

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Assign "; }
};

struct Save : public ASTNode
{
  char * file_name;

  Save(const MetaMapa & mapa, char * name) : file_name(name) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Save "; }
};




# endif // NET_TREE_H
