
# ifndef GRAFO_H
# define GRAFO_H

# include <tpl_sort_utils.H>
# include <tpl_dynSetTree.H>
# include <tpl_agraph.H>
# include <io_graph.H>

# include <tablas.H>

extern bool verbose;
extern size_t verbose_cycles;

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
  
  static string rif_ext(Uid origen_id, Uid procedencia_id, string nombre)
  {
    return ::rif_ext(origen_id, procedencia_id, move(nombre));
  }

  Nodo(const string & r = "Sin rif") : rif(r) {}

  Nodo(bool ext, const string & r, const string & nom)
    : rif(r), nombre(nom), extranjero(ext) {}

  Nodo(const string & r, const string & nom) 
    : rif(r), nombre(nom), extranjero(false) {}

  Nodo(Uid origen_id, Uid procedencia_id, const string & nom) 
    : rif(rif_ext(origen_id, procedencia_id, nom)), 
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
      return n1.rif < n2.rif; 
    }
  };

  struct CmpEqual
  {
    bool operator () (const Nodo & n1, const Nodo & n2) const noexcept
    {
      return n1.rif == n2.rif;
    }
  };
};
 
struct Arco
{
  Uid arco_id     = -1;
  string cod_aran = "Sin codigo";
  Uid producto_id = -1;
  Uid insumo_id   = -1;          // id en la tabla de insumos 
  double factor   = 0; // cuanto requiere insumo_id para una unidad prod_id
  double cantidad = 0;
  double coste    = 0;

  Arco() {}

  Arco(Uid aid, const string & cod, Uid prod_id, Uid ins_id,
       double fact, double cant, double c)
    : arco_id(aid), cod_aran(cod), producto_id(prod_id), insumo_id(ins_id),
      factor(fact), cantidad(cant), coste(c) {}

  string to_string() const 
  {
    return ::to_string(arco_id) + " " + cod_aran + " " + 
      ::to_string(producto_id) + " " + ::to_string(insumo_id) + " " +
      ::to_string(factor) + " " + ::to_string(cantidad) + " " + 
      ::to_string(coste);
  }

  friend ostream & operator << (ostream & out, const Arco & a)
  {
    return out << a.to_string();
  }

  friend istream & operator >> (istream & in, Arco & a)
  {
    in >> a.arco_id >> a.cod_aran >> a.producto_id >> a.insumo_id 
       >> a.factor >> a.cantidad >> a.coste;
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

  Grafo g;

  DynList<string> log;

  long anho = -1;

  const TablaUnidadesEconomicas tabla_unidades;
  const TablaSocios tabla_socios;
  const TablaPlantas tabla_plantas;
  const TablaProductos tabla_productos;
  const TablaInsumos tabla_insumos;
  const TablaProveedorInsumo tabla_proveedor_insumo;
  const TablaProveedores tabla_proveedores;

  struct CmpNode
  {
    bool operator () (Node * p1, Node * p2) const noexcept
    {
      return Nodo::CmpLess()(p1->get_info(), p2->get_info());
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

  pair<Node*, Producto*> target_node(Uid producto_id)
  {
    Producto * producto_ptr = tabla_productos(producto_id);
    assert(producto_ptr->id == producto_id);
    Planta * planta_ptr = tabla_plantas(producto_ptr->planta_id);
    assert(planta_ptr->id == producto_ptr->planta_id);
    auto p = search_node(planta_ptr->rif);
    if (p == nullptr)
      {
	p = g.emplace_node(planta_ptr->rif, planta_ptr->nombre);
	tabla_nodos.insert(p);
      }
    
    return make_pair(p, producto_ptr);
  }

  //                rif, nombre-empresa, cantidad, coste
  using Fab = tuple<string, string, double, double>;

  // Produce una lista filtrada de los elementos de l que tengan los
  // rifs asociados al código arancelario cod_aran
  DynList<Fab>
  filter_cod_aran(const DynList<Fab> & l, const string & cod_aran) const
  {
    DynSetTree<string> rifs; // Recorre productos asociados codigo arancelario
    auto productos = tabla_productos.por_cod_aran(cod_aran);
    for (auto it = productos.get_it(); it.has_curr(); it.next())
      {
	auto producto_ptr = it.get_curr();
	auto planta_ptr = tabla_plantas(producto_ptr->planta_id);
	if (planta_ptr == nullptr)
	  continue;
	rifs.append(planta_ptr->rif);
      }

    return l.filter([&rifs] (auto f) { return rifs.contains(get<0>(f)); });
  }

  //                            cantidad, coste
  using SemiArco = tuple<Node*, double, double>;

  // TEMPORAL
  size_t casos_vacios = 0;
  size_t casos_no_vacios = 0;
  size_t casos_nacionales = 0;
  // retorna la lista de arcos y el código arancelario
  pair<DynList<SemiArco>, string> source_node(Uid insumo_id)
  {
    Insumo * insumo_ptr = tabla_insumos(insumo_id);
    assert(insumo_ptr->id == insumo_id);
    const string & cod_aran = insumo_ptr->cod_aran;
    auto provs = tabla_proveedor_insumo(insumo_id);
    if (provs.is_empty())
      {
	stringstream s;
	s << "Insumo id " << insumo_id << " no tiene proveedores declarados ";
	log.append(s.str());
	return make_pair(DynList<SemiArco>(), cod_aran);
      } 

    // recorre los proveedores de insumo y los separa en nacionales y extranj
    DynList<Fab> nac_list, ext_list;
    for (auto it = provs.get_it(); it.has_curr(); it.next())
      {
	auto prov = it.get_curr();
	assert(prov->insumo_id == insumo_id);
	Proveedor * prov_ptr = tabla_proveedores(prov->proveedor_id);
	assert(prov_ptr != nullptr);

	const string & rif = prov_ptr->rif;
	string nombre = prov_ptr->nombre;
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
		nombre += " **Sin Registro";
	      }
	    nac_list.append(make_tuple(rif, move(nombre), 
				       prov->cantidad, prov->monto));
	  }
	else
	  {
	    auto t = make_tuple(Nodo::rif_ext(prov_ptr->pais_origen_id, 
					      prov_ptr->pais_procedencia_id,
					      nombre),
				move(nombre), prov->cantidad, prov->monto);
	    ext_list.append(move(t));
	  }
      }

    DynList<SemiArco> nac_nodos;
    // if par es vacío hay que registrar al menos un nodo. se trataria
    // de un proveedor que
    if (not nac_list.is_empty())
      {
	++casos_nacionales;
	auto producers = filter_cod_aran(nac_list, cod_aran);
	if (producers.is_empty())
	  {
	    ++casos_vacios;
	    nac_nodos = nac_list.map<SemiArco>([this] (auto t)
              {
		const string & rif = get<0>(t);
		Node node(Nodo(rif, move(get<1>(t))));
		auto ptr = tabla_nodos.search(&node);
		if (ptr != nullptr)
		  return make_tuple(*ptr, get<2>(t), get<3>(t));

		auto p = g.insert_node(move(node.get_info()));
		tabla_nodos.insert(p);
		return make_tuple(p, get<2>(t), get<3>(t));
	      });
	  }
	else 
	  {
	    ++casos_no_vacios;
	    nac_nodos = producers.map<SemiArco>([this] (auto & t)
              {
		Node node(Nodo(move(get<0>(t)), move(get<1>(t))));
		auto ptr = tabla_nodos.search(&node);
		if (ptr != nullptr)
		  return make_tuple(*ptr, get<2>(t), get<3>(t));

		auto p = g.insert_node(move(node.get_info()));
		tabla_nodos.insert(p);
		return make_tuple(p, get<2>(t), get<3>(t));
	      });
	  }
      }

    auto ext_nodos = ext_list.map<SemiArco>([this] (auto & t)
      {
	Node node(Nodo(true, move(get<0>(t)), move(get<1>(t))));
	auto ptr = tabla_nodos.search(&node);
	if (ptr != nullptr)
	  return make_tuple(*ptr, get<2>(t), get<3>(t));

	auto p = g.insert_node(move(node.get_info()));
	tabla_nodos.insert(p);
	return make_tuple(p, get<2>(t), get<3>(t));
      });

    nac_nodos.append(ext_nodos); // concatena ext_nodos a nac_nodos
    
    return make_pair(nac_nodos, cod_aran);
  }

  void read_arcs(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    if (::verbose)
      cout << "Reading arcs ..." << endl;

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	if (::verbose and i % verbose_cycles == 0)
	  cout << "    i = " << i << endl;
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	if (atol(row(3).c_str()) != anho)
	  continue;

	const Uid producto_id = atol(row(2).c_str());
	auto par = target_node(producto_id);
	auto tgt = par.first;
	auto producto_ptr = par.second;

	const Uid insumo_id = atol(row(1).c_str());
	auto lt = source_node(insumo_id);

	Uid arc_id = atol(row(0).c_str());
	double cantidad = atof(row(4).c_str());
	const string & cod_aran = get<1>(lt);

	if (get<0>(lt).is_empty())
	  {
	    stringstream s;
	    s << "arco id " << arc_id << " " << insumo_id << "[" << cod_aran
	      << "] " << producto_id << "sin productor de insumo";
	    // cout << "i = " << i << " " <<  insumo_id << " [" << arc_id << "] "
	    //  	 << producto_id << endl;
	    log.append(s.str());
	  }

	// assert(not get<0>(lt).is_empty());

	producto_ptr->comb.insert(make_tuple(insumo_id, cod_aran, 
					     cantidad, arc_id));

	for (auto it = get<0>(lt).get_it(); it.has_curr(); it.next())
	  {
	    auto & t = it.get_curr();
	    g.emplace_arc(get<0>(t), tgt, arc_id, cod_aran,
			  producto_id, insumo_id, 
			  cantidad, get<1>(t), get<2>(t));
	  }
      }
    cout << "Casos nacionales = " << casos_nacionales << endl
	 << "Casos vacios     = " << casos_vacios << endl
	 << "Casos no vacios  = " << casos_no_vacios << endl;
  }
  
public:

  Mapa(long __anho,
       const char * nom_unidades,
       const char * nom_socios,
       const char * nom_plantas, 
       const char * nom_productos,
       const char * nom_insumos,
       const char * nom_prov_ins,
       const char * nom_proveedores,
       const char * nom_arcos)
    : anho(__anho), tabla_unidades(nom_unidades), tabla_socios(nom_socios),
      tabla_plantas(nom_plantas), tabla_productos(nom_productos),
      tabla_insumos(nom_insumos), tabla_proveedor_insumo(nom_prov_ins, anho),
      tabla_proveedores(nom_proveedores)
  {
    read_arcs(nom_arcos);
  }

  void save(ostream & out)
  {
    if (::verbose) 
      cout << "Saving graph" << endl;
    IO_Graph<Grafo, Carga_Nodo, Guarda_Nodo>(g).save_in_text_mode(out);
    if (::verbose)
      cout << "done!" << endl
	   << endl;
  }
};

extern void guardar_grafo(GrafoSigesic & g, ostream & out);

extern GrafoSigesic cargar_grafo(istream & in);


# endif // GRAFO_H
