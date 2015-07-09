
# include <tclap/CmdLine.h>
# include <tpl_dynArray.H>
# include <grafo.H>


using namespace TCLAP;


int main()
{
  // auto plantas = load_plantas("unidadecon_subunidad_economica.csv", ",");

  // plantas.for_each([] (auto p) 
  // {
  //   cout << p.id << " " << p.rif << " " << p.nombre << endl; 
  // });
 

  // auto productos = load_productos("produccion_producto.csv", ",");

  // productos.for_each([] (const auto & p) { cout << p << endl; });

  // auto proveedores_insumo = 
  //   load_ProveedorInsumo("cmproveedores_proveedorinsumo.csv", ",");

  // proveedores_insumo.for_each([] (const auto & p) { cout << p << endl; });

  // auto proveedores = load_proveedores("cmproveedores_proveedor.csv", ",");

  // proveedores.for_each([] (const auto & p) { cout << p << endl; });

  Mapa mapa(2012, "unidadecon_subunidad_economica.csv",
	    "produccion_producto.csv", "produccion_insumo.csv",
	    "cmproveedores_proveedorinsumo.csv", "cmproveedores_proveedor.csv",
	    "produccion_producto_t_insumo.csv");
}
