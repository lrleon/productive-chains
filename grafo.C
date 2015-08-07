
# include <grafo.H>

void guardar_grafo(GrafoSigesic & g, ostream & out)
{
  IO_Graph<GrafoSigesic, Carga_Nodo, Guarda_Nodo>(g).save_in_text_mode(out);
}


GrafoSigesic cargar_grafo(istream & in)
{
  GrafoSigesic g;
  
  IO_Graph<GrafoSigesic, Carga_Nodo, Guarda_Nodo>(g).load_in_text_mode(in);

  return g;
}
