
# include <iostream>
# include <fstream>
# include <tclap/CmdLine.h>

# include <tpl_dynArray.H>

# include <tablas.H>


using namespace TCLAP;

bool verbose = false;

int main()
{
  ifstream in("productos.txt");
  TablaMetaProductos tbl(in);
  tbl.save(cout);  
}
