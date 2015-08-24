
# ifndef NET_H
# define NET_H


# include <regex>
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



extern Net build_net(const GrafoSigesic & g, 
		     const TablaProductores & tabla_productores);

extern void save_net(const Net & net, 
		     const TablaProductores & tabla_productores,
		     ostream & out);

struct MetaMapa
{
  mutable TablaProductores tabla_productores;
  mutable TablaMetaProductos tabla_productos;
  mutable TablaMetaSocios tabla_socios;
  mutable Net net;
  // Relation para conectividad

  using Node = Net::Node;
  using Arc = Net::Arc;

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
    if (::verbose)
      cout << "done!" << endl
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

  Node * search_node(const Productor & prod) const noexcept
  {
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
    return Test_For_Path<Net>()(net, src, tgt);
  }
  // {
  //   DynList<Node*> ret;

  //   return ret;
  // }

  // Net build_subcomponent(

  // DynList<Node*> nodos_insumo
};


# endif 
