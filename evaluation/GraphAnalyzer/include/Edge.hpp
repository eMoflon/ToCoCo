#ifndef GRAPHANALYZER_EDGE_HPP
#define GRAPHANALYZER_EDGE_HPP

#include <Node.hpp>
namespace GRAPHANALYZER
{
class Edge
{

#define NOWEIGHT = NAN;

  public:
    /**
     * Creates an Edge Object. Also adds itself to the neighbors list of the source and target node.
    **/
    Edge(const Node* source, const Node* target,const float costSourceTarget,const float costTargetSource);
    /**
     * Returns the cost from the source to the target.
    **/
    const float getCostSourceTarget() const;
    /**
     * Returns the cost from the target to the source, NAN if this edge is unidirectional.
    **/
    const float getCostTargetSource() const;
    /**
     * Returns wether this edge is bidirectional.
    **/
    const bool isBidirectional() const;
    /**
     * Returns the source Node
    **/
    const Node *getSource() const;
    /**
     * Returns the target node.
    **/
    const Node *getTarget() const;
    /**
     * Sets the cost of the inverted direction. Makes the edge bidirectional.
    **/
    void setCostTargetSource(float cost);
    /**
     * Returns wether two edges are equal to each other.
     * Two edges are equal iff their source and targets nodes are equal, and the weights between them are equal to one and another. Note that Source and target might be swapped, but still Edges are equal
    **/
    const bool equals(const Edge* other)const;

  private:
    /**
     * The cost from source to target
    **/
    const float m_cost_source_target;
    /**
     * The cost from target to source
    **/
    float m_cost_target_source;
    /**
     * Marks whether the edge is bidirectional
    **/
    bool m_is_bidirectional;
    /**
     * Reference to the source node
    **/
    const Node* m_source;
    /**
     * Reference to the target node
    **/
    const Node* m_target;
};
}

#endif