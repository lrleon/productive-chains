
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
   
       reachable <mapa-var> <productor-exp-1> <productor-exp-2>

   Este comando calcula linealmente si de alguna manera el el productor 
   <productor-exp-1> está relacionado con el productor <productor-exp-2>.

   Un expresión válida para un productor es un cadena constante con un rif,
   una variable string con el rif, una variable productor o una variable nodo.

   NOTA: Si los productores están relacionados la respuesta no indica el 
   sentido de relación; es decir, cuál de los dos productores involucrados
   antecede al otro.

   Use path para calcular un camino exacto.
   )";

static const char * cover =
R"(Calcula el grafo total de crubrimiento a partir de un nodo
   
       cover <mapa-var> <nodo-var>

   El grafo de cubrimiento es el grafo total que es alcanzable desde el nodo
   <nodo-var>. 

   <nodo-var> debe ser una variable de tipo nodo asignada mediante search node
   )";

static const char * upstream =
R"(Calcula el grafo aguas arriba a partir de un nodo y un producto
   
       upstream <mapa-var> <nodo-var> <prod-exp>

   Dado la variable nodo <nodo-var> sobre el mapa <mapa-var>, upstream revisa 
   la composición de insumos del producto en la expresión <prod-var>. A 
   partir de allí se miran hacia atrás los arcos de entrada de <nodo-var>
   que se correspondan con los insumos de <prod-var>. Luego, recursivamente, 
   se examinan los nodos asociados a los insumos y así recursivamente hasta 
   que ya nos sea posible.

   <prod-var> puede ser:

   1. Una constante o variable string conteniendo un código arancelario

   2. Una constante o variable entera conteniendo el id del producto

   3. Una variable de tipo producto
   )";

static const char * up =
R"(Calcula el grafo aguas arriba a partir de un nodo y un producto
   
       upstream <mapa-var> <nodo-var> <prod-exp> <int-exp>

   Dado la variable nodo <nodo-var> sobre el mapa <mapa-var>, upstream revisa 
   la composición de insumos del producto en la expresión <prod-var>. A 
   partir de allí se miran hacia atrás los arcos de entrada de <nodo-var>
   que se correspondan con los insumos de <prod-var>. Luego, recursivamente, 
   se examinan los nodos asociados a los insumos y así recursivamente hasta 
   que ya nos sea posible.

   <prod-var> puede ser:

     1. Una constante o variable string conteniendo un código arancelario

     2. Una constante o variable entera conteniendo el id del producto

     3. Una variable de tipo producto

   Los insumos son buscados mediante comparación con los nombres de insumo 
   versus producto. Para ello el criterio de mínima distancia de edición es
   usado en cojunto con un límte máximo entero <th-exp>. Si el nombre de insumo 
   más cercano al nombre del producto en distancia de edición excede <th-exp>
   entonces el insumo no es considerado
   )";

static const char * inputs =
R"(Calcula los insumos de entrada de un producto o los arcos de entrada de 
   un nodo.
   
       inputs <mapa-var> <nodo-var>

   Retorna los arcos de entrada del nodo <nodo-var> en el mapa <mapa-var>


       input <mapa-vap> <producto-exp>

   Retorna los insumos requeridos para fabricar el producto <producto-exp>.
   
   <producto-exp> puede ser un entero en cuyo caso se asume es un producto_id,
   o una variable entera conteniendo el producto_id o una variable producto.
   )";

static const char * arcs_help =
R"(Métodos de búsqueda de arcos
   
       arcs input id <mapa-var> <producer-exp>

   Retorna los arcos de entrada del productor <producer-exp> en el
   mapa <mapa-var>

       arcs product id <mapa-var> <producer-exp>


   Retorna los arcos de salida del productor <producer-exp> en el
   mapa <mapa-var>

       arcs regex <mapa-var> <regex-exp>

   Retorna los arcos del mapa <mapa-var> cuyo nombre de insumo encaje con
   la expresión regular <regex-exp>.

   ADVERTENCIA: si el mapa es muy grande, esta búsqueda puede ser muuuuuuuuuy
                leeeeeeeeeeenta.


       arcs input <mapa-vap> <producer-exp> <producto-exp>

   Retorna los arcos de entrada del <producto-exp> producido por el productor
   <producer-exp> en el mapa <mapa-vap>


       arcs output <mapa-vap> <producer-exp> <producto-exp>

   Retorna los arcos de salida del <producto-exp> producido por el productor
   <producer-exp> en el mapa <mapa-vap>

   
       arcs <mapa-var> <arc-id-exp>

   Retorna el arco del mapa <mapa-var> cuyo arco id es <arco-id-exp>
   )";

static const char * path_help =
R"(Calcula el camino mínimo (en arcos) entre un par de productores

       path <mapa-var> <productor-exp-1> <productor-exp-2>

   Este comando calcula linealmente un camino mínimo que comienza en el 
   productor definido por <productor-exp-1> y que termina en el productor
   definido con <productor-exp-2>.

   Un expresión válida para un productor es un cadena constante con un rif,
   una variable string con el rif, una variable productor o una variable nodo.

   NOTA: Tome en cuenta que si no existe el camino, quizá podría existir el
   inverso; es decir desde <productor-exp-2> hasta <productor-exp-1>.
   )";

static const char * rm_help =
R"(Elimina un arco de una red.

       rm arc <mapa-var> <net->var> <src-exp> <tgt_exp>

   Elimina de la red <net-var> asociada al mapa <mapa-var> el arco que conecta
   a los nodos <src-exp> y <tgt-exp>.

   <src-exp> y <tgt-exp> pueden ser variables o cadenas constantes con rif, 
   variables productor o variables nodo.

   NOTA: eliminar arcos dados dos nodos es ambigüo en el sentido de que
         pueden existir varios arcos. Usa la eliminación por id si requieres
         más precisión

   
       rm arc <mapa-var> <net->var> <id-exp>

   Elimina de la red <net-var> asociada al mapa <mapa-var> el arco cuyo id
   sea <id-exp>. <id-exp> debe eser un identificador de arco y puede ser 
   una constante o variable entera.

   CUIDADO: cualquier método para eliminar arco puede dejar a la red inconexa.
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
    case UPSTREAM: cout << up << endl; break;
    case INPUTS: cout << inputs << endl; break;
    case ARCS: cout << arcs_help << endl; break;
    case PATH: cout << path_help << endl; break;
    case RMARC: cout << rm_help << endl; break;
    default: cout << "No help topic" << endl; break;
    }
  return make_pair(true, "");
}
