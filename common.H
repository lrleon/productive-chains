
# include <autosprintf.h>

# include <fstream>
# include <parse-csv.H>

# include <tpl_dynArray.H>
# include <ahSort.H>

# include <csvparser.h>

using Uid = unsigned long;

using fmt = gnu::autosprintf;

struct Planta
{
  Uid id;
  string rif;
  string nombre;

  Planta(Uid i = -1) noexcept : id(i) {}

  Planta(Uid i, const string & r, const string & str) noexcept
    : id(i), rif(r), nombre(str) {}

  string to_string() const
  {
    return ::to_string(id) + " " + rif + " " + nombre;
  }
  
  friend ostream & operator << (ostream & out, const Planta & p)
  {
    return out << p.to_string();
  }
};

struct TablaPlantas
{
  DynArray<Planta> table;

  TablaPlantas(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	table.emplace(atol(row(0).c_str()), row(1), row(2));
      }

    in_place_sort(table, [] (auto p1, auto p2) { return p1.id < p2.id; });
  }
  
  const Planta & operator () (const Uid id) const
  {
    auto p = bsearch(table, Planta(id), 
		     [] (auto p1, auto p2) { return p1.id < p2.id; });
    if (not p.first)
      throw std::domain_error(fmt("Planta id %ld no encintrado", id));
    
    return p.second;
  }
};


struct Producto
{
  Uid id;
  string nombre;
  Uid planta_id;
  string cod_aran;

  Producto(Uid i = -1) noexcept : id(i) {}

  Producto(Uid i, const string & nom, const string & cod, Uid pla_id) noexcept
  : id(i), nombre(nom), planta_id(pla_id), cod_aran(cod) {}

  Producto(Uid pid, const string & cod) : planta_id(pid), cod_aran(cod) {}

  string to_string() const
  {
    return ::to_string(id) + " " + nombre + " " + cod_aran + " " + 
      ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Producto & p)
  {
    return out << p.to_string();
  }

  // Orden según planta_d,cod_aran
  struct Cmp_Cod_Planta
  {
    bool operator () (const Producto & p1, const Producto & p2) const noexcept
    {
      if (p1.planta_id == p2.planta_id)
	return p1.cod_aran < p2.cod_aran;
      return p1.planta_id < p2.planta_id;
    }
  };
}; 

struct TablaP
{
  DynArray<Producto> table;

  TablaP(const char * name)
  {    
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	table.emplace(atol(row(0).c_str()), row(1), row(3), 
		      atol(row(4).c_str()));
      }

    in_place_sort(table, [] (auto p1, auto p2) { return p1.id < p2.id; });
  }

  const Producto & operator () (Uid id) const
  {
    auto p = bsearch(table, Producto(id), 
		     [] (auto p1, auto p2) { return p1.id < p2.id; });
    if (not p.first)
      throw std::domain_error("Producto id %ld no encontrado");

    return p.second;
  }
};

struct TablaProductos : public TablaP
{
  DynArray<Producto*> index_by_planta;
  Producto::Cmp_Cod_Planta cmp;
  
  TablaProductos(const char * name) 
    : TablaP(name), cmp(Producto::Cmp_Cod_Planta())
  {
    index_by_planta = build_index_ptr(table, cmp);
  }

  DynList<Producto*> plantas(Uid planta_id, const string cod_aran) const
  {
    Producto tmp(planta_id, cod_aran);
    return bsearch_dup(index_by_planta, tmp, cmp);
      // [this] (auto p1, auto p2) { return cmp(*p1, *p2); } );	
  }
};

using TablaInsumos = TablaP;

struct ProveedorInsumo
{
  Uid id;
  Uid proveedor_id;
  Uid insumo_id;
  double monto;
  double cantidad;

  ProveedorInsumo(Uid i = -1) : id(i) {}

  ProveedorInsumo(Uid i, Uid pid, Uid iid, double m, double cant)
    : id(i), proveedor_id(pid), insumo_id(iid), monto(m), cantidad(cant) {}

  string to_string() const
  {
    return ::to_string(id) + " " + ::to_string(proveedor_id) + " " + 
      ::to_string(insumo_id) + " " + ::to_string(monto) + " " + 
      ::to_string(cantidad);
  }

  friend ostream & operator << (ostream & out, const ProveedorInsumo & p)
  {
    return out << p.to_string();
  }
};

struct TablaProveedorInsumo
{
  DynArray<ProveedorInsumo> table;

  TablaProveedorInsumo(const char * name, long anho)
  {
    CsvParser * parser = CsvParser_new(name, ",", true);

    auto header = CsvParser_getHeader(parser);
    if (header == nullptr)
      throw std::domain_error("failed to parse header (file name valid?)");
    CsvParser_destroy_row(header);

    CsvRow *row;
    while ((row = CsvParser_getRow(parser))) 
      {
	char **rowFields = CsvParser_getFields(row);
	long year = atol(rowFields[4]);
	if (year != anho)
	  continue;
	table.emplace(atol(rowFields[0]), atol(rowFields[1]), 
		      atol(rowFields[2]), atof(rowFields[3]), 
		      atof(rowFields[5]));
      CsvParser_destroy_row(row);
    }

  CsvParser_destroy(parser);

  in_place_sort(table, 
		[] (auto p1, auto p2) { return p1.insumo_id < p2.insumo_id; });
  }

  DynList<ProveedorInsumo*> operator() (Uid insumo_id) const
  {
    ProveedorInsumo p; p.insumo_id = insumo_id;
    auto ret = bsearch_dup(table, p, [] (auto p1, auto p2)
			   { return p1.insumo_id < p2.insumo_id; });
    if (ret.is_empty())
      throw std::domain_error(fmt("Insumo id %ld no encontrado", insumo_id));

    return ret;
  }
};

struct Proveedor
{
  Uid id;
  string nombre;
  string rif;
  Uid planta_id;

  Proveedor(Uid i = -1) : id(i) {}

  Proveedor(Uid i, const char * nom, const char * r, Uid pid)
    : id(i), nombre(nom), rif(r), planta_id(pid) {}

  string to_string() const
  {
    return ::to_string(id) + " " + 
      nombre + " " + 
      rif + 
      ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Proveedor & p)
  {
    return out << p.to_string();
  }
};

struct TablaProveedores
{
  DynArray<Proveedor> table;

  TablaProveedores(const char * name)
  {
    CsvParser * parser = CsvParser_new(name, ",", true);

    auto header = CsvParser_getHeader(parser);
    if (header == nullptr)
      throw std::domain_error("failed to parse header (file name valid?)");
    CsvParser_destroy_row(header);

    CsvRow *row;
    while ((row = CsvParser_getRow(parser))) 
      {
	char **rowFields = CsvParser_getFields(row);
	table.emplace(atol(rowFields[0]), rowFields[1], rowFields[2], 
		      atol(rowFields[6]));
	CsvParser_destroy_row(row);
      }

    CsvParser_destroy(parser);

    in_place_sort(table, [] (auto p1, auto p2) { return p1.id < p2.id; });
  }

  const Proveedor & operator () (Uid id) const
  {
    auto p = bsearch(table, Proveedor(id), 
		     [] (auto p1, auto p2) { return p1.id < p2.id; });
    if (not p.first)
      throw std::domain_error(fmt("Proveedor id %ld no encontrado", id));
    return p.second;
  }
};


