// TODO sustituir VarCover por VarNet

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
	      SEARCHPRODUCTID, SEARCHPRODUCTREGEX, SEARCHPRODUCTCOD,
	      SEARCHPRODUCTRIF, SEARCHNODE, NODE, COVER, REACHABLE,
	      UPSTREAM, INPUTS, ARCS, SAVE, PATH, RANKS, SHAREHOLDER, RMARC,
	      RMNODE, HOLDER, HEGEMONY,
	      TYPEINFO, HELP, DOT, BLOCKS, ERROR } type;

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
      case SEARCHPRODUCTCOD: return "SEARCHPRODUCTCOD";
      case SEARCHNODE: return "SEARCHNODE";
      case HELP: return "HELP";
      case TYPEINFO: return "TYPEINFO";
      case APPEND: return "APPEND";
      case LISTREAD: return "LISTREAD";
      case LISTWRITE: return "LISTWRITE";
      case NODE: return "NODE";
      case COVER: return "COVER";
      case REACHABLE: return "REACHABLE";
      case DOT: return "DOT";
      case BLOCKS: return "BLOCKS";
      case UPSTREAM: return "UPSTREAM";
      case INPUTS: return "INPUTS";
      case ARCS: return "ARCS";
      case SAVE: return "SAVE";
      case PATH: return "PATH";
      case RANKS: return "RANKS";
      case HOLDER: return "HOLDER";
      case SHAREHOLDER: return "SHAREHOLDER";
      case HEGEMONY: return "HEGEMONY";
      case RMARC: return "RMARC";
      case RMNODE: return "RMNODE";
      default: ERROR("Tipo de expresion invalido");
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

struct RmNode : public Exp
{
  const string vname;
  Exp * node_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net * net_ptr = nullptr;
  Net::Node * node_ptr = nullptr;
  RmNode(const string & name, Exp * exp) 
    : Exp(RMNODE), vname(name), node_exp(exp) {}
  ExecStatus execute();
  void free() { if (node_exp->type != VAR) delete node_exp; }
};

struct RmArcNodes : public Exp
{
  const string vname;
  Exp * src_exp = nullptr;
  Exp * tgt_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net * net_ptr = nullptr;
  Net::Node * src = nullptr;
  Net::Node * tgt = nullptr;
  RmArcNodes(const string & name, Exp * sexp, Exp * texp) 
    : Exp(RMARC), vname(name), src_exp(sexp), tgt_exp(texp) {}
  ExecStatus execute();
  void free()
  {
    if (src_exp->type != VAR) delete src_exp; 
    if (tgt_exp->type != VAR) delete tgt_exp; 
  }
};

struct RmArcId : public Exp
{
  const string vname;
  Exp * id_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net * net_ptr = nullptr;
  Uid arc_id;
  RmArcId(const string & name, Exp * exp) 
    : Exp(RMARC), vname(name), id_exp(exp) {}
  ExecStatus execute();
  void free() { if (id_exp->type != VAR) delete id_exp; }
};

struct Shareholder : public Exp
{                // rif-socio, nombre-socio, %
  using Desc = tuple<string, string, float>;
  const string mapa_name;
  MetaMapa * mapa_ptr = nullptr;
  DynList<Desc> lista;
  void report();
  Shareholder(const string & mname) : Exp(SHAREHOLDER), mapa_name(mname) {}
  void free() {}
};

struct ShareholderRif : public Shareholder
{
  Exp * producer_exp = nullptr;
  Productor * producer_ptr = nullptr;
  ShareholderRif(const string & mname, Exp * exp) 
    : Shareholder(mname), producer_exp(exp) {}
  ExecStatus execute();
  void free() { if (producer_exp->type != VAR) delete producer_exp; }
};

struct ShareholderRegex : public Shareholder
{
  Exp * regex_exp = nullptr;
  ShareholderRegex(const string & mname, Exp * exp) 
    : Shareholder(mname), regex_exp(exp) {}
  ExecStatus execute();
  void free() { if (regex_exp->type != VAR) delete regex_exp; }
};

struct Holder : public Exp
{              // rif-empresa, nombre-empresa, %
  using Desc = tuple<string, string, float>;
  const string mapa_name;
  MetaMapa * mapa_ptr = nullptr;
  DynList<Desc> lista;
  void report();
  Holder(const string & mname) : Exp(HOLDER), mapa_name(mname) {}
};

struct HoldigRif : public Holder
{
  Exp * rif_exp = nullptr;
  HoldigRif(const  string & mname, Exp * exp) : Holder(mname), rif_exp(exp) {}
  ExecStatus execute();
  void free() { if (rif_exp->type != VAR) delete rif_exp; }
};

struct HoldingRegex : public Holder
{
  Exp * str_exp = nullptr;
  HoldingRegex(const  string & mname, Exp * exp) 
    : Holder(mname), str_exp(exp) {}
  ExecStatus execute();
  void free() { if (str_exp->type != VAR) delete str_exp; }
};

struct ErrorExp : Exp
{
  string msg;
  ErrorExp(const string & m) : Exp(ERROR), msg(m) {}
  ExecStatus execute() { return make_pair(false, msg); }
  void free() {}
};

struct RanksExp : public Exp
{
  const string mapa_name;
  const string net_name;
  DynList<DynList<Net::Node*>> ranks;
  RanksExp(const string & mname, const string & nname)
    : Exp(RANKS), mapa_name(mname), net_name(nname) {}
  ExecStatus execute();
  void free() {}
};

struct ComputePath : public Exp
{
  const string mapa_name;
  Exp * src_exp = nullptr;
  Exp * tgt_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net::Node * src = nullptr;
  Net::Node * tgt = nullptr;

  ComputePath(const string & mname, Exp * sexp, Exp * texp)
    : Exp(PATH), mapa_name(mname), src_exp(sexp), tgt_exp(texp) {}
  ExecStatus execute();
  void free() 
  {
    if (src_exp->type != VAR)
      delete src_exp; 
    if (tgt_exp->type != VAR)
      delete tgt_exp; 
  }
};

struct Arc : public Exp
{
  const string mapa_name;
  MetaMapa * mapa_ptr = nullptr;
  DynList<Net::Arc*> arc_list;
  Arc(const string & name) : Exp(ARCS), mapa_name(name) {}
  ExecStatus semant_mapa();
  void report();
};

struct ArcsRegex : public Arc
{
  Exp * str_exp = nullptr;
  string str;
  ArcsRegex(const string mapa_name, Exp * exp) : Arc(mapa_name), str_exp(exp) {}
  ExecStatus execute();
  void free() { if (str_exp->type != VAR) delete str_exp; }
};

struct ArcsInt : public Arc
{
  Exp * int_exp = nullptr; // expresa un id de insumo, producto o arc
  long id = -1;
  ArcsInt(const string & mapa_name, Exp * exp) : Arc(mapa_name), int_exp(exp) {}
  ExecStatus semant();
  void free() { if (int_exp and int_exp->type != VAR) delete int_exp; }
};

struct ArcsInputId : public ArcsInt
{
  ArcsInputId(const string & mapa_name, Exp * exp) : ArcsInt(mapa_name, exp) {}
  ExecStatus execute();
  void free() { ArcsInt::free(); }
};

struct ArcsOutputId : public ArcsInt
{
  ArcsOutputId(const string & mapa_name, Exp * exp) : ArcsInt(mapa_name, exp) {}
  ExecStatus execute();
  void free() { ArcsInt::free(); }
};

struct ArcsProducer : public Arc
{
  Exp * producer_exp = nullptr;
  Exp * product_exp = nullptr;
  Productor * producer_ptr = nullptr;
  MetaProducto * product_ptr = nullptr;
  Net::Node * node_ptr = nullptr;

  ArcsProducer(const string & mapa_name, Exp * producer, Exp * product)
    : Arc(mapa_name), producer_exp(producer), product_exp(product) {}
  ExecStatus semant();
  void free() 
  {
    if (producer_exp and producer_exp->type != VAR) delete producer_exp; 
    if (product_exp and product_exp->type != VAR) delete product_exp;
  }
};

struct ArcsInputP : public ArcsProducer
{
  ArcsInputP(const string & mapa_name, Exp * producer, Exp * product)
    : ArcsProducer(mapa_name, producer, product) {}
  ExecStatus execute();
  void free() { ArcsProducer::free(); }
};


struct ArcsOutputP : public ArcsProducer
{
  ArcsOutputP(const string & mapa_name, Exp * producer, Exp * product)
    : ArcsProducer(mapa_name, producer, product) {}
  ExecStatus execute();
  void free() { ArcsProducer::free(); }
};

struct Arcs : public ArcsInt
{
  Arcs(const string & mapa_name, Exp * exp) : ArcsInt(mapa_name, exp) {}
  ExecStatus execute();
  void free() { ArcsInt::free(); }
};

struct Inputs : public Exp
{
  const string mapa_name;
  const string name = "";
  long producto_id = -1;
  MetaMapa * mapa_ptr = nullptr;
  MetaProducto * producto_ptr = nullptr;
  Net::Node * node_ptr = nullptr;
  Inputs(const string & name, const string & vname) 
    : Exp(INPUTS), mapa_name(name), name(vname) {}
  Inputs(const string & name, Uid id) 
    : Exp(INPUTS), mapa_name(name), producto_id(id) {}
  ExecStatus execute();
  void free() {}
  void report_product();
  void report_node();
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
  enum VarType { Map, List, String, Int, Producer, Product, Node, 
		 Cover, Ranks} var_type;

  Var(VarType type) : var_type(type) {}

  virtual ~Var() {} // virtual para poder liberar recursos de las derivadas

  virtual string info() const = 0;
  virtual string type_info() const = 0;
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
      case Node: return string("Node");
      case Cover: return string("Cover");
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
  Exp * index_exp = nullptr;

  Info(char * nom) : Exp(INFO), name(nom) {}

  Info(char * nom, Exp * exp) : Exp(INFO), name(nom), index_exp(exp) {}

  ExecStatus execute();
  void free() 
  {
    if (index_exp and index_exp->type != VAR)
      delete index_exp; 
  }
};

struct Ls : public Exp
{
  Ls() : Exp(LS) {}

  ExecStatus execute();
  void free() {}
};

struct TypeInfo : public Exp
{
  string name;
  Exp * index_exp = nullptr;
  TypeInfo(const string & n) : Exp(TYPEINFO), name(n) {}
  TypeInfo(const string & n, Exp * exp) 
    : Exp(TYPEINFO), name(n), index_exp(exp) {}
  ExecStatus execute();
  void free() { if (index_exp and index_exp->type != VAR) delete index_exp; }
};

struct Connected : public Exp
{
  const string mapa_name;
  Exp * src_exp = nullptr;
  Exp * tgt_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net::Node * src = nullptr;
  Net::Node * tgt = nullptr;
  Connected(const string & name, Exp * src, Exp * tgt) 
    : Exp(REACHABLE), mapa_name(name), src_exp(src), tgt_exp(tgt) {}
      // Valida que exp sea rif, producer o nodo y pone el el nodo en ptr
  ExecStatus semant_mapa();
  ExecStatus execute();
  void free() 
  {
    if (src_exp->type != VAR)
      delete src_exp; 
    if (tgt_exp->type != VAR)
      delete tgt_exp; 
  }
};

struct SubNet : public Exp
{
  const string mapa_name;
  Exp * node_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net::Node * src = nullptr;
  Net net;
  SubNet(Exp::Type type, const string & mapa, Exp * exp) 
    : Exp(type), mapa_name(mapa), node_exp(exp) {}
  ExecStatus semant();
};

struct Cover : public SubNet
{
  Cover(const string & mapa_name, Exp * node_exp) 
    : SubNet(COVER, mapa_name, node_exp) {}
  ExecStatus execute();
  void free() {}
};

struct UpstreamB : public SubNet
{
  Exp * product_exp = nullptr;
  Exp * threshold_exp = nullptr;
  MetaProducto * product_ptr = nullptr;
  size_t threshold = 0;
  UpstreamB(const string & mapa_name, Exp * node_exp, 
	    Exp * prod_exp, Exp * th_exp) 
    : SubNet(UPSTREAM, mapa_name, node_exp), 
      product_exp(prod_exp), threshold_exp(th_exp) {}
  ExecStatus execute();
  void free() 
  { 
    if (product_exp->type != VAR) delete product_exp;
    if (threshold_exp->type != VAR) delete threshold_exp; 
  }
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

struct VarRanks : public Var
{
  MetaMapa * mapa_ptr = nullptr;
  DynList<DynList<Net::Node*>> ranks;
  VarRanks() : Var(Ranks) {}
  VarRanks * clone() const { return new VarRanks; }
  void copy(Var * src) { *this = *static_cast<VarRanks*>(src); }
  string info() const;
  string type_info() const
  {
    size_t diameter = ranks.size();
    size_t ratio = ranks.foldl<size_t>(0, [] (auto a, auto r) 
      {
	return max(a, r.size());
      });
    stringstream s;
    s << "Rank var:" << endl
      << "    Diameter = " << diameter << endl
      << "    ratio    = " << ratio << endl;
    return s.str();
  }
};

struct VarMap : public Var
{
  MetaMapa value;
  DynList<Net*> net_list; // redes dependientes de este mapa
  VarMap() : Var(Map) {}
  VarMap * clone() const { return new VarMap; }
  void copy(Var * src) { value = static_cast<VarMap*>(src)->value; }
  string info() const 
  {
    stringstream s;
    s << "Map var:" << endl
      << "    " << value.tabla_productores.size() << " producers" << endl
      << "    " << value.tabla_productos.size() << " products" << endl
      << "    " << value.tabla_insumos.size() << " inputs" << endl
      << "    " << value.tabla_socios.size() << " stakeholders" << endl
      << "    " << value.net.vsize() << " nodes" << endl
      << "    " << value.net.esize() << " arcs" << endl;
    return s.str();
  }
  string type_info() const { return info(); }
};

struct VarNet : public Var
{
  MetaMapa * mapa_ptr = nullptr;
  Net        net;
  VarNet() : Var(Cover) {}
  VarNet * clone() const { return new VarNet; }
  void copy(Var * src) 
  {
    auto s = static_cast<VarNet*>(src);
    mapa_ptr = s->mapa_ptr;
    net      = s->net;
  }
  string info() const
  {
    stringstream s;
    s << "Cover var:" << endl;
    net.for_each_node([&s] (auto p) { s << "    " << *p->get_info() << endl; });
    return s.str();
  }
  string type_info() const 
  {
    stringstream s;
    s << "Cover var:" << endl
      << "    " << net.vsize() << " nodes" << endl
      << "    " << net.esize() << " arcs" << endl;
    return s.str();
  }
};

struct VarNode : public Var
{
  MetaMapa * mapa_ptr = nullptr;
  Net * net_ptr = nullptr;
  Net::Node * node_ptr = nullptr;

  VarNode() : Var(Node) {}
  VarNode * clone() const { return new VarNode; }
  void copy(Var * src) { *this = *static_cast<VarNode*>(src); }
  string info() const
  {
    return "Var node pointing to producer: " + node_ptr->get_info()->to_string();
  }
  string type_info() const { return "Var node on a map"; }
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
    else
      s << i << " elements";
    s << endl;
    return s.str();
  }

  string type_info() const 
  {
    return "List var with " + to_string(list.size()) + " items";
  }
};

struct VarString : public Var
{
  string value;
  VarString() : Var(String) {}
  VarString * clone() const { return new VarString; }
  void copy(Var * src) { value = static_cast<VarString*>(src)->value; }
  string info() const { return "String var: \"" + value + "\"\n"; }
  string type_info() const { return "String var"; }
};

struct VarInt : public Var
{
  long value;
  VarInt() : Var(Int) {}
  VarInt * clone() const { return new VarInt; }
  void copy(Var * src) { value = static_cast<VarInt*>(src)->value; }
  string info() const { return "Int var: " + to_string(value); }
  string type_info() const { return "Int var"; }
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
  MetaMapa * mapa_ptr = nullptr;
  Productor productor;

  VarProducer() : Var(Producer) {}
  VarProducer(const Productor & p) : VarProducer() { productor = p; }
  Var * clone() const { return new VarProducer; }
  void copy(Var * src) 
  {
    productor = static_cast<VarProducer*>(src)->productor; 
  }
  string info() const { return "Producer var: " + productor.to_string(); }
  string type_info() const { return "Producer var"; }
};

struct VarProduct : public Var
{
  MetaProducto product;
  
  VarProduct() : Var(Product) {}
  VarProduct(const MetaProducto & p) : VarProduct() { product = p; }
  Var * clone() const { return new VarProduct; }
  void copy(Var * src) { product = static_cast<VarProduct*>(src)->product; }
  string info() const { return "Product var: " + product.to_string(); }
  string type_info() const { return "Product var"; }
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
  void free() { if (index_exp->type != VAR) delete index_exp; }
};

struct ListWrite : public Exp, public ListAccess
{
  Exp * rexp = nullptr;

  ListWrite(const string & name, Exp * idx_exp, Exp * exp) 
    : Exp(LISTWRITE), ListAccess(name, idx_exp), rexp(exp) {}
  ExecStatus execute();
  void free() 
  {
    if (index_exp->type != VAR) 
      delete index_exp; 
    if (rexp->type != VAR)
      delete rexp;
  }
};

struct Search : public Exp
{
  string mapa_name;
  Exp * exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  string str;
  Uid id;

  Search(Exp::Type type, const string & name, Exp * e) 
    : Exp(type), mapa_name(name), exp(e) {}
  ExecStatus semant_mapa();
  ExecStatus semant_string();
  ExecStatus semant_int();
  void free() { if (exp->type != Exp::Type::VAR)delete exp; }
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

struct SearchProductsCod : public SearchProducts
{
  SearchProductsCod(const string & map_name, Exp * cod_exp)
    : SearchProducts(SEARCHPRODUCTCOD, map_name, cod_exp) {}
  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProductsCodCmd : public SearchProductsCod
{
  using SearchProductsCod::SearchProductsCod; 
  ExecStatus execute();
};

struct SearchProductsRif : public SearchProducts
{
  SearchProductsRif(const string & map_name, Exp * rif_exp)
    : SearchProducts(SEARCHPRODUCTRIF, map_name, rif_exp) {}
  ExecStatus semant();
  ExecStatus execute() { return semant(); }
};

struct SearchProductsRifCmd : public SearchProductsRif
{
  using SearchProductsRif::SearchProductsRif;
  ExecStatus execute();
};

struct SearchNode : public Search
{
  Net::Node * node_ptr = nullptr;

  SearchNode(const string & map_name, Exp * prod_exp) 
    : Search(SEARCHNODE, map_name, prod_exp) {}
  ExecStatus execute();
};

struct Load : public Exp
{
  Exp * name_exp = nullptr;
  MetaMapa mapa;

  Load(Exp * exp) noexcept : Exp(MAP), name_exp(exp) {}

  ExecStatus execute();
  void free() { if (name_exp->type != VAR) delete name_exp; }
};

struct Assign : public Exp
{
  string left_name;
  Exp * right_side = nullptr;

  Assign(string lname, Exp * rhs) 
    : Exp(ASSIGN), left_name(move(lname)), right_side(rhs) {}

  ExecStatus execute();
  void free() { delete right_side; }
};

struct Dot : public Exp
{
  const string net_name;
  Exp * file_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  Net * net_ptr  = nullptr;
  string file_name;
  Dot(const string & name, Exp * str_exp) 
    : Exp(DOT), net_name(name), file_exp(str_exp) {}
  ExecStatus execute();
  void free() { if (file_exp->type != Exp::Type::VAR) delete file_exp; }
};

struct Save : public Exp
{
  const string mapa_name;
  const string net_name;
  Exp * str_exp = nullptr;
  MetaMapa * mapa_ptr = nullptr;
  
  Save(const string & mname, const string & nname, Exp * exp)
    : Exp(SAVE), mapa_name(mname), net_name(nname), str_exp(exp) {}

  ExecStatus execute();
  void free() { if (str_exp->type != VAR) delete str_exp; }
};




# endif // NET_TREE_H
