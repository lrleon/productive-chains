
# include <iostream>
# include <tpl_dynArray.H>
# include <ahSort.H>
# include <csvparser.h>

using Uid = unsigned long;

struct Planta
{
  Uid id;
  string rif;
  string nombre;

  Planta() {}
  Planta(Uid i, const char * r, const char * str) 
    : id(i), rif(r), nombre(str) {}
};


DynArray<Planta> load_plantas(const char * name, const char *delimiter)
{
  DynArray<Planta> ret;

  CsvParser * parser = CsvParser_new(name, delimiter, true);

  auto header = CsvParser_getHeader(parser);
  if (header == nullptr)
    throw std::domain_error("failed to parse header (file name valid?)");
  CsvParser_destroy_row(header);

  CsvRow *row;
  while ((row = CsvParser_getRow(parser))) 
    {
      char **rowFields = CsvParser_getFields(row);
      ret.emplace(atol(rowFields[0]), rowFields[1], rowFields[2]);
      CsvParser_destroy_row(row);
    }

  CsvParser_destroy(parser);

  return sort(ret, [] (auto p1, auto p2) { return p1.id < p2.id; });
}


struct Producto
{
  Uid id;
  string nombre;
  string cod_aran;
  Uid planta_id;

  Producto() {}

  Producto(Uid i, const char * nom, const char * cod, Uid pla_id)
    : id(i), nombre(nom), cod_aran(cod), planta_id(pla_id) {}

  string to_string() const
  {
    return ::to_string(id) + " " + nombre + " " + cod_aran + " " + 
      ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Producto & p)
  {
    return out << p.to_string();
  }
};


DynArray<Producto> load_productos(const char * name, const char *delimiter)
{
  DynArray<Producto> ret;

  CsvParser * parser = CsvParser_new(name, delimiter, true);

  auto header = CsvParser_getHeader(parser);
  if (header == nullptr)
    throw std::domain_error("failed to parse header (file name valid?)");
  CsvParser_destroy_row(header);

  CsvRow *row;
  while ((row = CsvParser_getRow(parser))) 
    {
      char **rowFields = CsvParser_getFields(row);
      ret.emplace(atol(rowFields[0]), rowFields[1], 
		  rowFields[3], atol(rowFields[4]));
      CsvParser_destroy_row(row);
    }

  CsvParser_destroy(parser);

  return sort(ret, [] (auto p1, auto p2) { return p1.id < p2.id; });
}

struct ProveedorInsumo
{
  Uid id;
  Uid proveedor_id;
  Uid insumo_id;
  double monto;
  double cantidad;

  ProveedorInsumo() {}

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

DynArray<ProveedorInsumo> 
load_ProveedorInsumo(const char * name, const char *delimiter)
{
  DynArray<ProveedorInsumo> ret;

  CsvParser * parser = CsvParser_new(name, delimiter, true);

  auto header = CsvParser_getHeader(parser);
  if (header == nullptr)
    throw std::domain_error("failed to parse header (file name valid?)");
  CsvParser_destroy_row(header);

  CsvRow *row;
  while ((row = CsvParser_getRow(parser))) 
    {
      char **rowFields = CsvParser_getFields(row);
      ret.emplace(atol(rowFields[0]), atol(rowFields[1]), atol(rowFields[2]),
		  atof(rowFields[3]), atof(rowFields[5]));
      CsvParser_destroy_row(row);
    }

  CsvParser_destroy(parser);

  return sort(ret, [] (auto p1, auto p2) { return p1.id < p2.id; });
}
