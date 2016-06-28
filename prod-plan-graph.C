# include <prod-plan-graph.H>

void ProdPlanGraph::build_pp(MetaProducto * product, double quantity,
			     size_t max_threshold)
{
  struct CmpNode
  {
    bool operator () (Node * p, Node * q) const
    {
      return p->get_info().product->id < q->get_info().product->id;
    }
  };

  struct CmpArcs
  {
    bool operator () (Arc * a1, Arc * a2) const
    {
      if (a1->src_node < a2->src_node)
	return true;
      
      return not (a2->src_node < a1->src_node) and a1->tgt_node < a2->tgt_node;
    }
  };
  
  DynSetTreap<Node *, CmpNode> node_set;
  DynSetTreap<Arc *, CmpArcs> arc_set;

  DynListQueue<PPGraph::Node *> queue;
  
  Node * node = insert_node(NodeInfo(product, quantity));
  node->get_info().is_in_queue = true;

  node_set.insert(node);
  queue.put(node);

  while (not queue.is_empty())
    {
      Node * p = queue.get();
      
      p->get_info().is_in_queue = false;

      cout << "Computing plan for " << p->get_info().product->nombre
	   << ": " << p->get_info().quantity << endl;

      p->get_info().product->comb.for_each([&] (auto factor) {

	  Uid input_id = get<0>(factor);
	  string cod_aran = get<1>(factor);
	  double reqq = get<2>(factor);
	  Uid arc_id = get<3>(factor);

	  if (reqq == 0)
	    return;

	  // Looking for arc in net
	  Net::Arc * arc_in_net = map->get_arc_by_id(arc_id);
	  
	  if (arc_in_net == nullptr)
	    {
	      /* FIXME: Revisar porqué hay arcos no existentes. Creo que no
		 debería ocurrir */
	      stringstream s;
	      cout << "Arc with id " << arc_id << " does not exist. "
		   << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	      throw domain_error(s.str());
	    }

	  // Obtengo el productor que provee el insumo
	  Productor * producer = map->net.get_src_node(arc_in_net)->get_info();

	  assert(producer != nullptr);

	  MetaInsumo * input = map->tabla_insumos(input_id);

	  if (input == nullptr)
	    {
	      stringstream s;
	      cout << "Input with id " << input_id << " does not exist. "
		   << "THIS IS PROBABLY A BUG. PLEASE REPORT IT!";
	      throw domain_error(s.str());
	    }

	  /* Extraigo los productos con el código arancelario igual al insumo
	     con los nombres que más se parezcan según el umbral */
	  DynList<pair<Uid, string>> filtered_list =
	    producer->productos.filter([&](auto item) {

		// Si no es el cod aranc, entonces no va.
		if (item.second != cod_aran)
		  return false;
		
		// Busco el producto con id dado
		auto pr = map->tabla_productos(item.first);
		
		assert(pr != nullptr);
		
		size_t d = levenshtein(pr->nombre, input->nombre);
		
		return d <= max_threshold;
	      });

	  // Si no hay productos, salgo.
	  if (filtered_list.is_empty())
	    return;

	  // Busco el de distancia menor
	  MetaProducto * prod = nullptr;
	  size_t min_dist = max_threshold + 1;

	  for (auto item : filtered_list)
	    {
	      auto pr = map->tabla_productos(item.first);

	      size_t d = levenshtein(pr->nombre, input->nombre);

	      if (d < min_dist)
		{
		  min_dist = d;
		  prod = pr;
		}
	    }

	  assert(prod != nullptr);
	  		
	  // I got the product. Then compute quantity, insert node and connect
	  double quan = reqq * p->get_info().quantity;

	  // Verificar si existe un nodo en el grafo con prod
	  Node test(NodeInfo(prod, 0));
	  Node ** result = node_set.search(&test);

	  Node * q = nullptr;

	  if (result == nullptr)
	    {
	      q = insert_node(NodeInfo(prod, quan));
	      node_set.insert(q);
	      queue.put(q);
	    }
	  else
	    {
	      q = *result;
	      q->get_info().quantity += quan;

	      if (not q->get_info().is_in_queue)
		queue.put(q);
	    }

	  q->get_info().is_in_queue = true;

	  // Ver si existe arco entre q y p
	  Arc arc_test(q, p);
	  Arc ** res = arc_set.search(&arc_test);
	  Arc * a = nullptr;

	  if (res == nullptr)
	    {
	      a = insert_arc(q, p, quan);
	      arc_set.insert(a);
	    }
	  else
	    {
	      a = *res;
	      a->get_info() = quan;
	    }
	});
    }
}
