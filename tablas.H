
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

  // combinaci�n lineal que define al producto. Cada par es un insumo_id
  // y la cantidad requerida para fabricar una unidad del
  // producto. Sirve como huella dactilar de un producto pero tambi�n
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

  struct CmpLessCodAran   // Orden seg�n planta_d,cod_aran
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
	  throw domain_error(fmt("Tabla prov: nacion inv�lida en fila %ld", i));
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

const string BLANK = " ";

inline string rif_ext(Uid origen_id, Uid procedencia_id)
{
  stringstream s;
  s << origen_id << " " << procedencia_id;
  return s.str();
}

inline pair<Uid, Uid> pais(const string & rif) 
{
  Uid origen_id, procedencia_id;
  istringstream s(rif);
  s >> origen_id >> procedencia_id;
  return make_pair(origen_id, procedencia_id);
}

struct MetaProducto;
struct MetaInsumo;

struct TablaMetaProductos;
struct TablaMetaInsumos;

struct Productor
{ //                      cod_aran
  using Prod = tuple<Uid, string>;

  bool nacional;
  string rif;
  string nombre;
  //                    cod_aran
  DynSetTree<tuple<Uid, string>> productos; // productos que fabrica y/o vende

  Productor() {}

  Productor(bool nac, const string & r, const string & nom)
    : nacional(nac), rif(r), nombre(nom) {}

  string to_string() const
  {
    return (nacional ? "VEN" : "EXT") + BLANK + rif + BLANK + nombre;
  }

  friend ostream & operator << (ostream & out, const Productor & p)
  {
    out << p.nacional << BLANK	<< p.rif << p.nombre << endl
	<< "    Productos (" << p.productos.size() << "): ";
    p.productos.for_each([&out] (auto p) 
      { 
	out << "(" << get<0>(p) << "," << get<1>(p) << "), ";
      }); 
    out << endl;
    return out;
  }

  void save(ostream & out) const 
  {
    out << nacional << BLANK << rif << endl 
	<< nombre << endl
	<< productos.size() << endl;
    productos.for_each([&out] (auto p) 
		       { 
			 out << get<0>(p) << BLANK << get<1>(p) << BLANK;
		       }); 
    out << endl;
  }

  void load(istream & in)
  {
    in >> nacional;
    if (nacional)
      in >> rif;
    else
      {
	Uid origen_id, procedencia_id;
	in >> origen_id >> procedencia_id;
	rif = rif_ext(origen_id, procedencia_id);
      }
    getline(in, nombre);

    size_t n;
    in >> n;
    for (auto i = 0; i < n; ++i)
      {
	Uid id;
	string cod_aran;
	in >> id >> cod_aran;
	productos.append(make_tuple(id, cod_aran));
      }
  }

  bool es_nacional() const { return nacional; }
  bool es_extranjero() const { return not nacional; }

  friend bool operator < (const Productor & p1, const Productor & p2) 
  {
    if (p1.es_extranjero() or p2.es_extranjero()) // alguno extranjero?
      {
	if (p1.es_nacional())
	  return true; // n1 Nacional < n2 Extranjero
	if (p2.es_nacional())
	  return false; // n1 extranjero > n2 nacional
	  
	const auto pais1 = pais(p1.rif);
	const auto pais2 = pais(p2.rif);
	if (pais1.first == pais2.first)
	  return p1.nombre < p2.nombre;
      }
    return p1.rif < p2.rif; // Ambos son nacionales
  }
};


struct MetaProducto
{
  Uid id;
  string cod_aran;
  string nombre;
  DynSetTree<string> productores; // lista de rif de propductores 
  DynMapTree<Uid, double> comb;

  MetaProducto() {}
  MetaProducto(Uid i, const string & cod, const string & nom, 
	       const DynMapTree<Uid, double> & c)
    : id(i), cod_aran(cod), nombre(nom), comb(c) {}

  void save(ostream & out) const
  {
    out << id << BLANK << cod_aran << endl
	<< nombre << endl
	<< productores.size() << endl;
    productores.for_each([&out] (auto s) { out << s << endl; }); 
    out << endl
	<< comb.size() << endl;
    comb.for_each([&out] (auto p) { out << p.first << p.second << endl; });
  }

  void load(istream & in)
  {
    in >> id >> cod_aran;
    getline(in, nombre);
    size_t n;
    in >> n;
    for (auto i = 0; i < n; ++i)
      {
	string rif;
	in >> rif;
	productores.append(rif);
      }
    in >> n;
    for (auto i = 0; i < n; ++i)
      {
	Uid id;
	double factor;
	in >> id >> factor;
	comb.insert(id, factor);
      }
  }
};

struct TablaProductores;

struct TablaMetaProductos : public DynArray<MetaProducto>
{
  typedef bool (*CmpCod)(const MetaProducto &, const MetaProducto &);
  CmpCod cmp_cod = [] (const MetaProducto & p1, const MetaProducto & p2)
  { return p1.cod_aran < p2.cod_aran; };

  typedef bool (*CmpId)(const MetaProducto &, const MetaProducto &);
  CmpId cmp_id = [] (const MetaProducto & p1, const MetaProducto & p2)
  { return p1.id < p2.id; };

  DynArray<MetaProducto*> index;

  inline void adjuntar_productores(const TablaProductores & tabla_productores);

  TablaMetaProductos(const TablaProductos & tabla_productos,
		     const TablaProductores & tabla_productores)
  {
    size_t n = tabla_productos.size();
    size_t i = 0;
    reserve(n);
    tabla_productos.for_each([this, &i] (auto & p)
      {
	access(i++) = MetaProducto(p.id, p.cod_aran, p.nombre, p.comb);
      });
    in_place_sort(*this, cmp_cod);

    index = build_index_ptr(*this, cmp_id);

    adjuntar_productores(tabla_productores);
  }

  DynList<MetaProducto*> operator () (const string & cod_aran) const noexcept
  {
    MetaProducto p; p.cod_aran = cod_aran;
    return bsearch_dup(*this, p, cmp_cod);
  }

  MetaProducto * operator () (Uid id) const noexcept
  {
    MetaProducto p; p.id = id;
    return bsearch(index, p, cmp_id);
  }

  void save(ostream & out) const
  {
    out << size() << endl;
    for_each([&out] (const auto & p) { p.save(out); });
  }

  TablaMetaProductos(istream & in)
  {
    size_t n;
    cin >> n;
    reserve(n);
    for (auto i = 0; i < n; ++i)
      access(i).load(in);
    index = build_index_ptr(*this, cmp_id);
  }
};


/*
 * Orden de construcci�n debe ser *estrictamente* as�
 *
 * 1. Contruir el grafo. De aqu� se calculan los mimebros comb por cada producto
 *
 * 2. Construir la tabla de productores
 *
 * 3. Construir la tablas de productos
 *
 */

struct TablaProductores : DynArray<Productor>
{
  void adjuntar_productos(const TablaProductos & tabla_productos,
			  const TablaPlantas & tabla_plantas)
  {
    tabla_productos.for_each([this, &tabla_plantas] (auto & prod)
      {	
	auto planta_id = prod.planta_id;
	auto planta_ptr = tabla_plantas(planta_id);
	assert(planta_ptr != nullptr);
	auto productor_ptr = (*this)(planta_ptr->rif);
	productor_ptr->productos.append(make_pair(prod.id, prod.cod_aran));
      });
  }

  TablaProductores(const TablaUnidadesEconomicas & tabla_ue,
		   const TablaProveedores &        tabla_proveedores,
		   const TablaProductos &          tabla_productos,
		   const TablaPlantas &            tabla_plantas)
  {
    DynSetTree<Productor> tree;

    // Inserta primero las unidades econ�micas
    tabla_ue.for_each([&tree] (const auto & ue)
      {
	tree.emplace(true, ue.rif, ue.nombre);
      });

    // Ahora los proveedores, algunos de los cuales pueden ser unidades
    // econ�micas, en cuyo caso ya deber�an de estar insertadas
    tabla_proveedores.for_each([&tree] (const auto & prov)
      {
	if (prov.nacional)
	  {
	    tree.emplace(true, prov.rif, prov.nombre);
	    return;
	  }
	tree.insert_dup(Productor(false, rif_ext(prov.pais_origen_id,
						 prov.pais_procedencia_id), 
				  prov.nombre));
      });
    
    reserve(tree.size()); size_t i = 0;
    tree.mutable_for_each([this, &i] (auto & prod) 
			  { access(i++) = move(prod); });
    tree.empty();
    
    adjuntar_productos(tabla_productos, tabla_plantas);
  }
  
  // Solo funciona para los nacionales 
  Productor * operator () (const string & rif) const
  {
    Productor p;
    p.rif = rif;
    return bsearch(*this, p);
  }

  // Solo funciona para los extranjeros
  DynList<Productor*> operator () (Uid origen_id, Uid procedencia_id) const
  {
    Productor p;
    p.rif = rif_ext(origen_id, procedencia_id);
    p.nacional = false;
    return bsearch_dup(*this, p);
  }

  void save(ostream & out) const
  {
    out << size() << endl;
    for_each([&out] (const auto & p) { p.save(out); });
  }

  TablaProductores(istream & in)
  {
    size_t n;
    in >> n;
    reserve(n);
    for (size_t i = 0; i < n; ++i)
      access(i).load(in);
  }
};

void TablaMetaProductos::adjuntar_productores
(const TablaProductores & tabla_productores)
{
  tabla_productores.for_each([this] (const auto & productor)
    {
      productor.productos.for_each([this, &productor] (auto p)
        {
	  auto producto_id = get<0>(p);
	  auto producto_ptr = (*this)(producto_id);
	  producto_ptr->productores.append(productor.rif);
	});
    });
}
  