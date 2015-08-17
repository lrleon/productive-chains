
# include <net-tree.H>

# define ENDL "\n"

static const char * load =
  "    LOAD <string-exp>" ENDL
  ENDL
  "Lee el mapa expresado por <string-exp> y retorna un operando" ENDL
  "de tipo Mapa el cual puede almacenarse en una variable" ENDL
  ENDL
  "<string-exp> puede ser una constante string o una variable que" ENDL
  "especifica el nombre del archivo donde se encuentra el mapa" ENDL 
  ENDL
  "Ejemplo:" ENDL
  ENDL
  "    data = \"data.txt\"" ENDL
  "    mapa = load data" ENDL
  ENDL;

static const char * search =
  "search define a una gama de comandos de busqueda" ENDL
  ENDL
  "Busqueda de productor por rif" ENDL
  ENDL
  "search producer <mapa-var> <rif-exp>" ENDL
  ENDL
  "    Busca productor segun <rif-exp> en el mapa <mapa-var> " ENDL
  ENDL
  "search producers <mapa-var> <reg-exp>" ENDL
  ENDL
  "    Busca en el mapa <mapa-var> todos los productores cuyo nombre" ENDL
  "    encaje con la expresioon regular <reg-exp>" ENDL
  ENDL
  "search product id <mapa-var> <id-exp>" ENDL
  ENDL 
  "    Busca en el mapa <mapa-var> el producto identificado con <id-exp>" ENDL
  ENDL
  "search product regex <mapa-var> <id-exp>" ENDL
  ENDL 
  "    Busca en el mapa <mapa-var> todos los productos cuyo nombre encaje" ENDL
  "    con la expresión regular <reg-exp>" ENDL
  ENDL;

static const char * help =
  "Ayuda basica para analizador de cadenas productivas" ENDL;

ExecStatus Help::execute()
{
  cout << endl;
  switch (type)
    {
    case MAP: cout << load; break;
    case HELP: cout << help; break;
    case SEARCHPRODUCER:
    case SEARCHPRODUCERREGEX:
    case SEARCHPRODUCTID:
    case SEARCHPRODUCTREGEX: cout << ::search; break;      
    default: cout << "No help topic" << endl; break;
    }
  return make_pair(true, "");
}
