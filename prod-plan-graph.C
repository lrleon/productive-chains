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

string NodeInfo::get_cmp_key() const
{
  stringstream s;
  s << get_id() << '-';

  if (plant == nullptr)
    s << "-1";
  else
    s << plant->id;
  
  return s.str();
}

const string & NodeInfo::get_name() const
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

PlantCap * search_plant(MetaProducto * product, ClassPlantSet & cps,
			size_t threshold)
{
  if (get<0>(cps).is_empty())
    return nullptr;
  
  PlantMap & plant_map = get<0>(cps);

  auto p_ptr = plant_map.search(product->cod_aran);

  if (p_ptr == nullptr)
    return nullptr;

  ProductMap & product_map = p_ptr->second;

  auto pm_ptr = product_map.find_ptr([product,threshold](auto item) {
      return levenshtein(product->nombre, item.first->nombre) <= threshold;
    });

  if (pm_ptr == nullptr)
    return nullptr;
  

  if (get<1>(*pm_ptr->second) == 0)
    return nullptr;

  return pm_ptr->second;
}

         // Created node,    set quantiy
tuple<ProdPlanGraph::Node *, double>
ProdPlanGraph::create_node_and_connect(void * good, double quantity,
				       NodeInfo::ProductType t,
				       Node * p, PPNodesIdx & nodes,
				       PPArcsIdx & arcs,
				       ClassPlantSet & plant_set,
				       double threshold)
{
  double quantity_to_set = quantity;
  
  Node test_node(NodeInfo(good, 0));

  PlantCap * plant = nullptr;

  if (t == NodeInfo::ProductType::Product)
    {
      plant = search_plant(to_product(good), plant_set, threshold);

      if (plant != nullptr)
	{
	  test_node.get_info().plant = get<0>(*plant);
	  quantity_to_set = min(quantity, get<1>(*plant));
	  get<1>(*plant) -= quantity_to_set;
	}
    }
  
  Node ** result_node = nodes.search(&test_node);
  
  Node * q = nullptr;
  
  if (result_node == nullptr)
    {
      q = insert_node(NodeInfo(good, quantity_to_set, t));

      if (plant == nullptr)
	q->get_info().plant = nullptr;
      else
	q->get_info().plant = get<0>(*plant);
      
      nodes.insert(q);
    }
  else
    {
      q = *result_node;
      q->get_info().quantity += quantity_to_set;
    }
  
  // Ver si existe arco entre q y p
  Arc test_arc(q, p);
  Arc ** result_arc = arcs.search(&test_arc);
  Arc * a = nullptr;
  
  if (result_arc == nullptr)
    {
      a = insert_arc(q, p, quantity_to_set);
      arcs.insert(a);
    }
  else
    {
      a = *result_arc;
      a->get_info() = quantity_to_set;
    }
  
  return make_tuple(q, quantity_to_set);
}

void ProdPlanGraph::build_pp(DynList<pair<MetaProducto *, double>> & list,
			     size_t max_threshold,
			     DynSetTree<Productor *, Treap> & producer_set)
{
  NetArcsIdx net_arcs;

  cout << "Indexing arcs...\n";

  for (Net::Arc_Iterator it(map->net); it.has_curr(); it.next())
    net_arcs.insert(it.get_curr());
  
  cout << "Done!\n";

  ClassPlantSet class_plant_set = classify_plants(producer_set);

  PPNodesIdx node_set;
  PPArcsIdx arc_set;

  list.for_each([&](auto p)
		{
		  build_pp(p.first, p.second, max_threshold, net_arcs,
			   class_plant_set, node_set, arc_set);
		});

  cout << "Done!\n";
}

void ProdPlanGraph::build_pp(DynList<pair<MetaProducto *, double>> & list,
			     size_t max_threshold)
{
  DynSetTree<Productor *, Treap> empty_producer_set;

  build_pp(list, max_threshold, empty_producer_set);
}

void ProdPlanGraph::build_pp(MetaProducto * product, double quantity,
			     size_t max_threshold, NetArcsIdx & net_arcs,
			     ClassPlantSet & plant_set,
			     PPNodesIdx & node_set, PPArcsIdx & arc_set)
{
  DynListQueue<PPGraph::Node *> queue;
  
  double desired_quantity = quantity;
  
  while (desired_quantity > 0.0)
    {
      double quantity_to_set = desired_quantity;
      
      PlantCap * plant = search_plant(product, plant_set, max_threshold);

      Node test_node(NodeInfo(product, 0));

      if (plant != nullptr)
	{
	  test_node.get_info().plant = get<0>(*plant);
	  quantity_to_set = min(desired_quantity, get<1>(*plant));
	  get<1>(*plant) -= quantity_to_set;
	}
      
      Node ** result_node = node_set.search(&test_node);
      
      Node * node = nullptr;
      
      if (result_node == nullptr)
	{
	  node = insert_node(NodeInfo(product, quantity_to_set));

	  if (plant == nullptr)
	    node->get_info().plant = nullptr;
	  else
	    node->get_info().plant = get<0>(*plant);
	  
	  node_set.insert(node);
	}
      else
	{
	  node = *result_node;
	  node->get_info().quantity += quantity_to_set;
	}

      if (not node->get_info().is_in_queue)
	{
	  queue.put(node);
	  node->get_info().is_in_queue = true;
	}

      desired_quantity -= quantity_to_set;
    }

  while (not queue.is_empty())
    {
      Node * p = queue.get();
      
      p->get_info().is_in_queue = false;
      
      if (::verbose)
	cout << "Computing plan for "
	     << to_product(p->get_info().product)->nombre
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
				      p, node_set, arc_set, plant_set,
				      max_threshold);
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
				      p, node_set, arc_set, plant_set,
				      max_threshold);
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

	  double desired_quantity = quan;
	  
	  while (desired_quantity > 0.0)
	    {
	      auto res = create_node_and_connect(prod, desired_quantity,
						 NodeInfo::ProductType::Product,
						 p, node_set, arc_set,
						 plant_set, max_threshold);
	  
	      if (not get<0>(res)->get_info().is_in_queue)
		{
		  queue.put(get<0>(res));
		  get<0>(res)->get_info().is_in_queue = true;
		}

	      desired_quantity -= get<1>(res);
	    }
	});
    }
  if (::verbose)
    cout << "Done!\n";
}

ClassPlantSet ProdPlanGraph::classify_plants(ProducerSet & prod_set)
{
  ClassPlantSet ret;

  if (prod_set.is_empty())
    return ret;

  if (::verbose)
    cout << "Classifying plants...\n";
  
  prod_set.for_each([&](Productor * producer) {
      
      producer->productos.for_each([&](const auto & item) {

	  Planta * plant = map->tabla_plantas(get<1>(item.second));

	  if (plant->cap == 0)
	    return;

	  double possible_prod = 100.0 * get<2>(item.second) / plant->cap;
	  double available_prod = possible_prod - get<2>(item.second);

	  auto pc = get<1>(ret).search_or_insert(make_tuple(plant,
							    available_prod));

	  MetaProducto * product = map->tabla_productos(item.first);

	  auto & product_map = get<0>(ret)[get<0>(item.second)];

	  product_map.insert(product, pc);
	});
      
    });

  if (::verbose)
    cout << "Done!\n";

  return ret;
}
