# ifndef PRODPLANGRAPH_H
# define PRODPLANGRAPH_H

# include <htlist.H>
# include <tpl_dynListQueue.H>
# include <tpl_dynSetTree.H>
# include <tpl_agraph.H>

# include <net.H>

using namespace std;
using namespace Aleph;

struct NodeInfo
{
  void *         product;
  double         quantity;
  Planta *       plant = nullptr;
  bool           is_in_queue = false;

  enum class ProductType { Product, Input } type;

  NodeInfo(void * p, double q, ProductType t = ProductType::Product)
    : product(p), quantity(q), type(t)
  {
    // Empty
  }

  string get_cmp_key() const;

  Uid get_id() const;

  const string & get_name() const;

  bool operator == (const NodeInfo & info) const
  {
    return info.product == product;
  }

  friend ostream & operator << (ostream & out, const NodeInfo & info)
  {
    out << info.get_name() << "\\n" << info.quantity << "\\n"
	<< (info.plant == nullptr ? "New plant" : info.plant->nombre);
    return out;
  }
};

using PPNode = Graph_Anode<NodeInfo>;
using PPArc = Graph_Aarc<double>; // double for quantity in relationship
using PPGraph = Array_Graph<PPNode, PPArc>;

using ProducerSet = DynSetTree<Productor *, Treap>;

                       /* plant, capacity (available quantity) */
using PlantCap = tuple<Planta *, double>;

struct CmpPlantCap
{
  bool operator () (const PlantCap & a, const PlantCap & b) const
  {
    return get<0>(a) < get<0>(b);
  }
};

using PlantTable = DynSetTree<PlantCap, Treap, CmpPlantCap>;

using ProductMap = DynMapTreap<MetaProducto *, PlantCap *>;

using PlantMap = DynMapTreap<string, ProductMap>;

using ClassPlantSet = tuple<PlantMap, PlantTable>;

struct CmpNetArcs
{
  bool operator () (Net::Arc * a1, Net::Arc * a2) const
  {
    return a1->get_info().arco_id < a2->get_info().arco_id;
  }
};

struct CmpPPNodes
{
  bool operator () (PPNode * p, PPNode * q) const
  {
    return p->get_info().get_cmp_key() < q->get_info().get_cmp_key();
  }
};

struct CmpPPArcs
{
  bool operator () (PPArc * a1, PPArc * a2) const
  {
    if (a1->src_node < a2->src_node)
      return true;
    
    return not (a2->src_node < a1->src_node) and a1->tgt_node < a2->tgt_node;
  }
};

using NetArcsIdx = DynSetTreap<Net::Arc *, CmpNetArcs>;
using PPNodesIdx = DynSetTreap<PPGraph::Node *, CmpPPNodes>;
using PPArcsIdx = DynSetTreap<PPGraph::Arc *, CmpPPArcs>;

struct ProdPlanGraph : PPGraph
{
  using PPGraph::PPGraph;

  MetaMapa * map;
  
  ProdPlanGraph(MetaMapa * m = nullptr)
    : PPGraph(), map(m)
  {
    // Empty
  }

  Net::Arc * search_net_arc(NetArcsIdx &, Uid);

  tuple<Node *, double>
  create_node_and_connect(void *, double, NodeInfo::ProductType, Node *,
			  PPNodesIdx &, PPArcsIdx &, ClassPlantSet &, double);
  
  void build_pp(DynList<pair<MetaProducto *, double>> &, size_t,
		ProducerSet &);

  void build_pp(DynList<pair<MetaProducto *, double>> &, size_t);
  
private:
  /// Builds a production plan with a breadth first algorithm.
  void build_pp(MetaProducto *, double, size_t, NetArcsIdx &,
		ClassPlantSet &, PPNodesIdx &, PPArcsIdx &);

  ClassPlantSet classify_plants(ProducerSet &);
};

# endif // PRODPLANGRAPH_H
