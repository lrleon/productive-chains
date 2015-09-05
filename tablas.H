
# include <autosprintf.h>

# include <fstream>
# include <parse-csv.H>

# include <tpl_dynSetHash.H>
# include <tpl_dynMapTree.H>
# include <tpl_olhash.H>
# include <tpl_dynArrayHeap.H>
# include <ahSort.H>

extern bool verbose;

using Uid = unsigned long;

using fmt = gnu::autosprintf;

template <typename Key, class Cmp>
using HashType = DynSetHash<Key, Cmp>;

template <class T>
struct CmpLessId
{
  bool operator () (const T & i1, const T & i2) const noexcept
  { return i1.id < i2.id; }
};

template <class T>
struct CmpEqualId
{
  bool operator () (const T & i1, const T & i2) const noexcept
  { return i1.id == i2.id; }
};


template <class T>
size_t hash_id(const T & i) { return SuperFastHash(i.id); }

struct Socio
{
  string rif;
  string nombre;
  char nacionalidad;
  
  // lista de empresas en la cual this es accionista. Son pares rif-empresa y %
  DynList<pair<string, float>> empresas; 

  Socio() {}
  Socio(const string & r, const string & nom, char nac) 
    : rif(r), nombre(nom), nacionalidad(nac) {}
  Socio(const string & r) : rif(r) {}

  struct CmpLess
  {
    bool operator ()(const Socio & s1, const Socio & s2) const noexcept
    { return s1.rif < s2.rif; }
  };

  string to_string() const
  {
    string ret(rif + " " + nacionalidad + " " +
	       nombre + "\n" + "  Accionistas: ");
    empresas.for_each([&ret] (auto p) 
		      {
			ret += p.first + " " + ::to_string(p.second);
		      });
    return ret;
  }

  friend ostream & operator << (ostream & out, const Socio & s)
  {
    return out << s.to_string() << endl;
  }

  void save(ostream & out) const 
  {
    out << rif << endl
	<< nombre << endl
	<< nacionalidad << endl
	<< empresas.size();
    empresas.for_each([&out] (auto p) 
		      { 
			out << " " << p.first << " " << p.second; 
		      });
    out << endl;
  }

  void load(istream & in)
  {
    in >> rif;
    in.ignore();
    getline(in, nombre);
    in >> nacionalidad;
    size_t n;
    in >> n;
    for (size_t i = 0; i < n; ++i)
      {
	pair<string, float> p;
	in >> p.first >> p.second;
	empresas.append(p);
      }
  }
};

struct TablaSocios : public DynSetTree<Socio, Avl_Tree, Socio::CmpLess>
{
  TablaSocios(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    if (::verbose)
      cout << "Reading socios ..." << endl;

    size_t empty_counter = 0;
    auto header = csv_read_row(in, ','); 
    const size_t n = header.size();
    for (size_t i = 0; in.good(); ++i)
      {
	auto row = csv_read_row(in, ',');
	if (row.size() != n)
	  throw domain_error(fmt("number of fields of row %d is not %d",
				 row.size(), n));
	
	char nac = stoi(row(5)) == 1 ? 'V' : 'E';
	const string & unidad_economica_rif = row(2);
	string & rif_pasaporte = row(3);
	if (rif_pasaporte == "" or rif_pasaporte.size() == 0)
	  {
	    stringstream s;
	    s << "Sin-numero-" << empty_counter++;
	    rif_pasaporte = s.str();
	  }
	auto s = search_or_insert(Socio(rif_pasaporte, row(4), nac));
	s->empresas.append(make_pair(unidad_economica_rif, stof(row(7))));
      }
    if (::verbose)
      cout << "done!" << endl
	   << endl;
  }

  Socio * operator () (const string & rif) const 
  {
    return search(Socio(rif));
  }

  void save(ostream & out) const
  {
    if (::verbose)
      cout << "Saving socios ..." << endl;
    out << size() << endl;
    for_each([&out] (auto s) { s.save(out); });
    if (::verbose)
      cout << "done!" << endl
	   << endl;
  }
};

struct TablaMetaSocios : public DynArray<Socio>
{
  TablaMetaSocios() {}
  TablaMetaSocios(ifstream & in)
  {
    if (::verbose)
      cout << "Reading socios ..." << endl;
    size_t n;
    in >> n;
    reserve(n);
    for (size_t i = 0; i < n; ++i)
      access(i).load(in);
    if (::verbose)
      cout << "done!" << endl
	   << endl;
  }

  void save(ostream & out) const
  {
    if (::verbose)
      cout << "Saving socios ..." << endl;
    out << size() << endl;
    for_each([&out] (auto s) { s.save(out); });
    if (::verbose)
      cout << "done!" << endl
	   << endl;
  }

  Socio * operator () (const string & rif) const noexcept
  {
    return bsearch(*this, Socio(rif), Socio::CmpLess());
  }  
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

  static size_t hash(const UnidadEconomica & u) noexcept
  {
    return SuperFastHash(u.rif);
  }

  void init_hash(const char * name)
  {
    ifstream in(name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", name));

    if (::verbose)
      cout << "Reading economical unities ..." << endl;

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
    if (::verbose)
      cout << "done!" << endl
	   << endl;
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

  Planta(Uid i, const string & r, const string & str) 
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

    if (::verbose)
      cout << "Reading subeconomical unities" << endl;

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
    if (::verbose)
      cout << "Done! " << endl
	   << endl;
  }
  
  Planta * operator () (Uid id) const
  {
    return search(Planta(id));
  }
};

                 // insumo_id, cod_aran, cantidad, arco_id
using LinearFactor = tuple<Uid, string, double, Uid>;

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
  DynList<LinearFactor> comb;

  Producto(Uid i = -1) : id(i) {}

  Producto(Uid i, const string & nom, const string & cod, Uid pla_id) 
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

    if (::verbose)
      cout << "Reading " << name << endl;

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
    if (::verbose)
      cout << "Done!" << endl
	   << endl;
  }

  void cargar_costes(long anho, const char * costs_name) 
  {
    ifstream in(costs_name);
    if (in.fail()) 
      throw std::domain_error(fmt("No se pudo abrir %s", costs_name));

    if (::verbose)
      cout << "Loading " << costs_name << endl;

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
    if (::verbose)
      cout << "Done!" << endl
	   << endl;
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
    if (::verbose)
      cout << "    Building index for " << prod_name << endl;
    index.reserve(size());
    size_t i = 0;
    mutable_for_each([this, &i] (auto & p) { index(i++) = &p; });
    in_place_sort(index, [] (auto p1, auto p2)
		  { return p1->cod_aran < p2->cod_aran; });
    if (::verbose)
      cout << "    done!" << endl
	   << endl;
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

  ProveedorInsumo(Uid i = -1) noexcept : id(i) {}

  ProveedorInsumo(Uid i, Uid pid, Uid iid, double m, double cant) noexcept
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

    if (::verbose)
      cout << "Reading " << name << " for " << anho << endl;

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
    if (::verbose)
      cout << "Done!" << endl
	   << endl;
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

  bool es_nacional() const noexcept { return nacional; }

  bool es_extranjero() const noexcept { return not es_nacional(); }
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

    if (::verbose)
      cout << "Reading " << name << endl;
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
    if (::verbose)
      cout << "Done! " << endl
	   << endl;
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

inline string rif_ext(Uid origen_id, Uid procedencia_id, string nombre)
{
  replace(nombre.begin(), nombre.end(), ' ', '_');
  stringstream s;
  s << origen_id << " " << procedencia_id;
  return s.str() + " " + nombre;
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
  //    insumo_id, cod_aran
  DynMapTree<Uid, string> productos; // productos que fabrica y/o vende

      // Lista de socios accionistas. Cada par es rif accionista y %
  DynList<pair<string, float>> socios; 

  float total_accionistas() const noexcept
  {
    return socios.foldl<float>(0.0, [] (float acu, auto p)
  			       {
  				 return acu + p.second;
  			       });
  }

  Productor() {}

  Productor(bool nac, const string & r, const string & nom)
    : nacional(nac), rif(r), nombre(nom) {}

  string to_string() const
  {
    return (nacional ? "VEN" : "EXT") + BLANK + rif + BLANK + nombre;
  }

  friend ostream & operator << (ostream & out, const Productor & p)
  {
    out << (p.nacional ? "VEN" : "EXT")
	<< BLANK	<< p.rif << BLANK << p.nombre << endl
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
	<< productos.size();
    productos.for_each([&out] (auto p) 
		       {
			 out << BLANK << get<0>(p) << BLANK << get<1>(p);
		       }); 
    out << endl
    	<< socios.size();
    socios.for_each([&out] (auto p)
    		    { out << BLANK << p.first << BLANK << p.second; });
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
	string nom;
	in >> origen_id >> procedencia_id >> nom;
	rif = rif_ext(origen_id, procedencia_id, nom);
      }
    in.ignore();
    getline(in, nombre);

    size_t n;
    in >> n;
    for (auto i = 0; i < n; ++i)
      {
	Uid id;
	string cod_aran;
	in >> id >> cod_aran;
	productos.insert(id, cod_aran);
      }
    in >> n;                // leer socios
    for (auto i = 0; i < n; ++i)
      {
    	pair<string, float> p;
    	in >> p.first >> p.second;
    	socios.append(p);
      }
  }

  bool es_nacional() const noexcept { return nacional; }
  bool es_extranjero() const noexcept { return not nacional; }

  friend bool operator < (const Productor & p1, const Productor & p2) noexcept
  {
    return p1.rif < p2.rif;
  }
};

struct MetaProducto
{
  Uid id;
  string cod_aran;
  string nombre;
  DynSetTree<string> productores; // lista de rif de productores 
  DynList<LinearFactor> comb;   // lista uid insumo, cantidad

  string to_string() const
  {
    stringstream s;
    s << id << BLANK << cod_aran << BLANK << nombre << endl
      << productores.size() << " :";
    productores.for_each([&s] (auto p) { s << BLANK << p; });
    return s.str();
  }

  friend ostream & operator << (ostream & out, const MetaProducto & p) 
  {
    return out << p.to_string() << endl;
  }

  MetaProducto() {}
  MetaProducto(Uid i, const string & cod, const string & nom, 
	       const DynList<LinearFactor> & c)
    : id(i), cod_aran(cod), nombre(nom), comb(c) {}

  void save(ostream & out) const
  {
    out << id << BLANK << cod_aran << endl
	<< nombre << endl
	<< productores.size();
    productores.for_each([&out] (auto s) { out << BLANK << s; }); 
    out << endl
	<< comb.size();
    comb.for_each([&out] (auto c)
    { 
      out << " " << get<0>(c) << " " << get<1>(c) << " " 
	  << get<2>(c) << " " << get<3>(c);
    });
    out << endl;
  }

  void load(istream & in)
  {
    in >> id >> cod_aran;
    in.ignore();
    getline(in, nombre);
    size_t n;
    in >> n;                // leer productores
    for (auto i = 0; i < n; ++i)
      {
	string rif;
	in >> rif;
	productores.append(rif);
      }
    in.ignore();
    string line;
    getline(in, line);
    stringstream s(line);
    s >> n;                // leer fromula insumos
    for (auto i = 0; i < n; ++i)
      {
	Uid id;
	string cod;
	double factor;
	Uid arc_id;
	s >> id >> cod >> factor >> arc_id;
	comb.append(make_tuple(id, cod, factor, arc_id));
      }

    assert(eq(sort(comb.map<Uid>([] (auto c) { return get<0>(c); })), 
	      sort(unique(comb).map<Uid>([] (auto c) { return get<0>(c); }))));
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

  TablaMetaProductos() {}

  TablaMetaProductos(const TablaProductos & tabla_productos,
		     const TablaProductores & tabla_productores)
  {
    if (::verbose)
      cout << "Building MetaProdutos table" << endl;
    size_t n = tabla_productos.size();
    size_t i = 0;
    reserve(n);
    tabla_productos.for_each([this, &i] (auto & p)
      {
	access(i++) = MetaProducto(p.id, p.cod_aran, p.nombre, p.comb);
      });

    if (::verbose)
      cout << "Done!" << endl
	   << "    Sorting" << endl;
    in_place_sort(*this, cmp_cod);
    if (::verbose)
      cout << "    done!" << endl
	   << "    Building index ... " << endl;

    index = build_index_ptr(*this, cmp_id);

    if (::verbose)
      cout << "    done!" << endl;

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
    if (::verbose)
      cout << "Saving Productos" << endl;
    out << size() << endl;
    for_each([&out] (const auto & p) { p.save(out); });
    if (::verbose)
      cout << "done!" << endl;
  }

  TablaMetaProductos(istream & in)
  {
    if (::verbose)
      cout << "Loading MetaProductos" << endl;
    size_t n;
    in >> n;
    reserve(n);
    for (auto i = 0; i < n; ++i)
      access(i).load(in);
    index = build_index_ptr(*this, cmp_id);
    if (::verbose)
      cout << "end" << endl
	   << endl;
  }

  void autotest() const
  {
    if (::verbose)
      cout << "Checking productos consistency" << endl;
    const size_t n = size();
    for (auto i = 0; i < n; ++i)
      {
	auto p = access(i);
	auto cod_aran = p.cod_aran;
	auto l = (*this)(cod_aran);
	if (not l.all([&cod_aran] (auto p) { return p->cod_aran == cod_aran; }))
	  {
	    cout << "Para producto" << endl
		 << p << endl;
	    ERROR("Una de las entradas tiene cod_aran diferente");
	  }
	
	if ((*this)(p.id) == nullptr)
	  {
	    cout << "Para producto" << endl
		 << p << endl;
	    ERROR("id no encontrado");
	  }
      }
    if (::verbose)
      cout << "done! " << endl
	   << endl;
  } 
};

struct MetaInsumo 
{
  Uid id; // campo insumo_id
  string nombre; 

  MetaInsumo() {}
  MetaInsumo(Uid i, const string & nom) : id(i), nombre(nom) {}

  void save(ostream & out) const
  {
    out << id << endl
	<< nombre << endl;
    // cout << id << " " << nombre << endl;
  }

  void load(istream & in)
  {
    in >> id;
    in.ignore();
    getline(in, nombre);
    // cout << id << " " << nombre << endl;
  }
};

struct TablaMetaInsumos : public DynArray<MetaInsumo>
{
  typedef bool (*CmpId)(const MetaInsumo &, const MetaInsumo &);

  CmpId cmp_id = [] (const MetaInsumo & p1, const MetaInsumo & p2)
  { return p1.id < p2.id; };

  DynArray<MetaInsumo*> index;

  TablaMetaInsumos() {}

  TablaMetaInsumos(const TablaInsumos & tabla_insumos)
  {
    if (::verbose)
      cout << "Building MetaInsumos table" << endl;
    size_t n = tabla_insumos.size();
    size_t i = 0;
    reserve(n);
    tabla_insumos.for_each([this, &i] (auto & p)
      {
	access(i++) = MetaInsumo(p.id, p.nombre);
      });

    if (::verbose)
      cout << "Done!" << endl
	   << "    Sorting" << endl;
    in_place_sort(*this, cmp_id);

    if (::verbose)
      cout << "    done!" << endl;
  }

  MetaInsumo * operator () (Uid id) const noexcept
  {
    MetaInsumo p; p.id = id;
    return bsearch(*this, p, cmp_id);
  }

  void save(ostream & out) const
  {
    if (::verbose)
      cout << "Saving Inputs" << endl;
    out << size() << endl;
    for_each([&out] (const auto & p) { p.save(out); });
    if (::verbose)
      cout << "done!" << endl;
  }

  TablaMetaInsumos(istream & in)
  {
    if (::verbose)
      cout << "Loading MetaInsumos" << endl;
    size_t n;
    in >> n;
    reserve(n);
    for (auto i = 0; i < n; ++i)
      access(i).load(in);
    if (::verbose)
      cout << "end" << endl
  	   << endl;
  }
};

/*
 * Orden de construcción debe ser *estrictamente* así
 *
 * 1. Contruir el grafo. De aquí se calculan los mimebros comb por cada producto
 *
 * 2. Construir la tabla de productores
 *
 * 3. Construir la tablas de productos
 *
 */

struct TablaProductores : public DynArray<Productor>
{
  void adjuntar_productos(const TablaProductos & tabla_productos,
			  const TablaPlantas & tabla_plantas)
  {
    if (::verbose)
      cout << "    Computing products for each producer ..." << endl;
    tabla_productos.for_each([this, &tabla_plantas] (auto & prod)
      {	
	auto planta_id = prod.planta_id;
	auto planta_ptr = tabla_plantas(planta_id);
	assert(planta_ptr != nullptr);
	auto productor_ptr = (*this)(planta_ptr->rif);
	productor_ptr->productos.append(make_pair(prod.id, prod.cod_aran));
      });
    if (::verbose)
      cout << "    done" << endl;
  }

  void adjuntar_socios(const TablaSocios & tabla_socios)
  {
    tabla_socios.for_each([this] (const Socio & s)
      {
  	s.empresas.for_each([this, &s] (auto p)
          {
  	    auto prod_ptr = (*this)(p.first);
  	    if (prod_ptr == nullptr)
  	      throw domain_error("adjuntar_socios(): rif unidad ecom no esta");
  	    prod_ptr->socios.append(make_pair(s.rif, p.second));
  	  });
      });
  }

  TablaProductores() {}

  TablaProductores(const TablaUnidadesEconomicas & tabla_ue,
		   const TablaSocios &             tabla_socios,
		   const TablaProveedores &        tabla_proveedores,
		   const TablaProductos &          tabla_productos,
		   const TablaPlantas &            tabla_plantas)
  {
    if (::verbose)
      cout << "Building Productors table" << endl
	   << "    Reading econimical unities ... " << endl;

    DynSetTree<Productor> tree;

    // Inserta primero las unidades económicas
    tabla_ue.for_each([&tree] (const auto & ue)
      {
	tree.emplace(true, ue.rif, ue.nombre);
      });
    
    if (::verbose)
      cout << "    done" << endl
	   << "    Reading suppliers ... " << endl;

    // Ahora los proveedores, algunos de los cuales pueden ser unidades
    // económicas, en cuyo caso ya deberían de estar insertadas
    tabla_proveedores.for_each([&tree] (const auto & prov)
      {
	if (prov.nacional)
	  {
	    tree.emplace(true, prov.rif, prov.nombre);
	    return;
	  }
	tree.insert(Productor(false, rif_ext(prov.pais_origen_id,
					     prov.pais_procedencia_id,
					     prov.nombre), 
			      prov.nombre));
      });

    if (::verbose)
      cout << "    done" << endl
	   << "    building the table ... " << endl;
    
    reserve(tree.size()); size_t i = 0;
    tree.mutable_for_each([this, &i] (auto & prod) 
			  { access(i++) = move(prod); });

    if (::verbose)
      cout << "    done" << endl;

    tree.empty();
    
    adjuntar_productos(tabla_productos, tabla_plantas);
    adjuntar_socios(tabla_socios);
  }
  
  // Solo funciona para los nacionales 
  Productor * operator () (const string & rif) const
  {
    Productor p;
    p.rif = rif;
    return bsearch(*this, p);
  }

  // Determina el índice en la tabla de la entrada ptr
  size_t operator () (Productor * ptr) const
  {
    long ret = binary_search(*this, *ptr);
    if (ret < 0)
      throw domain_error("TablaProductores (): productor no encontrado");
    assert(ptr == &access(ret));
    return ret;
  }

  void save(ostream & out) const
  {
    if (::verbose)
      cout << "Saving Productores" << endl;
    out << size() << endl;
    for_each([&out] (const auto & p) { p.save(out); });
    if (::verbose)
      cout << "done!" << endl;
  }

  TablaProductores(istream & in)
  {
    if (::verbose)
      cout << "Loading Productores" << endl;
    size_t n;
    in >> n;
    reserve(n);
    for (size_t i = 0; i < n; ++i)
      access(i).load(in);
    if (::verbose)
      cout << "done" << endl 
	   << endl;
  }

  void autotest() const
  {
    if (::verbose)
      cout << "Checking for producers consistency" << endl;
    if (not is_sorted(*this))
      ERROR("autotest: not sorted");

    for (size_t i = 0, n = size(); i < n; ++i)
      {
	auto & p = access(i);
	if ((*(this))(p.rif) == nullptr)
	  {
	    cout << "Productor" << endl
		 << p << endl
		 << "    NO ENCONTRADO" << endl;
	    ERROR("");
	  }
	if ((*this)(&p) != i)
	  {
	    cout << "Indice de Productor dado puntero" << endl
		 << p << endl
		 << "    mal cualculado" << endl;
	    ERROR("");
	  }
      }

    if (::verbose)
      cout << "done" << endl
	   << endl;
  }
};

void TablaMetaProductos::adjuntar_productores
(const TablaProductores & tabla_productores)
{
  if (::verbose)
    cout << "    Computing producer for each product ... " << endl;
  tabla_productores.for_each([this] (const auto & productor)
    {
      productor.productos.for_each([this, &productor] (auto p)
        {
	  auto producto_id = get<0>(p);
	  auto producto_ptr = (*this)(producto_id);
	  producto_ptr->productores.append(productor.rif);
	});
    });
  if (::verbose)
    cout << "    done" << endl
	 << endl;
}
  
