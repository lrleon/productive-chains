
# ifndef NET_H
# define NET_H

# include <regex>
# include <iomanip>
# include <tpl_union.H>
# include <tpl_test_path.H>
# include <grafo.H>

using Net = Array_Graph<Graph_Anode<Productor*>, Graph_Aarc<Arco>>;

struct SaveNetNode
{
  TablaProductores * tabla_productores_ptr = nullptr;

  SaveNetNode() {}

  SaveNetNode(const TablaProductores & tbl) 
    : tabla_productores_ptr(&const_cast<TablaProductores&>(tbl)) {}

  void operator () (ostream & out, Net&, Net::Node * p) const
  {
    out << (*(tabla_productores_ptr))(p->get_info()) << endl;
  }
};

struct LoadNetNode
{
  TablaProductores * tabla_productores_ptr = nullptr;

  LoadNetNode() {}

  LoadNetNode(const TablaProductores & tbl) 
    : tabla_productores_ptr(&const_cast<TablaProductores&>(tbl)) {}

  void operator () (istream & in, Net&, Net::Node * p) const
  {
    size_t idx;
    in >> idx;
    p->get_info() = &tabla_productores_ptr->access(idx);
  }
};

struct LoadArc
{
  TablaProductores * tabla_productores_ptr = nullptr;
  Fixed_Relation * relation_ptr = nullptr;

  LoadArc() {}
  LoadArc(TablaProductores * tbl_ptr, Fixed_Relation * ptr) 
    : tabla_productores_ptr(tbl_ptr), relation_ptr(ptr) {}

  void operator () (istream & input, Net & net, Net::Arc * a)
  {
    auto src = net.get_src_node(a);
    auto tgt = net.get_tgt_node(a);
    auto prod_src_ptr = src->get_info();
    auto prod_tgt_ptr = tgt->get_info();
    size_t src_idx = (*tabla_productores_ptr)(prod_src_ptr);
    size_t tgt_idx = (*tabla_productores_ptr)(prod_tgt_ptr);
    relation_ptr->join(src_idx, tgt_idx);
    input >> a->get_info();
  }
};

extern Net build_net(const GrafoSigesic & g, 
		     const TablaProductores & tabla_productores);

extern void save_net(const Net & net, 
		     const TablaProductores & tabla_productores,
		     ostream & out);

struct MetaMapa
{
  mutable TablaProductores     tabla_productores;
  mutable TablaMetaProductos   tabla_productos;
  mutable TablaMetaInsumos     tabla_insumos;
  mutable TablaMetaSocios      tabla_socios;
  mutable Net                  net;
  mutable DynArray<Net::Node*> nodes;
  mutable Fixed_Relation       relation;
  
  using Node = Net::Node;
  using Arc = Net::Arc;

  typedef bool (*CmpRif)(Net::Node*, Net::Node*);

  // Nodos están ordenado por rif a efectos de encontrarlos
  // rapidamente. Esta es la lambda de comparación para ordenar los
  // nodos en el arreglo nodes y para buscarlos por la búsqueda binaria
  CmpRif cmprif = [] (Net::Node * p1, Net::Node * p2) 
  {
    return p1->get_info()->rif < p2->get_info()->rif;
  };

  void build_relation()
  {
    if (::verbose)
      cout << "    Computing network blocks ... " << endl;

    const size_t & n = net.vsize();
    nodes.reserve(n);
    size_t i = 0;
    net.for_each_node([&i, this] (auto p) { nodes(i++) = p; });
    in_place_sort(nodes, cmprif);

    relation.set_n(n);
    net.for_each_arc([this] (auto a)
      {
	auto src = net.get_src_node(a);
	auto tgt = net.get_tgt_node(a);
	size_t src_idx = binary_search(nodes, src, cmprif);
	size_t tgt_idx = binary_search(nodes, tgt, cmprif);
	assert(src_idx < net.vsize() and tgt_idx < net.vsize());
	relation.join(src_idx, tgt_idx);
      });

    if (::verbose)
      cout << "    networks blocsk already computed" << endl;
  }

  MetaMapa() {}

  MetaMapa(ifstream & in) 
    : tabla_productores(in), tabla_productos(in), 
      tabla_insumos(in), tabla_socios(in)
  {
    if (::verbose) 
      cout << "Loading Net ... " << endl;

    LoadNetNode l(tabla_productores);
    IO_Graph<Net, LoadNetNode, SaveNetNode> io(const_cast<Net&>(net));
    io.set_load_node(l);
    io.load_in_text_mode(in);

    build_relation();

    if (::verbose)
      cout << " all map done!" << endl 
	   << endl;
  }

  void save(ostream & out) const
  {
    tabla_productores.save(out);
    tabla_productos.save(out);
    save_net(net, tabla_productores, out);
  }

  void autotest() const
  {
    tabla_productores.autotest();
    tabla_productos.autotest();
  }

  // Retorna unalista de productos cuyo nombre empareje la expresión tegular
  DynList<MetaProducto*> productos_by_nom(const string & exp) const 
  {
    DynList<MetaProducto*> ret;
    regex reg(exp);
    tabla_productos.for_each([&ret, &reg] (auto & p)
      {
	if (regex_search(p.nombre, reg))
	  ret.append(const_cast<MetaProducto*>(&p));
      });
    return ret;
  }

  DynList<MetaProducto*> productos_by_rif(const string & rif) const noexcept
  {
    DynList<MetaProducto*> ret;
    auto productor = tabla_productores(rif);
    if (productor == nullptr)
      return ret;

    productor->productos.for_each([&ret, this] (auto t)
       {
	 ret.append(tabla_productos(get<0>(t)));
       });

    return ret;
  }

  DynList<Productor*> producers_by_name(const string & exp) const 
  {
    DynList<Productor*> ret;
    regex reg(exp);
    tabla_productores.for_each([&ret, &reg] (auto & p)
      {
	if (regex_search(p.nombre, reg))
	  ret.append(const_cast<Productor*>(&p));
      });

    return ret;
  }

  DynList<MetaProducto*> productos_by_cod_aran(const string & cod) 
    const noexcept
  {
    return tabla_productos(cod);
  }

  Node * search_node(const string & rif) const noexcept
  {
    Productor p; p.rif = rif;
    Net::Node node; node.get_info() = &p;
    auto ptr = bsearch(nodes, &node, cmprif);
    if (ptr == nullptr)
      return nullptr;
    return *ptr;
  }

  Node * search_node(const Productor & prod) const noexcept
  {
    return search_node(prod.rif);
    return net.search_node([&prod] (auto ptr) 
			   {
			     return ptr->get_info()->rif == prod.rif;
			   });
  }

  DynList<Node*> search_node_regex(const string & exp) const noexcept
  {
    regex reg(exp);
    return net.filter_nodes([&reg] (auto p) 
      {
	return regex_search(p->get_info()->nombre, reg);
      });
  }

  bool reachable(Node * src, Node * tgt) const
  {
    size_t src_idx = binary_search(nodes, src, cmprif);
    size_t tgt_idx = binary_search(nodes, tgt, cmprif);
    cout << "src = " << src->get_info()->rif << endl
	 << "tgt = " << tgt->get_info()->rif << endl
	 << "src_idx = " << src_idx << endl
	 << "tgt_idx = " << tgt_idx << endl;
    return relation.are_connected(src_idx, tgt_idx);
  }

private:

  /* 
     Retorna los productos producidos por productor_ptr que tienen
     codigo arancelario cod_aran
   */
  DynList<MetaProducto*> inter_productor_cod_aran(Productor * productor_ptr,
						  const string & cod_aran) const
  {
    DynSetTree<string> productos_productor;
    productor_ptr->productos.for_each([&productos_productor] (auto p)
      {
	productos_productor.append(p.second);
      });

        // todos los productos con cod_aran
    DynList<MetaProducto*> ret;
    tabla_productos(cod_aran).for_each([&ret, &productos_productor] (auto ptr)
      {
	if (productos_productor.has(ptr->cod_aran))
	  ret.append(ptr);
      });

    return ret;
  }

  /* 
     Retorna el primer producto encontrado cuya combinación linea quepa
     enteramente en el conjunto de arcos de entrada de tgt 
  */
  MetaProducto * first_match(Node * tgt, 
			     const DynList<MetaProducto*> & productos) const
  {
    // Códigos arancelarios de todos los arcos entrantes a tgt
    DynSetHash<string> arc_codes;
    for (auto it = net.get_in_it(tgt); it.has_curr(); it.next())
      arc_codes.append(it.get_curr()->get_info().cod_aran);

    for (auto it = productos.get_it(); it.has_curr(); it.next())
      {
	auto producto_ptr = it.get_curr();
	if (producto_ptr->comb.all([&arc_codes] (auto p)
				   { 
				     return arc_codes.has(get<1>(p));
				   }))
	  return producto_ptr;
      }
    return nullptr;
  }

  void upstream(Node * stgt,                     // Nodo de partida
		MetaProducto * tgt_producto_ptr, // productor de cod_aran
		Net & net_up,                    // red aguas arriba resultado
		Node * ttgt) const               // imagen de stgt en net_up
  {
    assert(stgt->get_info() == ttgt->get_info());
    const auto & tgt_productor = *stgt->get_info();

    assert(tgt_productor.productos.exists([tgt_producto_ptr] (auto p)
           { return p.second  == tgt_producto_ptr->cod_aran; }));

    // Recorrer los arcos de src y visitar aquellos relacionados a inputs
    const auto & inputs = tgt_producto_ptr->comb; // Insumos de prod
    for (auto it = net.get_in_it(stgt); it. has_curr(); it.next())
      {
	auto a = it.get_curr();
	if (not inputs.exists([id = a->get_info().insumo_id] (auto c)
			      { return id == get<0>(c); }))
	  continue;
	
	auto ssrc = net.get_src_node(a);
	auto tsrc = net_up.insert_node(ssrc->get_info());
	net_up.insert_arc(tsrc, ttgt, a->get_info());

	string & cod_aran = a->get_info().cod_aran;
	auto productor_ptr = ssrc->get_info();
 	auto productos = inter_productor_cod_aran(productor_ptr, cod_aran);
	MetaProducto * first_prod = first_match(ssrc, productos);

	upstream(ssrc, first_prod, net_up, tsrc);
      }
  }

public:

  Net upstream(Node * ssrc, MetaProducto * producto_ptr) const
  {
    Net ret;

    auto tsrc = ret.insert_node(ssrc->get_info());
    upstream(ssrc, producto_ptr, ret, tsrc);

    return ret;
  }

  // {
  //   DynList<Node*> ret;

  //   return ret;
  // }

  // Net build_subcomponent(

  // DynList<Node*> nodos_insumo
};


struct Write_Node
{
  static const size_t N = 15;

  void operator () (const Net&, Net::Node * p, ostream & out) const
  {
    auto prod_ptr = p->get_info();
    string nom = prod_ptr->nombre;
    nom.erase(std::remove(nom.begin(), nom.end(), '"'), nom.end());
    if (nom.size() > 15)
      nom.erase(N, std::string::npos);
    out << "label = \"" << prod_ptr->rif << "\\n" 
	<< nom << "\"";
  }
};

struct Write_Arc
{
  TablaMetaInsumos * tabla_insumos_ptr = nullptr;

  Write_Arc(const TablaMetaInsumos & tabla_insumos)
    : tabla_insumos_ptr(const_cast<TablaMetaInsumos*>(&tabla_insumos)) {}

  void operator () (const Net&, Net::Arc * a, ostream & out) const
  {
    const auto & info = a->get_info();
    auto prod_ptr = (*tabla_insumos_ptr)(info.insumo_id);
    string nom = prod_ptr->nombre;
    nom.erase(std::remove(nom.begin(), nom.end(), '"'), nom.end());
    out << "label = \"" << nom << " (" 
	<< info.arco_id << " " << info.insumo_id << ")" 
	<< "\\n"
	<< std::setprecision(8) 
	<< info.cod_aran << " " << info.cantidad << " " << info.coste 
	<< "\"";
  }
};

# endif 
