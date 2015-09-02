
# include <net-tree.H>

static const char * load =
  R"(LOAD <string-exp>"

  Lee el mapa expresado por <string-exp> y retorna un operando
  de tipo Mapa el cual puede almacenarse en una variable

  <string-exp> puede ser una constante string o una variable que
  especifica el nombre del archivo donde se encuentra el mapa

  Ejemplo:

      data = \"data.txt\"
      mapa = load data
  )";

static const char * search =
  R"(search define a una gama de comandos de busqueda

     Busqueda de productor por rif

     search producer <mapa-var> <rif-exp>

         Busca productor segun <rif-exp> en el mapa <mapa-var> 

     search producers <mapa-var> <reg-exp>

         Busca en el mapa <mapa-var> todos los productores cuyo nombre
         encaje con la expresioon regular <reg-exp>

     search product id <mapa-var> <id-exp>

         Busca en el mapa <mapa-var> el producto identificado con <id-exp>

     search product regex <mapa-var> <id-exp>

         Busca en el mapa <mapa-var> todos los productos cuyo nombre encaje
         con la expresión regular <reg-exp>

     search product cod <mapa-var> <cod-aran>

         Busca en el mapa <mapa-var> todos los productos cuyo codigo
         arancelario sea <cod-aran>

     search product rif <mapa-var> <rif>

         Busca en el mapa <mapa-var> todos los productos producidos por
         el ente productivo con rif <rif>

     search node <mapa-var> <producer-exp>

         Busca en el mapa <mapa-var> un nodo de red que corresponda al
         productor <producer-var>. <producer-var> puede ser un rif en una 
         cadena de caracteres, como constante o como variable, o también podría
         ser una variable de tipo productor
     )";

static const char * help = 
  R"(Ayuda basica para analizador de cadenas productivas

     Puedes tipear 
       
         help <topico>

     Para obtener una breve ayuda sobre un tema de interes.

     Posible, aunque seguramente incompleta, lista de tópicos:

       load
       search       

     @ 2015 CENDITEL
)";

static const char * typeinfo =
R"(Informacion basica de una variable
   
     type <nom-var>

   Retorna el tipo de variable y en caso de que se trate de una lista 
   retorna su longitud
   )";

static const char * info =
R"(Informacion sobre una variable y su contenido
   
     info <nom-var>

   Retorna el tipo de variable y el contenido completo de la variable

   CUIDADO: si la variable es una lista, entonces todo su contenido es
            mostrado. 

   NOTA: Puedes prescindir de la palabra info y simplemente tipear el nombre
         de la variable
   )";

static const char * reachable =
R"(Determina si dos nodos están relacionados
   
       reachable <mapa-var> <nodo-var-1> <nodo-var-2>
   )";

static const char * cover =
R"(Calcula el grafo total de crubrimiento a partir de un nodo
   
       cover <mapa-var> <nodo-var>
   )";

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
    case TYPEINFO: cout << typeinfo << endl; break;
    case INFO: cout << info << endl; break;
    case REACHABLE: cout << reachable << endl; break;
    case COVER: cout << cover << endl; break;
    default: cout << "No help topic" << endl; break;
    }
  return make_pair(true, "");
}
