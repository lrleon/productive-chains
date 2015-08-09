
# include <tclap/CmdLine.h>
# include <net.H>

bool verbose = false;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("transform-data", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", true);
  cmd.add(verbose);

  SwitchArg test("t", "test", "test for consistency", true);

	/***************** Archivos de salida ****************/
  ValueArg<string> meta("m", "meta-data", "nombre archivo de archivo data", 
			false, "mapa.txt", 
			"nombre de archivo dónde se escribirán los datos");
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

  ofstream out(meta.getValue());
  if (out.fail())
    throw domain_error(fmt("No puedo abrir %s", meta.getValue().c_str()));

  ifstream gstream(grafo.getValue());
  GrafoSigesic g = cargar_grafo(gstream);

  ifstream productores_stream(productores.getValue());
  if (productores_stream.fail())
    throw domain_error(fmt("No puedo abrir %s", 
			   productores.getValue().c_str()));
  TablaProductores tabla_productores(productores_stream);

  tabla_productores.autotest();
  tabla_productores.save(out);

  ifstream productos_stream(productos.getValue());
  if (productos_stream.fail())
    throw domain_error(fmt("No puedo abrir %s", productos.getValue().c_str()));
  TablaMetaProductos tabla_productos(productos_stream);

  tabla_productos.autotest();
  tabla_productos.save(out);  
    
  Net net = build_net(g, tabla_productores);

  save_net(net, tabla_productores, out);
}


int main(int argc, char *argv[])
{
  process_comand_line(argc, argv);
}
