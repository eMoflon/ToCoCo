#ifndef GRAPHANALYZER_NODE_CPP
#define GRAPHANALYZER_NODE_CPP
#include <Node.hpp>
namespace GRAPHANALYZER{
    Node::Node(const int id, const float xPos, const float yPos):m_x_pos(xPos),m_y_pos(yPos),m_id(id){};

    const int Node::getID() const{
        return this->m_id;
    }

    const float Node::getXPos() const{
        return this->m_x_pos;
    }
    
    const float Node::getYPos() const{
        return this->m_y_pos;
    }

    const bool Node::equals(const Node* other)const{
        return this->m_id==other->getID();
    }
}
#endif