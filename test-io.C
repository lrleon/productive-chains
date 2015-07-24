
# include <grafo.H>

bool verbose = false;

int main()
{
  ifstream in("productores.txt");
  TablaProductores tbl(in);
  tbl.save(cout);
}
