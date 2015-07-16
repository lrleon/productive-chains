
# include <tpl_sort_utils.H>
# include <tpl_dynSetTree.H>
# include <tpl_agraph.H>
# include <io_graph.H>
# include <common.H>



/* Un nodo representa a una planta asociada a un rif o a un proveedor
   extranjero.  
   
   Para el proveedor extranjero, el valor de planta_id es -2 y el nombre
   del país contiene la cadena "pais_origen_id pais_procedencia_id"
 */
struct Nodo
{
  string rif    = "Sin rif"; // si el proveedor es extranjero ==> par id
  string nombre = "Sin nombre"; 
  bool extranjero = false;

  bool es_extranjero() const noexcept { return extranjero; }
  bool es_nacional() const noexcept { return not es_extranjero(); }

  pair<Uid, Uid> pais() const
  {
    assert(es_extranjero());
    Uid origen_id, procedencia_id;
    istringstream s(rif);
    s >> origen_id >> procedencia_id;
    return make_pair(origen_id, procedencia_id);
  }
  
  static string rif_extranjero(Uid origen_id, Uid procedencia_id)
  {
    stringstream s;
    s << origen_id << " " << procedencia_id;
    return s.str();
  }

  Nodo(const string & r = "Sin rif") : rif(r) {}

  Nodo(bool ext, const string & r, const string & nom)
    : rif(r), nombre(nom), extranjero(ext) {}

  Nodo(const string & r, const string & nom) 
    : rif(r), nombre(nom), extranjero(false) {}

  Nodo(Uid origen_id, Uid procedencia_id, const string & nom) 
    : rif(rif_extranjero(origen_id, procedencia_id)), 
      nombre(nom), extranjero(true) {}

  string to_string() const
  {
    return (es_extranjero() ? "EXT " : "EXT") + nombre + " " + rif; 
  }

  friend ostream & operator << (ostream & out, const Nodo & p)
  {
    return out << p.to_string();
  }

  struct CmpLess
  {
    bool operator () (const Nodo & n1, const Nodo & n2) const noexcept
    {
      if (n1.es_extranjero() or n2.es_extranjero()) // alguno extranjero?
	{
	  if (n1.es_nacional())
	    return true; // n1 Nacional < n2 Extranjero
	  if (n2.es_nacional())
	    return false; // n1 extranjero > n2 nacional
	  
	  const auto p1 = n1.pais();
	  const auto p2 = n2.pais();
	  if (p1.first == p2.first)
	    return p1.second < p2.second;
          return p1.first < p2.first;
	}
      return n1.rif < n2.rif; // Ambos son nacionales
    }
  };

  struct CmpEqual
  {
    bool operator () (const Nodo & n1, const Nodo & n2) const noexcept
    {
      if (n1.es_extranjero() or n2.es_extranjero()) // alguno extranjero?
	{
	  if (n1.es_nacional() or n2.es_nacional())
	    return false; 
	  
	  const auto p1 = n1.pais();
	  const auto p2 = n2.pais();
	  return p1.first == p2.first and p1.second == p2.second;
	}
      return n1.rif == n2.rif; // Ambos son nacionales
    }
  };
};

struct Arco
{
  Uid arco_id     = -1;
  string cod_aran = "Sin codigo";
  Uid insumo_id   = -1;          // id en la tabla de insumos 
  double cantidad = 0;
  double coste    = 0;

  Arco() {}

  Arco(Uid aid, const string & cod, Uid id, double cant, double c)
    : arco_id(aid), cod_aran(cod), insumo_id(id), cantidad(cant), coste(c) {}

  string to_string() const 
  {
    return ::to_string(arco_id) + " " + cod_aran + " " 
      + ::to_string(insumo_id) + 
      " " + ::to_string(cantidad) + " " + ::to_string(coste);
  }

  friend ostream & operator << (ostream & out, const Arco & a)
  {
    return out << a.to_string();
  }

  friend istream & operator >> (istream & in, Arco & a)
  {
    in >> a.arco_id >> a.cod_aran >> a.insumo_id >> a.cantidad >> a.coste;
    // cout << a.arco_id << a.cod_aran << a.insumo_id << a.cantidad << a.coste
    // 	 << endl;
    return in;
  }
};


using GrafoSigesic = Array_Graph<Graph_Anode<Nodo>, Graph_Aarc<Arco>>;
using SG = GrafoSigesic;

static const string Quotes = "\"";

struct Guarda_Nodo
{
  void operator () (ostream & out, SG &, SG::Node * p) const
  {
    const auto & info = p->get_info();
    out << (info.es_extranjero() ? "E" : "V") << endl
	<< info.rif << endl
	<< info.nombre << endl;
  }
};
  
struct Carga_Nodo
{
  void operator () (istream & in, SG &, SG::Node * p) const
  {
    string nacional, rif, nombre;
    
    getline(in, nacional);
    getline(in, rif);
    getline(in, nombre);

    auto ptr = &p->get_info();
    if (nacional == "V")
      new (ptr) Nodo(rif, nombre);
    else 
      new (ptr) Nodo(true, rif, nombre);
  }
};
  

class Mapa
{
public:

  using Grafo = GrafoSigesic;
  using Node  = Grafo::Node;
  using Arc   = Grafo::Arc;

private:

  Grafo g;

  DynList<string> log;

  long anho = -1;

  const TablaUnidadesEconomicas tabla_unidades;
  const TablaPlantas tabla_plantas;
  const TablaProductos tabla_productos;
  const TablaInsumos tabla_insumos;
  const TablaProveedorInsumo tabla_proveedor_insumo;
  const TablaProveedores tabla_proveedores;

  struct CmpNode
  {
    bool operator () (Node * p1, Node * p2) const noexcept
    {
      return Nodo::Cmp()(p1->get_info(), p2->get_info());
    }
  };

  DynSetTree<Node*, Avl_Tree, CmpNode> tabla_nodos;

  Node * search_node(const string & rif) const 
  {
    Node node(rif);
    auto ptr = tabla_nodos.search(&node);
    if (ptr == nullptr)
      return nullptr;
    assert((*ptr)->get_info().rif == rif);
    return *ptr;
  }

  Node * target_node(Uid producto_id)
  {
    Producto * producto_ptr = tabla_productos(producto_id);
    assert(producto_ptr->id == producto_id);
    Planta * planta_ptr = tabla_plantas(producto_ptr->planta_id);
    assert(planta_ptr->id == producto_ptr->planta_id);
    auto p = search_node(planta_ptr->rif);
    if (p != nullptr)
      {
	assert(p->get_info().rif == planta_ptr->rif);
	return p;
      }

    p = g.emplace_node(planta_ptr->rif, planta_ptr->nombre);
    tabla_nodos.insert(p);
    return p;
  }

  pair<DynList<Node*>, string> filter_cod_aran(const DynList<Node*> & l, 
					       Insumo * insumo_ptr) const
  {
    const string & cod_aran = insumo_ptr->cod_aran;
    
    DynSetTree<string> rifs;// Recorre productos asociados codigo arancelario
    auto productos = tabla_productos.por_cod_aran(cod_aran);
    for (auto it = productos.get_it(); it.has_curr(); it.next())
      {
	auto producto_ptr = it.get_curr();
	auto planta_ptr = tabla_plantas(producto_ptr->planta_id);
	if (planta_ptr == nullptr)
	  continue;
	rifs.append(planta_ptr->rif);
      }

    auto ret = l.filter([this, &rifs] (auto p)
      {
	const auto & info = p->get_info();
	if (info.es_extranjero())
	  return true;
	return rifs.contains(info.rif);
      });
    
    return make_pair(ret, cod_aran);
  }

  pair<DynList<Node*>, string> source_node(Uid insumo_id)
  {
    DynList<Node*> l;
    Insumo * insumo_ptr = tabla_insumos(insumo_id);
    assert(insumo_ptr->id == insumo_id);
    auto provs = tabla_proveedor_insumo(insumo_id);
    if (provs.is_empty())
      {
	stringstream s;
	s << "Insumo id " << insumo_id << " no tiene proveedores declarados "
	  " para el año " << anho;
	log.append(s.str());
	return make_pair(l, "");
      }

    // recorre los proveedores del insumo para determinar nodos fuente
    for (auto it = provs.get_it(); it.has_curr(); it.next())
      {
	auto prov = it.get_curr();
	assert(prov->insumo_id == insumo_id);
	Proveedor * prov_ptr = tabla_proveedores(prov->proveedor_id);
	assert(prov_ptr != nullptr);

	string rif = prov_ptr->rif, nombre = prov_ptr->nombre;
	bool es_extranjero = true;
	if (prov_ptr->es_nacional())
	  {
	    es_extranjero = false;
	    auto unidad_ptr = tabla_unidades(prov_ptr->rif);
	    if (unidad_ptr == nullptr)
	      {
		stringstream s;
		s << "Rif " << prov_ptr->rif << " asociado a proveedor id "
		  << prov_ptr->id << " no fue encontrado como unid ecom";
		log.append(s.str());
		nombre += " **SR";
	      }
	  }
	else
	  rif = Nodo::rif_extranjero(prov_ptr->pais_origen_id, 
				     prov_ptr->pais_procedencia_id);

	Node node(Nodo(es_extranjero, move(rif), move(nombre)));
	auto ptr = tabla_nodos.search(&node);
	if (ptr != nullptr)
	  {
	    l.append(*ptr);
	    continue;
	  }

	auto p = g.insert_node(move(node.get_info()));
	tabla_nodos.insert(p);
	l.append(p);
      }
    
    return filter_cod_aran(l, insumo_ptr);
  }

  void read_arcs(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	if (atol(row(3).c_str()) != anho)
	  continue;

	const Uid producto_id = atol(row(2).c_str());
	auto tgt = target_node(producto_id);

	const Uid insumo_id = atol(row(1).c_str());
	auto par = source_node(insumo_id);
	// cout << "Sources:" << endl;
	// par.first.for_each([] (auto p)
        //   {
	//     cout << "    " << p->get_info().rif << " "; 
	//   });
	// cout << endl;

	for (auto it = par.first.get_it(); it.has_curr(); it.next())
	  {
	    // auto a = 
	      g.emplace_arc(it.get_curr(), tgt, atol(row(0).c_str()), 
				   par.second, insumo_id, 0, 0);
	    // cout << it.get_curr()->get_info().rif << " [" << a->get_info() 
	    // 	 << "] " << tgt->get_info().rif << endl;
	  }
	// cout << "++++++++++++++++" << endl;
      }
  }
  
public:

  Mapa(long __anho,
       const char * nom_unidades,
       const char * nom_plantas, 
       const char * nom_productos,
       const char * nom_insumos,
       const char * nom_prov_ins,
       const char * nom_proveedores,
       const char * nom_arcos)
    : anho(__anho), tabla_unidades(nom_unidades), tabla_plantas(nom_plantas), 
      tabla_productos(nom_productos), tabla_insumos(nom_insumos),
      tabla_proveedor_insumo(nom_prov_ins, anho),
      tabla_proveedores(nom_proveedores)
  {
    read_arcs(nom_arcos);
  }

  void save(ostream & out)
  {
    IO_Graph<Grafo, Carga_Nodo, Guarda_Nodo>(g).save_in_text_mode(out);
  }
};

void guardar_grafo(GrafoSigesic & g, ostream & out)
{
  IO_Graph<GrafoSigesic, Carga_Nodo, Guarda_Nodo>(g).save_in_text_mode(out);
}


GrafoSigesic cargar_grafo(istream & in)
{
  GrafoSigesic g;
  
  IO_Graph<GrafoSigesic, Carga_Nodo, Guarda_Nodo>(g).load_in_text_mode(in);

  return g;
}
