#ifndef GRAPHANAYLZER_ABDELTAGRAPH_HPP
#define GRAPHANAYLZER_ABDELTAGRAPH_HPP
#include <DeltaGraph.hpp>
#include <vector>
#include <Edge.hpp>
namespace GRAPHANALYZER
{
/**
 * An AB Deltagraph holds all edges from the graphs A and B and highlights those that are not part of both.
**/
class ABDeltaGraph : public DeltaGraph
{
  public:

    virtual bool toDot(const std::string filename) const;
    
    virtual bool toGraphML(const std::string filename) const;
    
    virtual void calculateDeltaGraph(const Graph* a, const Graph* b);

  private:
    /**
     * The edges that were part of A but not B
    **/
    std::vector<Edge*> m_marked_edges_1;
    /**
     * The edges that were part of B but not A
    **/
    std::vector<Edge*> m_marked_edges_2;
    /**
     * The edges that were part of both A and B
    **/
    std::vector<Edge*> m_unmarked_edges;
};
}
#endif