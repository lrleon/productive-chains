
# include <tclap/CmdLine.h>
# include <grafo.H>

bool verbose = true;

size_t verbose_cycles;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("transform-data", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", true);
  cmd.add(verbose);

	/***************** Archivos de salida ****************/
  ValueArg<string> gname("g", "archivo-grafo", "nombre archivo del grafo", 
			 false, "grafo.txt", 
			 "nombre de archivo donde se escribirá el grafo");
  cmd.add(gname);

  ValueArg<string>
    Pname("P", "archivo-productores", "nombre archivo productores", false,
	  "productores.txt", 
	  "nombre de archivo dónde se escribirán los productores");
  cmd.add(Pname);
  
  ValueArg<string> 
    pname("p", "archivo-productos", "nombre archivo productos", false, 
	  "productos.txt", "nombre dónde se escribirán los productos");
  cmd.add(pname);

       /****************** Archivos de entrada ****************/
  ValueArg<string>   // nom_unidades
    unidad_economica("u", "unidad-economica", "nombre archivo unidad economica",
		     false, "unidad_economica.csv",
		     "nombre archivo unidad economica");
  cmd.add(unidad_economica);

  ValueArg<string> // nom_plantas
    subunidad_economica("s", "subunidad-economica",
			"nombre archivo subunidad economica (plantas)", 
			false, 
			"unidadecon_subunidad_economica.csv",
			"nombre archivo subunidad economica (plantas)");
  cmd.add(subunidad_economica);

  ValueArg<string> // nom_proveedores
    proveedor("V", "archivo-proveedores", "nombre archivo proveedores", false,
	      "cmproveedores_proveedor.csv", "nombre archivo proveedores");
  cmd.add(proveedor);

  ValueArg<string> // nom_insumos
    insumo("d", "archivo-insumos", "nombre archivo insumos",
	   false, "produccion_insumo.csv", "nombre archivo proveedores insumo");
  cmd.add(insumo);

  ValueArg<string> // nom_productos
    produccion("r", "archivo-produccion", "nombre archivo producción", false,
	       "produccion_producto.csv", 
	       "nombre archivo producción de productos");
  cmd.add(produccion);

  ValueArg<string> // nom_prov_ins
    proveedor_insumo("I", "archivo-proveedor-insumo",
		     "nombre archivo proveedor insumo", false,
		     "cmproveedores_proveedorinsumo.csv",
		     "nombre archivo proveedor insumo");
  cmd.add(proveedor_insumo);

  ValueArg<string> // nom_arcos
    arcos("a", "archivo-arcos", "nombre archivo arcos", false,
	  "produccion_producto_t_insumo.csv",
	  "nombre archivo de arcos (relación insumo/producto");
  cmd.add(arcos);

  ValueArg<long> anho("A", "anho", "año de estudio", false, 2012,
		      "Año para el cual se desea transformar la data");

  ValueArg<size_t> feedback("f", "feedback", "Num de iteraciones en verbose",
			    false, 10000, 
			    "Número de lecturas de arcos para imprimir avance");
  cmd.add(feedback);

  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();

  verbose_cycles = feedback.getValue();

  Mapa mapa(anho.getValue(), unidad_economica.getValue().c_str(),
	    subunidad_economica.getValue().c_str(), 
	    produccion.getValue().c_str(), insumo.getValue().c_str(),
	    proveedor_insumo.getValue().c_str(), proveedor.getValue().c_str(),
	    arcos.getValue().c_str());

  {
    ofstream gout(gname.getValue());
    if (gout.fail())
      throw domain_error(fmt("No puedo crear %s", gname.getValue().c_str()));
    mapa.save(gout);
  }

  ofstream productores_out(Pname.getValue());
  if (productores_out.fail())
    throw domain_error(fmt("No puedo crear %s", Pname.getValue().c_str()));
  TablaProductores tabla_productores(mapa.tabla_unidades,
				     mapa.tabla_proveedores,
				     mapa.tabla_productos,
				     mapa.tabla_plantas);
  tabla_productores.save(productores_out);

  ofstream productos_out(pname.getValue());
  if (productos_out.fail())
    throw domain_error(fmt("No puedo crear %s", pname.getValue().c_str()));
  TablaMetaProductos tabla_productos(mapa.tabla_productos, tabla_productores);
  tabla_productos.save(productos_out);
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
