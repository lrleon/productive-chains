# include <demand-satisfaction.H>

Net::Arc * DemandSatisfaction::search_net_arc(ArcsIndex & net_arcs, Uid id)
{
  Net::Arc test;
  test.get_info().arco_id = id;

  Net::Arc ** result = net_arcs.search(&test);

  if (result == nullptr)
    return nullptr;

  return *result;
}

double DemandSatisfaction::infer_increase(const Tuple & t)
{
  double q = get<2>(t);
  Planta * plant = map->tabla_plantas(get<1>(t));
  double cap = plant->cap;

  // Si está en cap 0, no tiene sentido calcular
  if (cap == 0.0)
    return 0.0;

  // Calculo el 100% posible de producción
  double possible_prod = 100.0 * q / cap;

  // Retorno lo que podría aumentar
  return possible_prod - q;
}

DemandSatisfaction::Result
DemandSatisfaction::simple_aproach(MetaProducto * product, double quantity,
				   size_t max_threshold)
{
  Result r;
  get<0>(r) = true;

  ArcsIndex arcs;
  
  Net & net = map->net;

  if (::verbose)
    cout << "Indexing arcs...\n";
  for (Net::Arc_Iterator it(net); it.has_curr(); it.next())
    arcs.insert(it.get_curr());
  if (::verbose)
    cout << "Done!\n";

  if (::verbose)
    cout << "Computing demand satisfaction...\n";
  
  DynListQueue<QTuple> queue;

  double desired_quantity = quantity;

  product->productores.for_each([&](const string & rif) {

	// Busco el productor con el rif actual.
	Productor * producer = map->tabla_productores(rif);

	// Busco en el productor la tupla del producto
	auto t = producer->productos[product->id];

	// Con la tupla, infiero la cantidad de producción que se puede aumentar
	double possible_increase = infer_increase(t);
	double increase = 0.0;

	if (possible_increase > desired_quantity)
	  {
	    increase = desired_quantity;
	    desired_quantity = 0.0;
	  }
	else
	  {
	    increase = possible_increase;
	    desired_quantity -= increase;
	  }

	if (increase > 0)
	  queue.put(make_tuple(product, increase));
      });

  if (desired_quantity > 0) // No se satisface la demanda aguas abajo
    {
      get<0>(r) = false;
      get<1>(r).append(make_pair(product, quantity));
      if (::verbose)
	cout << "Done!\n";
      return r;
    }
  
  while (not queue.is_empty())
    {
      // Proceso de analizar hacia atrás
      // Extraigo de la cola
      auto qt = queue.get();
      double req_quantity = get<1>(qt);

      // Veo la dependencia del producto para calcular cuánto se requiere.
      get<0>(qt)->comb.for_each([&](auto factor) {
	  Uid input_id = get<0>(factor);
	  string cod_aran = get<1>(factor);
	  double reqq = get<2>(factor);
	  Uid arc_id = get<3>(factor);
	  
	  if (reqq == 0)
	    return;
	  
	  MetaInsumo * input = map->tabla_insumos(input_id);
	  double quan = reqq * req_quantity;
	  
	  assert(input != nullptr);

	  Net::Arc * arc_in_net = search_net_arc(arcs, arc_id);

	  // Si no existe arco, agrego déficit en los insumos y salgo
	  if (arc_in_net == nullptr) // Creo nodo insumo y no encolo
	    {
	      get<0>(r) = false;
	      get<2>(r).append(make_pair(input, quan));
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

	  // Si no hay productos, inserto en insumos y salgo.
	  if (filtered_list.is_empty())
	    {
	      get<0>(r) = false;
	      get<2>(r).append(make_pair(input, quan));
	      return;
	    }

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

	  // Busco en el productor la tupla correspondiente a prod
	  auto t = producer->productos[prod->id];

	  double possible_increase = infer_increase(t);
	  double increase = 0.0;
	  
	  if (possible_increase > desired_quantity)
	  {
	    increase = quan;
	    quan = 0.0;
	  }
	  else
	    {
	      increase = possible_increase;
	      quan -= increase;
	    }
	  
	  if (quan == 0)
	    queue.put(make_tuple(prod, increase));
	  else
	    {
	      get<0>(r) = false;
	      get<1>(r).append(make_pair(prod, quan));
	    }
	  
	});
    }

  if (::verbose)
    cout << "Done!\n";
  
  return r;
}
