
# include <tclap/CmdLine.h>
# include <grafo.H>

bool verbose = true;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("transform-data", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", true);
  cmd.add(verbose);

  ValueArg<string> gname("g", "graph-file", "nombre archivo del grafo", false,
			 "grafo.txt", 
			 "nombre de archivo donde se pondrá el grafo");
  cmd.add(gname);

  ValueArg<string> pname("p", "productor-file", "nombre archivo productores", 
			 false, "productores.txt", 
			 "nombre de archivo de productores");
  cmd.add(pname);
  
  ValueArg<string> unidad_economica("u", "unidad-economica",
				    "nombre archivo unidad economica", false,
				    "unidad_economica.csv",
				    "nombre archivo unidad economica");
  cmd.add(unidad_economica);


  cmd.parse(argc, argv);
  ::verbose = verbose.getValue();
}


int main(int argc, char *argv[])
{
  process_comand_line(argc, argv);
  Mapa mapa(2012, "unidad_economica.csv",
	    "unidadecon_subunidad_economica.csv",
	    "produccion_producto.csv", "produccion_insumo.csv", 
	    "cmproveedores_proveedorinsumo.csv", "cmproveedores_proveedor.csv",
	    "produccion_producto_t_insumo.csv");

  mapa.save(cout);
}
