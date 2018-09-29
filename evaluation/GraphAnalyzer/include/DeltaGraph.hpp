#ifndef GRAPHANAYLZER_DELTAGRAPH_HPP
#define GRAPHANAYLZER_DELTAGRAPH_HPP
#include <Graph.hpp>
namespace GRAPHANALYZER
{
/**
 * A DeltaGraph is a special case of graph that was constructed merging two other graphs. It holds information over nodes and edges that were not present in both original graphs.
**/
class DeltaGraph : public Graph
{
  public:
    /**
     * Outputs the graph as Dot file. Returns true on success false otherwise.
    **/
    virtual bool toDot(const std::string filename) const=0;
    /**
     * Outputs the graph as GraphML file. Returns true on success false otherwise.
    **/
    virtual bool toGraphML(const std::string filename) const=0;
    /**
     * Calculates a Delta Graph from a and b and stores it in this.
    **/
    virtual void calculateDeltaGraph(const Graph* a, const Graph* b) =0;

  private:
  /*
   * indicates that the delta graph was calculated.
  **/
    bool m_calculated;
};
}
#endif