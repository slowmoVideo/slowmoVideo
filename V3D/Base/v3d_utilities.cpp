#include "Base/v3d_utilities.h"

#include <queue>
#include <list>
#include <map>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/connected_components.hpp>

using namespace std;

namespace
{

   template <typename T>
   struct PropertyMapFromVector
   {
         typedef T value_type;
         typedef T& reference;
         typedef int key_type;
         typedef boost::read_write_property_map_tag category;

         PropertyMapFromVector(std::vector<T>& vec_)
            : vec(vec_)
         { }

         std::vector<T>& vec;
   }; // end struct PropertyMapFromVector

   template <typename T>
   inline T&
   get(PropertyMapFromVector<T>& pmap, int key) { return pmap.vec[key]; }

   template <typename T>
   inline void
   put(PropertyMapFromVector<T>& pmap, int key, T val) { pmap.vec[key] = val; }

} // end namespace

namespace V3D
{

   void
   getMinimumSpanningForest(vector<pair<int, int> > const& edges, vector<double> const& weights,
                            vector<pair<int, int> >& mstEdges, vector<set<int> >& connComponents)
   {
      CompressedRangeMapping nodeIdMap;
      for (size_t i = 0; i < edges.size(); ++i)
      {
         nodeIdMap.addElement(edges[i].first);
         nodeIdMap.addElement(edges[i].second);
      }

      int const nNodes = nodeIdMap.size();

      using namespace boost;

      typedef int Vertex;
      typedef pair<int, int> E;
      typedef pair<double, E> Edge;

      priority_queue<Edge> Q;
      for (size_t k = 0; k < edges.size(); ++k)
      {
         int const    m0 = nodeIdMap.toCompressed(edges[k].first);
         int const    m1 = nodeIdMap.toCompressed(edges[k].second);
         double const w  = weights[k];

         Q.push(make_pair(w, make_pair(m0, m1)));
      } // end for (k)

      typedef std::vector<size_t> RankMap;
      typedef std::vector<Vertex> ParentMap;

      RankMap   rank_map(nNodes);
      ParentMap parent_map(nNodes);

      typedef PropertyMapFromVector<size_t> Rank;
      typedef PropertyMapFromVector<Vertex> Parent;

      Rank rank(rank_map);
      Parent parent(parent_map);

      disjoint_sets<Rank, Parent> dset(rank, parent);
      for (Vertex i = 0; i < nNodes; ++i) dset.make_set(i);

      mstEdges.clear();

      while (!Q.empty())
      {
         Edge e = Q.top(); // Note that this extracts the edge with the largest weight (priority queue)
         Q.pop();
         Vertex u = dset.find_set(e.second.first);
         Vertex v = dset.find_set(e.second.second);
         if (u != v) // No circle condition
         {
            mstEdges.push_back(e.second);
            //cout << "Adding MST edge (" << e.second.first << ", " << e.second.second << ") with weight " << e.first << endl;
            dset.link(u, v);
         }
      } // end while

      // Detect connected components in the MST to obtain individual reconstructions.
      connComponents.clear();
      typedef adjacency_list <vecS, vecS, undirectedS> Graph;
      Graph G;

      for (size_t i = 0; i < mstEdges.size(); ++i)
      {
         int view1 = mstEdges[i].first;
         int view2 = mstEdges[i].second;
         add_edge(view1, view2, G);
      }

      vector<int> componentMap(num_vertices(G));
      int const nComponents = connected_components(G, &componentMap[0]);

      // Bring ids to original domain
      for (size_t i = 0; i < mstEdges.size(); ++i)
      {
         int n1 = mstEdges[i].first;
         int n2 = mstEdges[i].second;
         mstEdges[i].first  = nodeIdMap.toOrig(n1);
         mstEdges[i].second = nodeIdMap.toOrig(n2);
      }

      connComponents.resize(nComponents);

      for (size_t i = 0; i < componentMap.size(); ++i)
      {
         int component = componentMap[i];
         connComponents[component].insert(nodeIdMap.toOrig(i));
      }
   } // end getMinimumSpanningForest()

} // end namespace V3D
