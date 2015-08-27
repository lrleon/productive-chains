
# ifndef NET_H
# define NET_H

# include <regex>
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
    : tabla_productores(in), tabla_productos(in), tabla_socios(in)
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
  // {
  //   DynList<Node*> ret;

  //   return ret;
  // }

  // Net build_subcomponent(

  // DynList<Node*> nodos_insumo
};


# endif 
