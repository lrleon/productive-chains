
# include <iostream>
# include <fstream>
# include <tclap/CmdLine.h>

# include <tpl_dynArray.H>

# include <grafo.H>

using namespace TCLAP;

bool verbose = true;

int main()
{
  ifstream in("grafo.txt");
  auto g = cargar_grafo(in);
  guardar_grafo(g, cout);  
}
