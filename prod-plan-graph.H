# ifndef PRODPLANGRAPH_H
# define PRODPLANGRAPH_H

# include <tpl_dynListQueue.H>
# include <tpl_dynSetTree.H>
# include <tpl_agraph.H>

# include <net.H>

using namespace std;
using namespace Aleph;

struct NodeInfo
{
  MetaProducto * product;
  double         quantity;
  bool           is_in_queue = false;

  NodeInfo(MetaProducto * p, double q)
    : product(p), quantity(q)
  {
    // Empty
  }

  bool operator == (const NodeInfo & info) const
  {
    return info.product == product;
  }

  friend ostream & operator << (ostream & out, const NodeInfo & info)
  {
    out << info.product->nombre << "\\n" << info.quantity;
    return out;
  }
};

using PPNode = Graph_Anode<NodeInfo>;
using PPArc = Graph_Aarc<double>; // double for quantity in relationship
using PPGraph = Array_Graph<PPNode, PPArc>;

struct ProdPlanGraph : PPGraph
{
  using PPGraph::PPGraph;

  MetaMapa * map;

               // Map, Product, quantity
  ProdPlanGraph(MetaMapa * m)
    : PPGraph(), map(m)
  {
    // Empty
  }

  /// Builds a production plan with an breadth first algorithm.
  void build_pp(MetaProducto *, double, size_t);
};

# endif // PRODPLANGRAPH_H
