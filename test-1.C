
# include <tclap/CmdLine.h>

# include <tpl_dynArray.H>

# include <common.H>


using namespace TCLAP;


int main()
{
  TablaUnidadesEconomicas tabla_unidades = "unidad_economica.csv";
  tabla_unidades.for_each([] (auto u) { cout << u << endl; });

  TablaPlantas tabla_plantas = "unidadecon_subunidad_economica.csv";

  assert(tabla_plantas.all([&tabla_unidades] (const auto & planta)
    {
      return tabla_unidades(planta.rif) != nullptr; 
    }));
  
  tabla_plantas.for_each([] (auto p) { cout << p << endl; });

  // TablaProductos tabla_productos = "produccion_producto.csv";
  // tabla_productos.for_each([] (auto p) { cout << p << endl; });

  // tabla_productos.index.for_each([] (auto p)
  //   {
  //     cout << p->planta_id << " " << p->cod_aran << " " << p->id 
  // 	   << " " << p->nombre << endl;
  //   });


  // TablaInsumos tabla_insumos = "produccion_insumo.csv";
  // tabla_insumos.for_each([] (auto p) { cout << p << endl; });

  // TablaProveedorInsumo 
  //   tabla_proveedor_insumo = { "cmproveedores_proveedorinsumo.csv", 2013 };
  // tabla_proveedor_insumo.for_each([] (auto p) { cout << p << endl; });

  // TablaP tabla = "cmproveedores_proveedorinsumo.csv"; 
  
  // tabla_insumos.for_each([&tabla_proveedor_insumo] (auto i)
  //   {
  //     cout << "Insumo " << i << endl;
  //     auto proveedores = tabla_proveedor_insumo(i.id);
  //     proveedores.for_each([] (auto p) { cout << "    " << *p << endl; });
  //   });

  // TablaProveedores tabla_proveedores = "cmproveedores_proveedor.csv";
  // tabla_proveedores.for_each([] (auto p) { cout << p << endl; });

  // // for (auto it = tabla_proveedores.table.get_it(); it.has_curr(); it.next())
  // //   cout << it.get_curr() << endl;

  // auto l = table_proveedor_insumo(153813);
  // l.for_each([] (auto p) { cout << *p << endl; });
  // cout << "++++++++++++++++" << endl;

  // auto p = l.get_first();
  
  // cout << "Este es el primer proveedor-insumo de la lista " << *p << endl
  //      << "Buscar proveedor_id = " << p->proveedor_id << endl;

  // cout << "Buscando" << endl;

  // const Proveedor & prov = tabla_proveedores(p->proveedor_id);

  // cout << "Encontrado " <<  prov <<
  //   endl;

  // cout << tabla_proveedores(proveedor->proveedor_id) << endl;

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

  // Mapa mapa(2012, "unidadecon_subunidad_economica.csv",
  // 	    "produccion_producto.csv", "produccion_insumo.csv",
  // 	    "cmproveedores_proveedorinsumo.csv", "cmproveedores_proveedor.csv",
  // 	    "produccion_producto_t_insumo.csv");
}
