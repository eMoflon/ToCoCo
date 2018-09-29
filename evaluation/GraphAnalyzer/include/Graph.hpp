#ifndef GRAPHANALYZER_GRAPH_HPP
#define GRAPHANALYZER_GRAPH_HPP

#include <Edge.hpp>
#include <Node.hpp>
#include <string>
#include <map>
#include <vector>
#include <Utilities.hpp>
namespace GRAPHANALYZER
{
class Graph
{
  public:
    /**
     * Constructs an empty graph object. 
    **/
    Graph();
    /**
     * adds a node to the graph
    **/
    void addNode(Node* node);
    /**
     * Returns all the Nodes in the Graph as a vector.
    **/
    const std::vector<const Node*> getNodes() const;
    /**
     * Returns a reference to a node with ID nodeID 
    **/
    const Node *getNode(const int nodeID) const;
    /**
     * Gets all Edges that are outbound from sourceNode. Assumes sourceNode is part of the Graph.
    **/
    const std::vector<const Edge*> getEdgesFrom(const Node* sourceNode)const;
    /**
     * Gets the edge from sourceNode to targetNode if there is one. Returns true on success and stores the edge in result, otherwise false
    **/
    bool getEdgeFromTo(const Node* sourceNode,const Node* targetNode,Edge** result)const;
    /**
     * Inserts an Edge into the Graph. If the edge is already inserted it returns. If the edge is part of the graph but in inverse direction it makes the edge bidirectional if it isn't yet. In all other cases e is inserted.
    **/
    void insertEdge(Edge* e);

    /**
     * Reads a graph from fileAddr in the format passed.
    **/
    static Graph* readGraph(INPUTFORMAT format,std::string fileAddr);

    /**
     * Writes the graph to fileAddr in the format passed.
    **/
    virtual bool writeGraph(OUTPUTFORMAT format,std::string fileAddr)const;

    bool contains(const Edge* e)const;

  protected:
    /**
     * Creates the output line for an edge with a given colour
    **/
    std::string drawEdge(const Edge* e,std::string colour="black")const;
    /**
     * Creates the output line for a node.
    **/
    std::string drawNode(const Node* n)const;
    /**
     * Map from node IDs to node references.
    **/
    std::map<int, Node*> m_nodes;
    /**
     * Maps the node Id of the source node to a map that maps the id of the target node to the edge from source to target.
    **/
    std::map<int,std::map<int, Edge*>> m_edges;
    /**
     * Searches the graph to an edge that is equals to original and stores it in target. Returns true on success, false otherwise
    **/
  private:
    /**
     * Parses an Edge from the stream. The stream is supposed to consist of 4 ints only.
    **/
    bool parseEdge(std::istringstream &stream);
    /**
     * Parses a Node from the stream. The stream is supposed to consist of 3 ints only.
    **/
    bool parseNode(std::istringstream &stream);
    /**
     * Reads a graph from a Dot file, and stores it in destination. Returns true on success, false otherwise
    **/
    static bool fromDot(const std::string filename,Graph *destination);
    /**
     * Reads a graph from a GraphMl file, and stores it in destination. Returns true on success, false otherwise
    **/
    static bool fromGraphMl(const std::string filename, Graph *destination);
    /**
     * Reads a graph from a TC Evaluation file, and stores it in destination. Returns true on success, false otherwise
    **/
    static bool fromEvalOutput(const std::string filename, Graph *destination);
    /**
     * Outputs the graph as Dot file. Returns true on success false otherwise.
    **/
    virtual bool toDot(const std::string filename) const;
    /**
     * Outputs the graph as GraphML file. Returns true on success false otherwise.
    **/
    virtual bool toGraphML(const std::string filename) const;
};
}

#endif