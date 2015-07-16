
# include <grafo.H>




int main()
{
  ifstream in("red-1012.txt");
  GrafoSigesic g = cargar_grafo(in);

  guardar_grafo(g, cout);

}
