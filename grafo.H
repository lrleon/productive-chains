
# include <tpl_sort_utils.H>
# include <tpl_dynSetTree.H>
# include <tpl_agraph.H>
# include <common.H>

/* Un nodo representa a una planta asociada a un rif o a un proveedor
   extranjero.  
   
   Para el proveedor extranjero, el valor de planta_id es -2 y el nombre
   del país contiene la cadena "pais_origen_id pais_procedencia_id"
 */
struct Nodo
{
  static const Uid EXTRANJERO = -2;
  string rif    = "Sin rif"; // si el proveedor es extranjero ==> par id
  string nombre = "Sin nombre"; 
  Uid planta_id = -1; // Valor EXTRANJERO ==> proveedor es extranjero

  bool es_extranjero() const noexcept { return planta_id == EXTRANJERO; }
  bool es_nacional() const noexcept { return not es_extranjero(); }

  pair<Uid, Uid> pais() const
  {
    assert(es_extranjero());
    Uid origen_id, procedencia_id;
    istringstream s(rif);
    s >> origen_id >> procedencia_id;
    return make_pair(origen_id, procedencia_id);
  }

  const auto & origen_id(const pair<Uid, Uid> & p) const 
  {
    return p.first; 
  }

  const auto & procedencia_id(const pair<Uid, Uid> & p) const 
  {
    return p.second; 
  }

  void set_country(Uid origen_id, Uid procedencia_id)
  {
    planta_id = EXTRANJERO;
    stringstream s;
    s << origen_id << " " << procedencia_id;
    rif = s.str();
  }

  Nodo() {}

  Nodo(const string & r, const string & nom, Uid pid) 
    : rif(r), nombre(nom), planta_id(pid) {}

  string to_string() const
  {
    if (es_extranjero())
      return "EXT " + nombre + " " + rif;
    return "VEN " + rif + " " + nombre + " " + ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Nodo & p)
  {
    return out << p.to_string();
  }

  struct Cmp
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
      // Ambos son nacionales
      if (n1.planta_id == n2.planta_id)
	return n1.rif < n2.rif;
      return n1.planta_id < n2.planta_id;
    }
  };
};

struct Arco
{
  string cod_aran = "Sin codigo";
  Uid insumo_id   = -1;          // id en la tabla de insumos
  double cantidad = 0;
  double coste    = 0;

  Arco() {}

  Arco(const string & cod, Uid id, double cant, double c)
    : cod_aran(cod), insumo_id(id), cantidad(cant), coste(c) {}

  string to_string() const 
  {
    return cod_aran + " " + ::to_string(insumo_id) + " " + 
      ::to_string(cantidad) + " " + ::to_string(coste);
  }

  friend ostream & operator << (ostream & out, const Arco & a)
  {
    return out << a.to_string();
  }
};


class Mapa
{
public:

  using Grafo = Array_Graph<Graph_Anode<Nodo>, Graph_Aarc<Arco>>;
  using Node  = Grafo::Node;
  using Arc   = Grafo::Arc;

private:

  Grafo g;

  DynList<string> log;

  long anho = -1;

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

  Node * target_node(Uid producto_id)
  {
    Producto * producto_ptr = tabla_productos(producto_id);
    assert(producto_ptr->id == producto_id);
    Planta * planta_ptr = tabla_plantas(producto_ptr->planta_id);
    assert(planta_ptr->id == producto_ptr->planta_id);
    Mapa::Node n(Nodo(planta_ptr->rif, producto_ptr->nombre, planta_ptr->id));
    auto ptr = tabla_nodos.search(&n);
    if (ptr != nullptr)
      return *ptr;

    auto p = g.insert_node(Nodo(planta_ptr->rif, producto_ptr->nombre, 
				planta_ptr->id));
    tabla_nodos.insert(p);
    return p;
  }

  DynList<Node*> source_node(Uid insumo_id)
  {
    DynList<Node*> ret;
    Insumo * insumo_ptr = tabla_insumos(insumo_id);
    assert(insumo_ptr->id == insumo_id);
    auto provs = tabla_proveedor_insumo(insumo_id);
    if (provs.is_empty())
      {
	stringstream s;
	s << "Insumo id " << insumo_id << " no tiene proveedores declarados "
	  " para el año " << anho;
	log.append(s.str());
	return ret;
      }

    provs.for_each([this, &ret] (const auto & p)
    {
      Nodo n;
      Proveedor * prov_ptr = tabla_proveedores(p->proveedor_id);
      assert(prov_ptr != nullptr);
      // auto planta_ptr = tabla_plantas(
      if (prov_ptr == nullptr) 
	{    // proveedor es extranjero
	  cout << "Supuesto proveedor extranjero" << endl;
	  n.set_country(prov_ptr->pais_origen_id, 
			prov_ptr->pais_procedencia_id);
	  n.nombre = prov_ptr->nombre;
	}
      else 
	{
	  n.rif = prov_ptr->rif;
	  n.nombre = prov_ptr->nombre;
	  n.planta_id = prov_ptr->id;
	}

      Node node(move(n));
      auto ptr = tabla_nodos.search(&node);
      if (ptr != nullptr)
	ret.append(*ptr);
      else
	{
	  auto p = g.insert_node(move(node.get_info()));
	  tabla_nodos.insert(p);
	  ret.append(p);
	}
    });
    
    return ret;
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
	if (i % 100000 == 0)
	  cout << "Read " << i << " entries" << endl;
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	if (atol(row(3).c_str()) != anho)
	  continue;

	const Uid producto_id = atol(row(2).c_str());
	auto src = target_node(producto_id);

	const Uid insumo_id = atol(row(1).c_str());
	auto tgt_list = source_node(insumo_id);
	// tgt_list.for_each([] (auto p) { cout << p->get_info() << endl; });
      }
    
    g.for_each_node([] (auto p) { cout << p->get_info() << endl; });
    cout << "Insertados " << g.vsize() << " nodos" << endl; 
  }
  
public:

  Mapa(long __anho,
       const char * nom_plantas, 
       const char * nom_productos,
       const char * nom_insumos,
       const char * nom_prov_ins,
       const char * nom_proveedores,
       const char * nom_arcos)
    : anho(__anho), tabla_plantas(nom_plantas), 
      tabla_productos(nom_productos), tabla_insumos(nom_insumos),
      tabla_proveedor_insumo(nom_prov_ins, anho),
      tabla_proveedores(nom_proveedores)
  {
    read_arcs(nom_arcos);
  }

};
