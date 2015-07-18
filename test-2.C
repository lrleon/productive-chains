
# include <tclap/CmdLine.h>

# include <tpl_dynArray.H>

# include <grafo.H>


using namespace TCLAP;


int main()
{
  Mapa mapa(2012, "unidad_economica.csv",
	    "unidadecon_subunidad_economica.csv",
	    "produccion_producto.csv", "produccion_insumo.csv", 
	    "cmproveedores_proveedorinsumo.csv", "cmproveedores_proveedor.csv",
	    "produccion_producto_t_insumo.csv");

  mapa.save(cout);
}
