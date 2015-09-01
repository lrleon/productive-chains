
# include <net.H>


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
    cout << "Saving Net ... " << endl;
  SaveNetNode s(tabla_productores);
  IO_Graph<Net, LoadNetNode, SaveNetNode> io(const_cast<Net&>(net));
  io.set_store_node(s);
  io.save_in_text_mode(out);
  if (::verbose)
    cout << "done!" << endl
	 << endl;
}
