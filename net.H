
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
  TablaProductores   tabla_productores;
  TablaMetaProductos tabla_productos;
  TablaMetaInsumos   tabla_insumos;
  TablaMetaSocios    tabla_socios;
  Net                net;
  // DynSetTree<string> var_names; // TODO: 
  
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

  // ~MetaMapa()
  // {
  //   if (::verbose)
  //     cout << "Warning: other variables bounded to a deleted map var could"
  // 	   << "become inconsistent" << endl
  // 	   << "It is recommended to logout in order to clean the internal state"
  // 	   << endl
  // 	   << "Concretely if you have deleted a map var because you saved" 
  // 	   << endl
  // 	   << "a sub net in another map var" << endl;
  // }

  void save(ostream & out) const
  {
    tabla_productores.save(out);
    tabla_productos.save(out);
    tabla_insumos.save(out);
    tabla_socios.save(out);
    save_net(net, tabla_productores, out);
  }

  MetaMapa compress(const Net & subnet) const
  {
    MetaMapa ret;
    
    if (::verbose)
      cout << "Copying producers ... " << endl;

    {
      DynSetTree<string> socios;
      DynSetTree<Uid> productos;

      for (auto it = subnet.get_node_it(); it.has_curr(); it.next()) 
	{
	  auto & productor = *it.get_curr()->get_info();
	  ret.tabla_productores.append(productor);
	  productor.productos.for_each([&productos] (const auto & prod)
				       {
					 productos.append(prod.first);
				       });
	  productor.socios.for_each([&socios] (const auto & s)
				    {
				      socios.append(s.first);
				    });
	}

      if (::verbose)
	cout << "Building skateholder table ..." << endl
	     << endl;
      
      ret.tabla_socios.reserve(socios.size());
      size_t i = 0;
      socios.for_each([&ret, this, &i] (auto id)
        {
	  ret.tabla_socios.access(i++) = *tabla_socios(id);
	});

      if (::verbose)
	cout << "done!" << endl
	     << socios.size() << " skateholders" << endl
	     << endl
	     << "Building products table ..."<< endl;

      ret.tabla_productos.reserve(productos.size());
      i = 0;
      productos.for_each([&ret, this, &i] (auto id)
        {
	  ret.tabla_productos.access(i++) = *tabla_productos(id);
	});

      if (::verbose)
	cout << "done!" << endl
	     << productos.size() << " products" << endl
	     << endl;
    }

    if (::verbose)
      cout << "done producers copy!" << endl
	   << ret.tabla_productores.size() << " producers" << endl
	   << endl
	   << "Sorting producer before net copy copy ..." << endl;

    in_place_sort(ret.tabla_productores);

    if (::verbose)
      cout << "done!" << endl
	   << ret.tabla_productores.size() << " producers" << endl
	   << endl
	   << "Creating net nodes ..." << endl;

    DynMapTree<string, Node*> nodes; // producto_id --> Node*
    for (auto it = ret.tabla_productores.get_it(); it.has_curr(); it.next())
      {
	auto & productor = it.get_curr();
	auto ptr = ret.net.insert_node(&productor);
	nodes.insert(productor.rif, ptr);
      }
    
    if (::verbose)
      cout << "done!" << endl
	   << nodes.size() << " nodes" << endl
	   << endl
	   << "Creating arcs ..." << endl;

    DynSetTree<Uid> insumos; // los insumos cuando se hagan los arcos

    for (auto it = subnet.get_arc_it(); it.has_curr(); it.next())
      {
	auto a = it.get_curr();
	const auto & info = a->get_info();
	auto ssrc = subnet.get_src_node(a);
	auto stgt = subnet.get_tgt_node(a);
	auto tsrc = nodes.find(ssrc->get_info()->rif);
	auto ttgt = nodes.find(stgt->get_info()->rif);
	ret.net.insert_arc(tsrc, ttgt, info);
	insumos.append(info.insumo_id);
      }

    if (::verbose)
      cout << "done"
	   << ret.net.esize() << " arcs" << endl
	   << endl
	   << "Building inputs table ..." << endl;

    size_t i = 0;
    ret.tabla_insumos.reserve(insumos.size());
    insumos.for_each([&ret, this, &i] (auto id) 
      {
	ret.tabla_insumos.access(i++) = *tabla_insumos(id);
      });

    if (::verbose)
      cout << "done!" << endl
	   << insumos.size() << insumos.size() << endl
	   << endl;

    return ret;
  }

  void save(const Net & subnet, ostream & out) const
  {
    compress(subnet).save(out);    
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
