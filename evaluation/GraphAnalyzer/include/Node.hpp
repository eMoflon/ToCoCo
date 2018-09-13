#ifndef GRAPHANALYZER_NODE_HPP
#define GRAPHANALYZER_NODE_HPP
#include <vector>
#include <string>

namespace GRAPHANALYZER
{
class Node
{
  public:
    /**
     * Creates a new Node object. Edges are supposed to be added later.
    **/
    Node(const int id, const float xPos, const float yPos);
    /**
     * Returns the ID
    **/
    const int getID() const;
    /**
     * returns the position in x direction
    **/
    const float getXPos() const;
    /**
     * returns the position in y direction
    **/
    const float getYPos() const;
    /**
     * Compares this to another Node, based on its ID.
    **/
    const bool equals(const Node* other)const;

  private:
    /**
     * the position in x direction
    **/
    const float m_x_pos;
    /**
     * the position in y direction
    **/
    const float m_y_pos;
    /**
     * the node id
    **/
    const int m_id;
};
}

#endif