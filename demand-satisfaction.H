# ifndef DEMAND_SATISFACTION_H
# define DEMAND_SATISFACTION_H

# include <htlist.H>
# include <tpl_dynListQueue.H>
# include <tpl_dynSetTree.H>

# include <net.H>

struct DemandSatisfaction
{
  using PQ = pair<MetaProducto *, double>;
  using IQ = pair<MetaInsumo *, double>;
  using Result = tuple<bool, DynList<PQ>, DynList<IQ>>;
  
  MetaMapa * map;

  DemandSatisfaction(MetaMapa * m = nullptr)
    : map(m) { }

  struct CmpNetArcs
  {
    bool operator () (Net::Arc * a, Net::Arc * e)
    {
      return a->get_info().arco_id < e->get_info().arco_id;
    }
  };
  
  using ArcsIndex = DynSetTreap<Net::Arc *, CmpNetArcs>;
  
  Net::Arc * search_net_arc(ArcsIndex &, Uid);

  using Tuple = tuple<string, Uid, double>;

  using QTuple = tuple<MetaProducto *, double>;
  
  double infer_increase(const Tuple &);

  Result simple_aproach(MetaProducto *, double, size_t);
};

# endif
