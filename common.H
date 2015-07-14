
# include <autosprintf.h>

# include <fstream>
# include <parse-csv.H>

# include <tpl_dynArrayHeap.H>
# include <ahSort.H>

using Uid = unsigned long;

using fmt = gnu::autosprintf;

template <class T>
struct CmpId
{
  bool operator () (const T & i1, const T & i2) const { return i1.id < i2.id; }
};

struct UnidadEconomica
{
  string rif = "No definido";
  string nombre = "Sin Nombre";

  UnidadEconomica(const string & r = "No definido", 
		  const string & nom = "Sin Nombre") : rif(r), nombre(nom) {}

  string to_string() const { return rif + " " + nombre; }

  friend ostream & operator << (ostream & out, const UnidadEconomica & u)
  {
    return out << u.to_string();
  }
};

struct TablaUnidadesEconomicas : public DynArray<UnidadEconomica>
{
  struct Cmp
  {
    bool operator () (const UnidadEconomica & u1, 
		      const UnidadEconomica & u2) const noexcept
    {
      return u1.rif < u2.rif;
    }
  };

  TablaUnidadesEconomicas(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    DynArrayHeap<UnidadEconomica, Cmp> heap;
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	heap.emplace(row(0), row(2));
      }
      
    this->reserve(heap.size());
    for (size_t i = 0; not heap.is_empty(); ++i)
      this->access(i) = move(heap.getMin());
  }
  
  UnidadEconomica * operator () (const string rif) const
  {
    return bsearch(*this, UnidadEconomica(rif), Cmp());
  }
};

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

struct TablaPlantas : public DynArray<Planta>
{
  TablaPlantas(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    DynArrayHeap<Planta, CmpId<Planta>> heap;
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	heap.emplace(atol(row(0).c_str()), row(1), row(2));
      }
      
    this->reserve(heap.size());
    for (size_t i = 0; not heap.is_empty(); ++i)
      this->access(i) = move(heap.getMin());
  }
  
  Planta * operator () (Uid id) const
  {
    return bsearch(*this, Planta(id), CmpId<Planta>());
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

  struct CmpCodAran   // Orden según planta_d,cod_aran
  {
    bool operator () (const Producto & p1, const Producto & p2) const noexcept
    {
      return p1.cod_aran < p2.cod_aran;
    }
  };
}; 

struct TablaP : public DynArray<Producto> 
{
  TablaP(const char * name)
  {    
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    DynArrayHeap<Producto, CmpId<Producto>> heap; 
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	heap.emplace(atol(row(0).c_str()), row(1), row(3), 
		     atol(row(4).c_str()));
      }

    reserve(heap.size());
    for (size_t i = 0; not heap.is_empty(); ++i)
      this->access(i) = move(heap.get());
  }

 Producto * operator () (Uid id) const
  {
    auto p = bsearch(*this, Producto(id), 
		     [] (auto p1, auto p2) { return p1.id < p2.id; });
    if (p == nullptr)
      throw std::domain_error("Producto id %ld no encontrado");

    return p;
  }
};

struct TablaProductos : public TablaP
{
  DynArray<Producto*> index;
  Producto::CmpCodAran cmp;
  
  TablaProductos(const char * name) 
    : TablaP(name), cmp(Producto::CmpCodAran())
  {
    index = build_index_ptr(*this, cmp);
  }

  DynList<Producto*> por_cod_aran(const string & cod_aran) const
  {
    Producto tmp; tmp.cod_aran = cod_aran;
    return bsearch_dup(index, tmp, cmp);
  }
};

using Insumo = Producto;
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

struct TablaProveedorInsumo : public DynArray<ProveedorInsumo> 
{
  struct Cmp
  { 
    bool operator () (const ProveedorInsumo & pi1, 
		      const ProveedorInsumo & pi2) const noexcept
    {
      return pi1.insumo_id < pi2.insumo_id;
    }
  };

  TablaProveedorInsumo(const char * name, long anho)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    DynArrayHeap<ProveedorInsumo, Cmp> heap;
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));

	long year = atol(row(4).c_str());
	if (year != anho)
	  continue;
	heap.emplace(atol(row(0).c_str()), atol(row[1].c_str()),
		     atol(row(2).c_str()), atof(row(3).c_str()), 
		     atof(row(5).c_str()));
      }

    this->reserve(heap.size());
    for (size_t i = 0; not heap.is_empty(); ++i)
      this->access(i) = move(heap.getMin());
  }

  DynList<ProveedorInsumo*> operator() (Uid insumo_id) const
  {
    ProveedorInsumo p; p.insumo_id = insumo_id;
    return bsearch_dup(*this, p, Cmp());
  } 
};

struct Proveedor
{
  Uid id;
  string nombre;
  string rif;
  Uid pais_origen_id;
  Uid pais_procedencia_id;
  Uid planta_id;
  bool nacional = true;

  Proveedor(Uid i = -1) : id(i) {}

  Proveedor(Uid i, const string & nom, const string & r, 
	    Uid origen_id, Uid procedencia_id, const string & nac, Uid pid)
    : id(i), nombre(nom), rif(r), pais_origen_id(origen_id),
      pais_procedencia_id(procedencia_id), planta_id(pid), nacional(nac == "V")
  {}

  string to_string() const
  {
    const string type = nacional ? "VEN" : "EXT";
    return type + " " + ::to_string(id) + " " + nombre + " " + rif + 
      " " + ::to_string(planta_id);
  }

  friend ostream & operator << (ostream & out, const Proveedor & p)
  {
    return out << p.to_string();
  }

  bool es_nacional() const { return nacional; }

  bool es_extranjero() const { return not es_nacional(); }
};

struct TablaProveedores : public DynArray<Proveedor>
{
  TablaProveedores(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    DynArrayHeap<Proveedor, CmpId<Proveedor>> heap;
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));

	auto nac = row(5);
	if (nac != "V" and nac != "E")
	  throw domain_error(fmt("Tabla prov: nacion inválida en fila %ld", i));
	heap.emplace(atol(row(0).c_str()), row(1), row(2), 
		     atol(row(3).c_str()), atol(row(4).c_str()), nac,
		     atol(row(6).c_str()));
      }

    this->reserve(heap.size());
    for (size_t i = 0; not heap.is_empty(); ++i)
      this->access(i) = move(heap.getMin());
  }

  Proveedor * operator () (Uid id) const
  {
    auto p = bsearch(*this, Proveedor(id), 
		     [] (auto p1, auto p2) { return p1.id < p2.id; });
    if (p == nullptr)
      throw std::domain_error(fmt("Proveedor id %ld no encontrado", id));
    return p;
  }
};


