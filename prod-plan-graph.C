# include <prod-plan-graph.H>
# include <ahNow.H>

Net::Arc * ProdPlanGraph::search_net_arc(NetArcsIdx & net_arcs, Uid id)
{
  Net::Arc test;
  test.get_info().arco_id = id;

  Net::Arc ** result = net_arcs.search(&test);

  if (result == nullptr)
    return nullptr;

  return *result;
}

MetaProducto * to_product(void * p)
{
  return reinterpret_cast<MetaProducto *>(p);
}

MetaInsumo * to_input(void * p)
{
  return reinterpret_cast<MetaInsumo *>(p);
}

string NodeInfo::to_str() const
{
  if (type == ProductType::Product)
    return to_product(product)->nombre;
  else
    return to_input(product)->nombre;
}

Uid NodeInfo::get_id() const
{
  if (type == ProductType::Product)
    return to_product(product)->id;
  else
    return to_input(product)->id;
}

ProdPlanGraph::Node *
ProdPlanGraph::create_node_and_connect(void * good, double quantity,
				       NodeInfo::ProductType t,
				       Node * p, PPNodesIdx & nodes,
				       PPArcsIdx & arcs)
{
  Node test_node(NodeInfo(good, 0));
  Node ** result_node = nodes.search(&test_node);
  
  Node * q = nullptr;
  
  if (result_node == nullptr)
    {
      q = insert_node(NodeInfo(good, quantity, t));
      nodes.insert(q);
    }
  else
    {
      q = *result_node;
      q->get_info().quantity += quantity;
    }
  
  // Ver si existe arco entre q y p
  Arc test_arc(q, p);
  Arc ** result_arc = arcs.search(&test_arc);
  Arc * a = nullptr;
  
  if (result_arc == nullptr)
    {
      a = insert_arc(q, p, quantity);
      arcs.insert(a);
    }
  else
    {
      a = *result_arc;
      a->get_info() = quantity;
    }
  
  return q;
}

void ProdPlanGraph::build_pp(DynList<pair<MetaProducto *, double>> & list,
			     size_t max_threshold)
{
  NetArcsIdx net_arcs;

  cout << "Indexing arcs...\n";

  for (Net::Arc_Iterator it(map->net); it.has_curr(); it.next())
    net_arcs.insert(it.get_curr());
  
  cout << "Done!\n";

  list.for_each([&](auto p)
		{
		  build_pp(p.first, p.second, max_threshold, net_arcs);
		});

  cout << "Done!\n";
}


void ProdPlanGraph::build_pp(MetaProducto * product, double quantity,
			     size_t max_threshold, NetArcsIdx & net_arcs)
{
  PPNodesIdx node_set;
  PPArcsIdx arc_set;

  DynListQueue<PPGraph::Node *> queue;
  
  Node * node = insert_node(NodeInfo(product, quantity));
  node->get_info().is_in_queue = true;

  node_set.insert(node);
  queue.put(node);

  while (not queue.is_empty())
    {
      Node * p = queue.get();
      
      p->get_info().is_in_queue = false;

      cout << "Computing plan for " << to_product(p->get_info().product)->nombre
	   << ": " << p->get_info().quantity << endl;

      to_product(p->get_info().product)->comb.for_each([&] (auto factor) {
	  
	  Uid input_id = get<0>(factor);
	  string cod_aran = get<1>(factor);
	  double reqq = get<2>(factor);
	  Uid arc_id = get<3>(factor);

	  if (reqq == 0)
	    return;

	  MetaInsumo * input = map->tabla_insumos(input_id);
	  double quan = reqq * p->get_info().quantity;

	  assert(input != nullptr);

	  // Looking for arc in net
	  Net::Arc * arc_in_net = search_net_arc(net_arcs, arc_id);
	  
	  if (arc_in_net == nullptr) // Creo nodo insumo y no encolo
	    {
	      create_node_and_connect(input, quan, NodeInfo::ProductType::Input,
				      p, node_set, arc_set);
	      return;
	    }

	  // Existe arco, obtengo el productor que provee el insumo
	  Productor * producer = map->net.get_src_node(arc_in_net)->get_info();

	  assert(producer != nullptr);
	  
	  /* Extraigo los productos con el código arancelario igual al insumo
	     con los nombres que más se parezcan según el umbral */
	  DynList<Productor::Prod> filtered_list =
	    producer->productos.filter([&](auto item) {

		// Si no es el cod aranc, entonces no va.
		if (get<0>(item.second) != cod_aran)
		  return false;
		
		// Busco el producto con id dado
		auto pr = map->tabla_productos(item.first);
		
		assert(pr != nullptr);
		
		size_t d = levenshtein(pr->nombre, input->nombre);
		
		return d <= max_threshold;
	      });

	  // Si no hay productos, creo nodo de insumo y salgo.
	  if (filtered_list.is_empty())
	    {
	      create_node_and_connect(input, quan, NodeInfo::ProductType::Input,
				      p, node_set, arc_set);
	      return;
	    }

	  // Busco el de distancia menor
	  MetaProducto * prod = nullptr;
	  size_t min_dist = max_threshold + 1;

	  for (auto item : filtered_list)
	    {
	      auto pr = map->tabla_productos(get<0>(item));

	      size_t d = levenshtein(pr->nombre, input->nombre);

	      if (d < min_dist)
		{
		  min_dist = d;
		  prod = pr;
		}
	    }

	  assert(prod != nullptr);

	  Node * q = create_node_and_connect(prod, quan,
					     NodeInfo::ProductType::Product,
					     p, node_set, arc_set);

	  if (not q->get_info().is_in_queue)
	    {
	      queue.put(q);
	      q->get_info().is_in_queue = true;
	    }
	});
    }
}
