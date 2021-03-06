
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
			 "nombre de archivo donde se escribir� el grafo");
  cmd.add(gname);

  ValueArg<string>
    Pname("P", "archivo-productores", "nombre archivo productores", false,
	  "productores.txt", 
	  "nombre de archivo d�nde se escribir�n los productores");
  cmd.add(Pname);
  
  ValueArg<string> 
    pname("p", "archivo-productos", "nombre archivo productos", false, 
	  "productos.txt", "nombre d�nde se escribir�n los productos");
  cmd.add(pname);

  ValueArg<string>
    sname("A", "archivo-socios", "nombre archivo accionistas", false,
	  "socios.txt", "nombre de archivo donde se escirben los socios");
  cmd.add(sname);

  ValueArg<string> 
    iname("i", "archivo-input", "nombre archivo insumos", false,
	  "insumos.txt", "nombre de archivo de insumos");
  cmd.add(iname);

  ValueArg<string> 
    fname("F", "archivo-plantas", "nombre archivo plantas", false,
	  "plantas.txt", "nombre de archivo de plantas");
  cmd.add(fname);

       /****************** Archivos de entrada ****************/
  ValueArg<string>   // nom_unidades
    unidad_economica("u", "unidad-economica", "nombre archivo unidad economica",
		     false, "unidad_economica.csv",
		     "nombre archivo unidad economica");
  cmd.add(unidad_economica);

  ValueArg<string> // nombre archivo socios
    socios("S", "socios", "nombre archivo socios", false, 
	   "unidadecon_socio.csv", "nombre archivo de socios");
  cmd.add(socios);

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
    produccion("r", "archivo-produccion", "nombre archivo producci�n", false,
	       "produccion_producto.csv", 
	       "nombre archivo producci�n de productos");
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
	  "nombre archivo de arcos (relaci�n insumo/producto");
  cmd.add(arcos);

  ValueArg<long> anho("y", "anho", "a�o de estudio", false, 2012,
		      "A�o para el cual se desea transformar la data");
  cmd.add(anho);

  ValueArg<size_t> feedback("f", "feedback", "Num de iteraciones en verbose",
			    false, 10000, 
			    "N�mero de lecturas de arcos para imprimir avance");
  cmd.add(feedback);

  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();

  verbose_cycles = feedback.getValue();

  Mapa mapa(anho.getValue(), unidad_economica.getValue().c_str(),
	    socios.getValue().c_str(), subunidad_economica.getValue().c_str(), 
	    produccion.getValue().c_str(), insumo.getValue().c_str(),
	    proveedor_insumo.getValue().c_str(), proveedor.getValue().c_str(),
	    arcos.getValue().c_str());

  {
    ofstream gout(gname.getValue());
    if (gout.fail())
      throw domain_error((char*)fmt("No puedo crear %s",
				    gname.getValue().c_str()));
    mapa.save(gout);

    ofstream log("log.txt");
    mapa.log.for_each([&log] (auto s) { log << s << endl; } );
  }

  ofstream productores_out(Pname.getValue());
  if (productores_out.fail())
    throw domain_error((char*)fmt("No puedo crear %s",
				  Pname.getValue().c_str()));
  TablaProductores tabla_productores(mapa.tabla_unidades,
				     mapa.tabla_socios,
				     mapa.tabla_proveedores,
				     mapa.tabla_productos,
				     mapa.tabla_plantas);
  tabla_productores.save(productores_out);

  ofstream productos_out(pname.getValue());
  if (productos_out.fail())
    throw domain_error((char*)fmt("No puedo crear %s",
				  pname.getValue().c_str()));
  TablaMetaProductos tabla_productos(mapa.tabla_productos, tabla_productores);
  tabla_productos.save(productos_out);

  ofstream socios_out(sname.getValue());
  if (socios_out.fail())
    throw domain_error((char*)fmt("No puedo crear %s",
				  sname.getValue().c_str()));
  mapa.tabla_socios.save(socios_out);

  ofstream insumos_out(iname.getValue());
  if (insumos_out.fail())
    throw domain_error((char*)fmt("No puedo crear %s",
				  iname.getValue().c_str()));
  TablaMetaInsumos tabla_insumos(mapa.tabla_insumos);
  tabla_insumos.save(insumos_out);

  ofstream plantas_out(fname.getValue());
  if (plantas_out.fail())
    throw domain_error((char*)fmt("No puedo crear %s",
				  fname.getValue().c_str()));
  mapa.tabla_plantas.save(plantas_out);  
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
  return 0;
}
