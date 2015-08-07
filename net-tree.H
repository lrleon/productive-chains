
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
  enum Type { GRAPH, LIST, STRCONST, ID, VAR, ASSIGN, INFO } type;
  void * result = nullptr;

  string type_string() const
  {
    switch (type)
      {
      case GRAPH: return string("GRAPH");
      case LIST: return string("LIST");
      case STRCONST: return string("STRCONST");
      case ID: return string("ID");
      case VAR: return string("VAR");
      case ASSIGN: return string("ASSIGN");
      default:
	ERROR("Tipo de expresion invalido");
      }
  }

  Exp(Type t) : type(t) { }

  ExecStatus execute() 
  { 
    WARNING("StringEpx: execute method called"); return ExecStatus(); 
  }

  ostream & trace(ostream & out) const { return out << "Exp "; }

  // TODO: exportar un void* al resultado cuya responsabilidad la tiene
  // execute() de  modificarlo
};

struct StringExp : public Exp
{
  const string value;

  StringExp(char * str) : Exp(STRCONST), value(str) {}

  ExecStatus execute() 
  {
    WARNING("StringExp: execute method called"); return ExecStatus();     
  }

  ostream & trace(ostream & out) const { return out << "StringExp "; }
};

struct Var
{
  enum VarType { Map, List, String, Int } var_type;

  Var(VarType type) : var_type(type) {}

  virtual ~Var() {} // virtual para poder liberar recursos de las derivadas

  virtual string info() const = 0;

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
  }
};

struct Varname : public Exp
{
  const string name;
  
private:

  Var * value_ptr = nullptr; // instancia la variable a la que esta asociada

public:

  bool has_value() const noexcept { return value_ptr != nullptr; }

  void free_var() noexcept
  {
    if (value_ptr == nullptr)
      return;

    delete value_ptr;
    value_ptr = nullptr;
  }

  Varname(char * nom) : Exp(VAR), name(nom) {}

  ~Varname() { if (value_ptr != nullptr) delete value_ptr; }

  ExecStatus execute() 
  {
    WARNING("Varname: execute method called"); return ExecStatus();  
  }

  ostream & trace(ostream & out) const { return out << "Varname "; }

  Var * get_value_ptr() const noexcept { return value_ptr; }

  void set_value_ptr(Var * ptr) noexcept { value_ptr = ptr; }
};

struct Info : Exp
{
  const string name;

  Info(char * nom) : Exp(INFO), name(nom) {}

  ExecStatus execute();

  ostream & trace(ostream & out) const { return out << "Info "; }
};

struct VarMap : public Var
{
  MetaMapa value;
  VarMap() : Var(Map) {}

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

  string info() const { return "String var: \"" + value + "\"\n"; }
};

struct VarInt : public Var
{
  long value;
  VarInt() : Var(Int) {}

  string info() const { return "Int var: " + to_string(value) + '\n'; }
};

struct IdExp : public Exp
{
  const Uid id;

  IdExp(const string & val) : Exp(ID), id(stol(val)) {}

  ExecStatus execute() 
  {
    WARNING("IdExp: execute method called"); return ExecStatus(); 
  }

  ostream & trace(ostream & out) const { return out << "IdExp "; }
};

struct NetExp : public Exp
{
  Net net;

  NetExp() : Exp(GRAPH) { }

  ostream & trace(ostream & out) const { return out << "NetExp "; }
};

struct Load : public Exp
{
  StringExp * file_name = nullptr;
  MetaMapa mapa;

  Load(StringExp * exp) noexcept : Exp(GRAPH), file_name(exp) {}

  ExecStatus execute();

  ~Load() { delete file_name; }

  ostream & trace(ostream & out) const { return out << "Load "; }
};

struct Assign : public Exp
{
  Varname * left_side;
  Exp * right_side;

  Assign(Varname * var, Exp * rhs) 
    : Exp(ASSIGN), left_side(var), right_side(rhs) {}

  ~Assign()
  {
    delete left_side;
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
