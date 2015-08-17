
# include <net-tree.H>

# define ENDL "\n"

static const char * load =
  R"(    LOAD <string-exp>" ENDL

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
     )";

static const char * help = 
  R"(Ayuda basica para analizador de cadenas productivas

     Puedes tipear 
       
         help <topico>

     Para obtner una breve ayuda.

     Posible, aunque seguramente incompleta, lista de tópicos:

       load
       search       

     @ 2015 CENDITEL
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
    default: cout << "No help topic" << endl; break;
    }
  return make_pair(true, "");
}
