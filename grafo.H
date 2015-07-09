
# include <tpl_sort_utils.H>
# include <tpl_agraph.H>
# include <common.H>

struct Nodo
{
  string rif;
  string nombre;
  Uid planta_id;

  Nodo() {}

  Nodo(const string & r, const string & nom, Uid pid) 
    : rif(r), nombre(nom), planta_id(pid) {}

  string to_string() const
  {
    return rif + " " + nombre + " " + ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Nodo & p)
  {
    return out << p.to_string();
  }
};


struct Arco
{
  string cod_aran;
  double cantidad;
  double coste;

  Arco() {}

  Arco(const string & cod, double cant, double c)
    : cod_aran(cod), cantidad(cant), coste(c) {}

  string to_string() const 
  {
    return cod_aran + " " + ::to_string(cantidad) + " " + ::to_string(coste);
  }

  friend ostream & operator << (ostream & out, const Arco & a)
  {
    return out << a.to_string();
  }
};


class Mapa
{
public:

  using Grafo = Array_Graph<Graph_Anode<Nodo>, Graph_Aarc<Arco>>;

private:

  Grafo g;

  long anho = -1;

  DynArray<Planta> plantas;
  DynArray<Producto> productos;
  DynArray<size_t> producto_idx;
  DynArray<Producto> insumos;
  DynArray<ProveedorInsumo> proveedores_insumo;
  DynArray<Proveedor> proveedores;

  void load_plantas(const char * name) { plantas = ::load_plantas(name, ","); }

  void load_productos(const char * name) 
  {
    productos = ::load_productos(name, ",");
  }

  void load_insumos(const char * name) 
  {
    insumos = ::load_productos(name, ",");
  }

  void load_ProveedorInsumo(const char * name, long el_anho)
  {
    anho = el_anho;
    proveedores_insumo = ::load_ProveedorInsumo(name, ",", anho);
  }

  void load_proveedores(const char * name) 
  {
    proveedores = ::load_proveedores(name, ",");
  }

  auto search_proveedor_insumo(Uid id) const
  {
    const ProveedorInsumo & pi = id;
    auto l = bsearch_dup(proveedores_insumo, pi, 
			 [] (auto p1, auto p2) 
			 { return p1.insumo_id < p2.insumo_id; });
    if (l.is_empty())
      throw std::domain_error("Insumo id not encontrado en tabla de pares"
			      "proveedor insumo");
    return l;
  }  

  static const Producto & 
  search_producto(const DynArray<Producto> & table, Uid id)
  {
    const Producto p = id;
    auto i = binary_search(table, p, 
			   [] (auto p1, auto p2) { return p1.id < p2.id; });
    const Producto & ret = table(i);
    if (ret.id != id)
      throw std::domain_error("Producto no encontrado");

    return ret;
  }

  const Producto & search_producto(Uid id) const
  {
    return search_producto(productos, id);
  }

  const Producto & search_insumo(Uid id) const
  {
    return search_producto(insumos, id);
  }

  const Planta & search_planta_producto(Uid producto_id) const
  {
    const Producto & producto = search_producto(producto_id);
    const Planta pl = producto.planta_id;
    auto i = binary_search(plantas, pl, 
			   [] (auto p1, auto p2) { return p1.id < p2.id; });
    const Planta & ret = plantas(i);
    if (ret.id != producto.planta_id)
       throw std::domain_error("Planta id no encontrada");

    return ret;
  }

  const Planta & search_planta_insumo(Uid producto_id) const
  {
    const Producto & insumo = search_insumo(producto_id);
    const ProveedorInsumo & pi = search_proveedor_insumo(insumo.id);
    const Planta pl = insumo.planta_id;
    auto i = binary_search(plantas, pl, 
			   [] (auto p1, auto p2) { return p1.id < p2.id; });
    const Planta & ret = plantas(i);
    if (ret.id != insumo.planta_id)
       throw std::domain_error("Planta id no encontrada");

    return ret;
  }

public:

  Mapa(long el_anho,
       const char * nom_plantas, 
       const char * nom_productos,
       const char * nom_insumos,
       const char * nom_prov_ins,
       const char * nom_proveedores,
       const char * /* nom_arcos */)
  {
    load_plantas(nom_plantas);
    load_productos(nom_productos);
    producto_idx = build_index(productos, Producto::Cmp_Cod_Planta());
    load_insumos(nom_insumos);
    load_ProveedorInsumo(nom_prov_ins, el_anho);
    load_proveedores(nom_proveedores);
  }

};
