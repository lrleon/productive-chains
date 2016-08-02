
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

  ValueArg<string> socios("s", "socios", "nombre archivo socios", false,
			  "socios.txt",  "nombre archivo socios");
  cmd.add(socios);

  ValueArg<string> grafo("g", "archivo-grafo", "nombre archivo grafo", false,
			 "grafo.txt", "nombre archivo grafo");
  cmd.add(grafo);

  ValueArg<string> insumos("i", "archivo-insumos", "nombre archivo insumos",
			   false, "insumos.txt", "nombre archivo insumos");
  cmd.add(insumos);

  ValueArg<string> plantas("f", "archivo-plantas", "nombre archivo plantas",
			   false, "plantas.txt", "nombre archivo plantas");
  cmd.add(plantas);

  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();

  ofstream out(meta.getValue());
  if (out.fail())
    throw domain_error((char*)fmt("No puedo abrir %s", meta.getValue().c_str()));

  ifstream gstream(grafo.getValue());
  GrafoSigesic g = cargar_grafo(gstream);

  ifstream productores_stream(productores.getValue());
  if (productores_stream.fail())
    throw domain_error((char*)fmt("No puedo abrir %s", 
			   productores.getValue().c_str()));
  TablaProductores tabla_productores(productores_stream);
  tabla_productores.save(out);

  ifstream productos_stream(productos.getValue());
  if (productos_stream.fail())
    throw domain_error((char*)fmt("No puedo abrir %s",
				  productos.getValue().c_str()));
  TablaMetaProductos tabla_productos(productos_stream);
  tabla_productos.save(out);

  ifstream insumos_stream(insumos.getValue());
  if (insumos_stream.fail())
    throw domain_error((char*)fmt("No puedo abrir %s",
				  insumos.getValue().c_str()));
  TablaMetaInsumos tabla_insumos(insumos_stream);
  tabla_insumos.save(out);

  ifstream socios_stream(socios.getValue());
  if (socios_stream.fail())
    throw domain_error((char*)fmt("No puedo abrir %s",
				  socios.getValue().c_str()));
  TablaMetaSocios tabla_socios(socios_stream);
  tabla_socios.save(out);

  ifstream plantas_stream(plantas.getValue());
  if (plantas_stream.fail())
    throw domain_error((char*)fmt("No puedo abrir %s",
				  plantas.getValue().c_str()));
  TablaPlantas tabla_plantas(plantas_stream);
  tabla_plantas.save(out);

  Net net = build_net(g, tabla_productores);
  save_net(net, tabla_productores, out);
}


int main(int argc, char *argv[])
{
  process_comand_line(argc, argv);
  return 0;
}
