
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

  void operator () (ifstream & in, Net&, Net::Node * p) const
  {
    size_t idx;
    in >> idx;
    p->get_info() = &tabla_productores_ptr->access(idx);
  }
};



Net build_net(const GrafoSigesic & g,
	      const TablaProductores & tabla_productores)
{
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

  return net;
}


void save_net(const Net & net, const TablaProductores & tabla_productores,
	      ostream & out)
{
  SaveNetNode s(tabla_productores);
  IO_Graph<Net, LoadNetNode, SaveNetNode> io(const_cast<Net&>(net));
  io.set_store_node(s);
  io.save_in_text_mode(out);
}
