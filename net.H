
# ifndef NET_H
# define NET_H

# include <regex>
# include <iomanip>
# include <vector>
# include <tpl_graph_utils.H>
# include <grafo.H>
# include <utils.H>


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

  void save(const Net & subnet, ostream & out) const
  {
    // mutable TablaProductores     tabla_productores;
  // mutable TablaMetaProductos   tabla_productos;
  // mutable TablaMetaInsumos     tabla_insumos;
  // mutable TablaMetaSocios      tabla_socios;
  // mutable Net                  net;
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
    return net.search_node([&rif] (auto p) 
			   {
			     return p->get_info()->rif == rif; 
			   });
  }

  Node * search_node(const Productor & prod) const noexcept
  {
    return search_node(prod.rif);
    return net.search_node([&prod] (auto ptr) 
			   {
			     return ptr->get_info()->rif == prod.rif;
			   });
  }

  DynList<Node*> search_node_regex(const string & exp) const
  {
    regex reg(exp);
    return net.filter_nodes([&reg] (auto p) 
      {
	return regex_search(p->get_info()->nombre, reg);
      });
  }

  bool reachable(Node * src, Node * tgt) const
  {
    return test_for_path(net, src, tgt);
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

  using Dist = tuple<size_t, MetaProducto*>;

  Dist min_distance_match(const DynList<MetaProducto*> & productos,
			  const string & input_name,
			  const size_t threshold) const
  {
    auto name = only_alpha(input_name);

    Dist m = make_tuple(numeric_limits<size_t>::max(), nullptr);
    auto r = 
      productos.foldl<Dist>(m, [&name, threshold] (const Dist & d, auto ptr)
      {
	auto dist = levenshtein(only_alpha(ptr->nombre), name);
	if (dist < threshold and dist < get<0>(d))
	  return make_tuple(dist, ptr);
	return d;
      }); 
    return r;
  }

  static const long Visited = Convert_Tree;

  static DynList<MetaProducto*> *& productos_list(Node * p)
  {
    assert(NODE_COOKIE(p));
    return (DynList<MetaProducto*>*&) NODE_COUNTER(p);
  }

  void 
  upstream_best(Node * stgt,                     // Nodo de partida
		MetaProducto * tgt_producto_ptr, // productor de cod_aran
		Net & net_up,                    // red aguas arriba resultado
		Node * ttgt,                     // imagen de stgt en net_up
		size_t threshold) const          // tolerancia en string dist
  {
    assert(mapped_node<Net>(stgt) == ttgt);
    assert(stgt->get_info() == ttgt->get_info());

    const auto & tgt_productor = *stgt->get_info();

    assert(tgt_productor.productos.exists([tgt_producto_ptr] (auto p)
           { return p.second  == tgt_producto_ptr->cod_aran; }));
    
    cout <<  "Expanding " << tgt_productor.nombre << endl
	 << tgt_producto_ptr->nombre << " (" << tgt_producto_ptr->id << ")" 
	 << endl
	 << "++++++++++++++++" << endl;

    if (IS_NODE_VISITED(stgt, Visited) and 
	productos_list(ttgt)->exists([tgt_producto_ptr] (auto p)
				     { return tgt_producto_ptr == p; }))
      {
	cout << "Warning: cycle detected at node " << tgt_productor.rif
	     << " " << tgt_productor.nombre << endl;
	return;
      }
    NODE_BITS(stgt).set_bit(Visited, true);
    productos_list(ttgt)->append(tgt_producto_ptr);

    DynMapTree<Uid, LinearFactor> inputs;
    tgt_producto_ptr->comb.for_each([&inputs] (auto fact)
				    {
				      inputs.insert(get<0>(fact), fact);
				    });
    
    const auto & producto_id = tgt_producto_ptr->id;
        // Recorrer arcos de src y visitar aquellos relacionados a inputs
    for (auto it = net.get_in_it(stgt); 
	 it.has_curr() and not inputs.is_empty(); it.next())
      {
	auto a = it.get_curr();
	const auto & info = a->get_info();
	if (producto_id != info.producto_id)
	  continue;

	const Uid & insumo_id = info.insumo_id;
	auto p = inputs.search(insumo_id);
	if (p == nullptr) // arco a se corresponde con algún input? 
	  continue;

	inputs.remove(insumo_id);

	auto ssrc = net.get_src_node(a);
	auto productor_ptr = ssrc->get_info();
	
	Node * tsrc = nullptr;
	if (NODE_COOKIE(ssrc))
	  tsrc = mapped_node<Net>(ssrc);
	else
	  {
	    tsrc = net_up.insert_node(ssrc->get_info());
	    Net::map_nodes(ssrc, tsrc);
	    productos_list(tsrc) = new DynList<MetaProducto*>;
	  }
	assert(productos_list(tsrc));
	net_up.insert_arc(tsrc, ttgt, a->get_info());

	const string & cod_aran = info.cod_aran;
	const auto & input_name = tabla_insumos(insumo_id)->nombre;
 	auto productos = inter_productor_cod_aran(productor_ptr, cod_aran);
	auto dist = min_distance_match(productos, input_name, threshold);

	auto prod = get<1>(dist);
	if (prod != nullptr)
	  upstream_best(ssrc, get<1>(dist), net_up, tsrc, threshold);
      }
  }

public:

  Net upstream_best(Node * ssrc, MetaProducto * producto_ptr, 
		    size_t threshold) const
  {
    net.reset_nodes();
    Net ret;

    auto tsrc = ret.insert_node(ssrc->get_info());
    Net::map_nodes(ssrc, tsrc);
    productos_list(tsrc) = new DynList<MetaProducto*>;
    productos_list(tsrc)->append(producto_ptr);

    upstream_best(ssrc, producto_ptr, ret, tsrc, threshold);

    ret.for_each_node([] (auto p) 
		      { 
			if (productos_list(p)) 
			  delete productos_list(p);
		      });

    return ret;
  }
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
