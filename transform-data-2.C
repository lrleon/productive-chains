
# include <tclap/CmdLine.h>
# include <net.H>

bool verbose = true;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("transform-data", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", true);
  cmd.add(verbose);

	/***************** Archivos de salida ****************/
  ValueArg<string> meta("m", "meta-data", "nombre archivo de archivo data", 
			false, "meta-data.txt", 
			"nombre de archivo donde se escribirán los datos");
  cmd.add(meta);

       /****************** Archivos de entrada ****************/
  ValueArg<string> productores("P", "productores", "nombre archivo productores",
			       false, "productores.txt", 
			       "nombre archivo de productores");
  cmd.add(productores);

  ValueArg<string> productos("p", "productos", "nombre archivo productos", 
			false, "productos.txt", "nombre archivo metaproductos");
  cmd.add(productos);

  ValueArg<string> grafo("g", "archivo-grafo", "nombre archivo grafo", true,
			 "grafo.txt", "nombre archivo grafo");
  cmd.add(grafo);

  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();

  ifstream gstream(grafo.getValue());
  GrafoSigesic g = cargar_grafo(gstream);

  ifstream productores_stream(productores.getValue());
  if (productores_stream.fail())
    throw domain_error(fmt("No puedo abrir %s", 
			   productores.getValue().c_str()));
  TablaProductores tabla_productores(productores_stream);

  tabla_productores.autotest();
  tabla_productores.save(cout);

  Net net = build_net(g, tabla_productores);

  save_net(net, tabla_productores, cout);
  
  clear_graph(g);



  // tabla_productores.save(productores_out);

  // ofstream productos_out(pname.getValue());
  // if (productos_out.fail())
  //   throw domain_error(fmt("No puedo crear %s", pname.getValue().c_str()));
  // TablaMetaProductos tabla_productos(mapa.tabla_productos, tabla_productores);
  // tabla_productos.save(productos_out);
}


int main(int argc, char *argv[])
{
  process_comand_line(argc, argv);
  // Mapa mapa(2012, "unidad_economica.csv",
  // 	    "unidadecon_subunidad_economica.csv",
  // 	    "produccion_producto.csv", "produccion_insumo.csv", 
  // 	    "cmproveedores_proveedorinsumo.csv", "cmproveedores_proveedor.csv",
  // 	    "produccion_producto_t_insumo.csv");

  // mapa.save(cout);
}
