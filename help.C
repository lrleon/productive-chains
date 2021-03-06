
# include <net-tree.H>

static const char * demand =
  R"(DEMAND <map-var> <id-exp> <cant> <max-threshold>

  Estudia si en el mapa expresado por <map-var> se satisface la demanda <cant>
  del bien identificado con <id-exp>. <max-threshold> especifica la distancia
  de edici�n m�xima entre dos nombres de productos para hacer emparejamiento

  Ejemplo:

  d = demand mapa 12345 1000 50

)";

static const char * prodplan =
  R"(PRODPLAN <map-var> <id-exp> <cant> <max-threshold>
     PRODPLAN <map-var> <id-exp> <cant> <max-threshold> <producer-set>
     PRODPLAN <map-var> <res-demand> <max-threshold>
     PRODPLAN <map-var> <res-demand> <max-threshold> <producer-set>

  Ejemplo:

  pp = prodplan mapa 12345 1000 50

)";

static const char * producerset =
  R"(PRODUCERSET <mapa> { <rif-1>, <rif-2>, ..., <rif-n> }
    
  Ejemplos:

  ps1 = producerset mapa { \"J123456789\", \"G987654321\" }
  ps2 = producerset mapa { }
)";

static const char * addtoproducerset =
  R"(ADDTOPRODUCERSET <mapa> <producer-set> { <rif-1>, <rif-2>, ..., <rif-n> }
    
  Ejemplo:

  addtoproducerset mapa ps { \"J123456789\", \"G987654321\" }
)";

static const char * rmfromproducerset =
  R"(RMFROMPRODUCERSET <mapa> <producer-set> { <rif-1>, <rif-2>, ..., <rif-n> }
    
  Ejemplo:

  rmfromproducerset mapa ps { \"J123456789\", \"G987654321\" }
)";

static const char * load =
  R"(LOAD <string-exp>

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
         encaje con la expresion regular <reg-exp>

     search product id <mapa-var> <id-exp>

         Busca en el mapa <mapa-var> el producto identificado con <id-exp>

     search product regex <mapa-var> <id-exp>

         Busca en el mapa <mapa-var> todos los productos cuyo nombre encaje
         con la expresi�n regular <reg-exp>

     search product cod <mapa-var> <cod-aran>

         Busca en el mapa <mapa-var> todos los productos cuyo codigo
         arancelario sea <cod-aran>

     search product rif <mapa-var> <rif>

         Busca en el mapa <mapa-var> todos los productos producidos por
         el ente productivo con rif <rif>

     search node <mapa-var> <producer-exp>

         Busca en el mapa <mapa-var> un nodo de red que corresponda al
         productor <producer-var>. <producer-var> puede ser un rif en una 
         cadena de caracteres, como constante o como variable, o tambi�n podr�a
         ser una variable de tipo productor
     )";

static const char * help = 
  R"(Ayuda basica para analizador de cadenas productivas

     Puedes tipear 
       
         help <topico>

     Para obtener una breve ayuda sobre un tema de interes.

     Posible, aunque seguramente incompleta, lista de t�picos:

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
R"(Determina si dos nodos est�n relacionados
   
       reachable <mapa-var> <productor-exp-1> <productor-exp-2>

   Este comando calcula linealmente si de alguna manera el el productor 
   <productor-exp-1> est� relacionado con el productor <productor-exp-2>.

   Un expresi�n v�lida para un productor es un cadena constante con un rif,
   una variable string con el rif, una variable productor o una variable nodo.

   NOTA: Si los productores est�n relacionados la respuesta no indica el 
   sentido de relaci�n; es decir, cu�l de los dos productores involucrados
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
   
       upstream <mapa-var> <nodo-var> <prod-exp> <int-exp>

   Dado la variable nodo <nodo-var> sobre el mapa <mapa-var>, upstream revisa 
   la composici�n de insumos del producto en la expresi�n <prod-var>. A 
   partir de all� se miran hacia atr�s los arcos de entrada de <nodo-var>
   que se correspondan con los insumos de <prod-var>. Luego, recursivamente, 
   se examinan los nodos asociados a los insumos y as� recursivamente hasta 
   que ya nos sea posible.

   <prod-var> puede ser:

     1. Una constante o variable string conteniendo un c�digo arancelario

     2. Una constante o variable entera conteniendo el id del producto

     3. Una variable de tipo producto

   Los insumos son buscados mediante comparaci�n con los nombres de insumo 
   versus producto. Para ello el criterio de m�nima distancia de edici�n es
   usado en cojunto con un l�mte m�ximo entero <th-exp>. Si el nombre de insumo 
   m�s cercano al nombre del producto en distancia de edici�n excede <th-exp>
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
R"(M�todos de b�squeda de arcos
   
       arcs input id <mapa-var> <producer-exp>

   Retorna los arcos de entrada del productor <producer-exp> en el
   mapa <mapa-var>

       arcs product id <mapa-var> <producer-exp>


   Retorna los arcos de salida del productor <producer-exp> en el
   mapa <mapa-var>

       arcs regex <mapa-var> <regex-exp>

   Retorna los arcos del mapa <mapa-var> cuyo nombre de insumo encaje con
   la expresi�n regular <regex-exp>.

   ADVERTENCIA: si el mapa es muy grande, esta b�squeda puede ser muuuuuuuuuy
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
R"(Calcula el camino m�nimo (en arcos) entre un par de productores

       path <mapa-var> <productor-exp-1> <productor-exp-2>

   Este comando calcula linealmente un camino m�nimo que comienza en el 
   productor definido por <productor-exp-1> y que termina en el productor
   definido con <productor-exp-2>.

   Un expresi�n v�lida para un productor es un cadena constante con un rif,
   una variable string con el rif, una variable productor o una variable nodo.

   NOTA: Tome en cuenta que si no existe el camino, quiz� podr�a existir el
   inverso; es decir desde <productor-exp-2> hasta <productor-exp-1>.
   )";

static const char * rmarc_help =
R"(Elimina un arco de una red.

       rm arc <var> <net->var> <src-exp> <tgt_exp>

   Elimina del mapa o red <var> el arco que conecta a los nodos
   <src-exp> y <tgt-exp>.

   <src-exp> y <tgt-exp> pueden ser variables o cadenas constantes con rif, 
   variables productor o variables nodo.

   NOTA: eliminar arcos dados dos nodos es ambig�o en el sentido de que
         pueden existir varios arcos. Usa la eliminaci�n por id si requieres
         m�s precisi�n

   
       rm arc <var> <id-exp>

   Elimina del mapa o red <var> el arco cuyo id sea <id-exp>. <id-exp> debe
   ser un identificador de arco y puede ser una constante o variable entera.

   CUIDADO: cualquier m�todo para eliminar arco puede dejar a la red inconexa.
   )";

static const char * rmnode_help =
R"(Elimina un arco de una red.

       rm node <var> <node-exp>

   Elimina del mapa o red <var> el nodo <node-exp>.

   <node-exp> puede ser una cadena con el rif, una variable productor o una 
   variable nodo.
   )";

static const char * shareholder_help =
R"(Reporte de accionistas por empresa

       search shareholder <mapa-var> <producer-exp>

   Proporciona la lista de accionistas de la empresa <producer-exp> en el 
   mapa <mapa-var>. <producer-var> pruede ser una cadena con el rif 
   (constante o variable), una variable productor o una variable nodo.  


       search shareholder regex <mapa-var> <reg-exp>

   Proporciona los accionistas vinculados a las empresas del mapa <map-var>
   cuyo nombre corresponda con la expresi�n regular <reg-exp>
   )";

static const char * holder_help =
R"(Reporte de empresas por accionista

       search holding <mapa-var> <shareholder-exp>

   Proporciona la lista de empresas del mapa <mapa-var> pose�das por el
   propietario <shareholder-exp>. <shareholder-exp> debe ser una cadena 
   (constante o variable) con el rif del propietario.


       search holding regex <mapa-var> <reg-exp>

   Porporciona la lista de accionistas que correspondan con la expresi�n 
   regular <reg-exp> y las empresas sobre las cuales estos ejercen propiedad.
   )";

static const char * hegemony_help =
R"(Reporte de empresas por hegemon�a

       search hareholder hegemony <mapa-var> <int-exp>

   Proporciona la lista de accionistas del mapa <mapa-var> que tengan m�s
   de <int-exp> empresas; El resultado es ordenado desdecendentemente por 
   cantidad de empresas.
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
    case UPSTREAM: cout << upstream << endl; break;
    case INPUTS: cout << inputs << endl; break;
    case ARCS: cout << arcs_help << endl; break;
    case PATH: cout << path_help << endl; break;
    case RMARC: cout << rmarc_help << endl; break;
    case RMNODE: cout << rmnode_help << endl; break;
    case SHAREHOLDER: cout << shareholder_help << endl; break;
    case HOLDER: cout << holder_help << endl; break;
    case HEGEMONY: cout << hegemony_help << endl; break;
    case DEMAND: cout << demand << endl; break;
    case PRODPLAN: cout << prodplan << endl; break;
    case PRODSET: cout << producerset << endl; break;
    case ADDPRODUCERSET: cout << addtoproducerset << endl; break;
    case RMPRODUCERSET: cout << rmfromproducerset << endl; break;
      
    default: cout << "No help topic" << endl; break;
    }
  return make_pair(true, "");
}
