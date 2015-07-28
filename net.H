

# include <regex>
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



Net build_net(const GrafoSigesic & g,
	      const TablaProductores & tabla_productores)
{
  if (::verbose) 
    cout << "Building Net ... " << endl;
  Net net;

  g.for_each_node([&net, &tabla_productores] (auto p)
    {
      auto info = p->get_info();
      auto productor_ptr = tabla_productores(info.rif);
      assert(productor_ptr);
      auto pp = net.emplace_node(productor_ptr);
      map_nodes<GrafoSigesic, Net>(p, pp);
    });

  g.for_each_arc([&net, &g] (auto garc)
    {
      auto gsrc = g.get_src_node(garc);
      auto gtgt = g.get_tgt_node(garc);
      auto nsrc = mapped_node<GrafoSigesic, Net>(gsrc);
      auto ntgt = mapped_node<GrafoSigesic, Net>(gtgt);
      net.insert_arc(nsrc, ntgt, move(garc->get_info()));
    });

  if (::verbose)
    cout << "done!" << endl
	 << endl;
  
  return net;
}


void save_net(const Net & net, const TablaProductores & tabla_productores,
	      ostream & out)
{
  if (::verbose) 
    cout << "Building Net ... " << endl;
  SaveNetNode s(tabla_productores);
  IO_Graph<Net, LoadNetNode, SaveNetNode> io(const_cast<Net&>(net));
  io.set_store_node(s);
  io.save_in_text_mode(out);
  if (::verbose)
    cout << "done!" << endl
	 << endl;
}


struct MetaMapa
{
  const TablaProductores tabla_productores;
  const TablaMetaProductos tabla_productos;
  const Net net;

  using Node = Net::Node;
  using Arc = Net::Arc;

  MetaMapa(ifstream & in) 
    : tabla_productores(in), tabla_productos(in)
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
  DynList<MetaProducto*> productos_by_nom(const string & exp) const noexcept
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

  DynList<Productor*> productores_by_name(const string & exp) const noexcept
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

  DynList<Productor*> productos_by_cod_aran(const string & cod) const noexcept
  {
    return tabla_productos(cod);
  }

  DynList<Node*> search_node(const DynList<string> & rifs)
  {
    DynList<Node*> ret;

    return ret;
  }

  DynList<Node*> nodos_insumo
};
