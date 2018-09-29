#ifndef GRAPHANALYZER_INOUTDELTAGRAPH_HPP
#define GRAPHANALYZER_INOUTDELTAGRAPH_HPP
#include <DeltaGraph.hpp>
#include <vector>
#include <Edge.hpp>
namespace GRAPHANALYZER
{
/**
 * An In Out DeltaGraph will hold all edges of the original graph a and highlight edges that are also part of B.
 * This is thought to compare in and output topologies.
**/
class InOutDeltaGraph : public DeltaGraph
{
  public:
    virtual bool toDot(const std::string filename) const;

    virtual bool toGraphML(const std::string filename) const;

    virtual void calculateDeltaGraph(const Graph* a, const Graph* b);
  private:
    /**
     * The edges that were not present in both original graphs.
    **/
    std::vector<Edge *> m_unmarked_edges;
    /**
     * The edges that were not present in both original graphs.
    **/
    std::vector<Edge *> m_marked_edges;
};
}
#endif