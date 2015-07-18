
# include <autosprintf.h>

# include <fstream>
# include <parse-csv.H>

# include <tpl_dynSetHash.H>
# include <tpl_olhash.H>
# include <tpl_dynArrayHeap.H>
# include <ahSort.H>

using Uid = unsigned long;

using fmt = gnu::autosprintf;

// template <typename Key, class Cmp>
// using HashType = OLhashTable<Key, Cmp>;

template <typename Key, class Cmp>
using HashType = DynSetHash<Key, Cmp>;

template <class T>
struct CmpLessId
{
  bool operator () (const T & i1, const T & i2) const { return i1.id < i2.id; }
};

template <class T>
struct CmpEqualId
{
  bool operator () (const T & i1, const T & i2) const { return i1.id == i2.id; }
};


template <class T>
size_t hash_id(const T & i) { return SuperFastHash(i.id); }

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

  struct CmpEqual
  {
    bool operator () (const UnidadEconomica & u1, 
		      const UnidadEconomica & u2) const noexcept
    {
      return u1.rif == u2.rif;
    }
  };
};


struct TablaUnidadesEconomicas 
  : public HashType<UnidadEconomica, UnidadEconomica::CmpEqual>
{
  using HashTable = HashType<UnidadEconomica, UnidadEconomica::CmpEqual>;

  static size_t hash(const UnidadEconomica & u)
  {
    return SuperFastHash(u.rif);
  }

  void init_hash(const char * name)
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
	emplace(row(0), row(2));
      }
  }

  TablaUnidadesEconomicas(const char * name) 
    : HashTable(8000, hash, UnidadEconomica::CmpEqual())
  {
    init_hash(name);
  }

  UnidadEconomica * operator () (const string rif) const
  {
    return search(UnidadEconomica(rif));
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

  using CmpEqual = CmpEqualId<Planta>;
};

struct TablaPlantas : public HashType<Planta, Planta::CmpEqual>
{
  using HashTable = HashType<Planta, Planta::CmpEqual>;
  TablaPlantas(const char * name)
    : HashTable(2171, hash_id<Planta>, Planta::CmpEqual())
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
	emplace(atol(row(0).c_str()), row(1), row(2));
      }
  }
  
  Planta * operator () (Uid id) const
  {
    return search(Planta(id));
  }
};


struct Producto
{
  Uid id;
  string nombre;
  Uid planta_id;
  string cod_aran;

  double cantidad_decl; // si producto == lo producido
  double cantidad_real; // si producto == lo vendido
  double coste;

  // combinación lineal que define al producto. Cada par es un insumo_id
  // y la cantidad requerida para fabricar una unidad del
  // producto. Sirve como huella dactilar de un producto pero también
  // para filtrar arcos en un grafo
  DynMapTree<Uid, double> comb;

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

  struct CmpLessCodAran   // Orden según planta_d,cod_aran
  {
    bool operator () (const Producto & p1, const Producto & p2) const noexcept
    {
      return p1.cod_aran < p2.cod_aran;
    }
  };

  using CmpEqual = CmpEqualId<Producto>;
}; 

struct TablaP : public HashType<Producto, Producto::CmpEqual>
{
  using HashTable = HashType<Producto, Producto::CmpEqual>;

  TablaP(const char * name)
    : HashTable(166611, hash_id<Producto>, Producto::CmpEqual())
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
	emplace(atol(row(0).c_str()), row(1), row(3), atol(row(4).c_str()));
      }
  }

  void cargar_costes(long anho, const char * costs_name) 
  {
    ifstream in(costs_name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", costs_name));

    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	if (anho != atol(row(2).c_str()))
	  continue;

	Producto * producto_ptr = search(atol(row(1).c_str()));
	assert(producto_ptr != nullptr);
	producto_ptr->cantidad_decl = atof(row(3).c_str());
	producto_ptr->cantidad_real = atof(row(4).c_str());
	producto_ptr->coste = atof(row(5).c_str());
	  
	emplace(atol(row(0).c_str()), row(1), row(3), atol(row(4).c_str()));
      }
  }

 Producto * operator () (Uid id) const
  {
    auto p = search(Producto(id));
    if (p == nullptr)
      throw std::domain_error("Producto id %ld no encontrado");

    return p;
  }
};

struct TablaProductos : public TablaP
{
  DynArray<Producto*> index;
  Producto::CmpLessCodAran cmp;
  
  TablaProductos(const char * prod_name) 
    : TablaP(prod_name), cmp(Producto::CmpLessCodAran())
  {
    index.reserve(size());
    size_t i = 0;
    mutable_for_each([this, &i] (auto & p) { index(i++) = &p; });
    in_place_sort(index, [] (auto p1, auto p2)
		  { return p1->cod_aran < p2->cod_aran; });
  }

  DynList<Producto*> por_cod_aran(const string & cod_aran) const
  {
    Producto tmp; tmp.cod_aran = cod_aran;
    return bsearch_dup(index, tmp, cmp);
  }
};


using Insumo = Producto;
using TablaInsumos = TablaP;
// struct TablaInsumos : public TablaP
// {
//   TablaInsumos(long anho, const char * prod_name, const char * costs_name) 
//     : TablaP(prod_name)
//   {
//     cargar_costes(anho, costs_name);
//   }
// };

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
  struct CmpLess
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
    DynArrayHeap<ProveedorInsumo, CmpLess> heap;
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
    return bsearch_dup(*this, p, CmpLess());
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

struct TablaProveedores : public HashType<Proveedor, CmpEqualId<Proveedor>>
{
  using HashTable = HashType<Proveedor, CmpEqualId<Proveedor>>;

  TablaProveedores(const char * name)
    : HashTable(45279, hash_id<Proveedor>, CmpEqualId<Proveedor>())
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

	auto nac = row(5);
	if (nac != "V" and nac != "E")
	  throw domain_error(fmt("Tabla prov: nacion inválida en fila %ld", i));
	emplace(atol(row(0).c_str()), row(1), row(2), atol(row(3).c_str()), 
		atol(row(4).c_str()), nac, atol(row(6).c_str()));
      }
  }

  Proveedor * operator () (Uid id) const
  {
    auto p = search(Proveedor(id));
    if (p == nullptr)
      throw std::domain_error(fmt("Proveedor id %ld no encontrado", id));
    return p;
  }
};



