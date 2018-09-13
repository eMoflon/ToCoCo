#ifndef GRAPHANALYZER_EDGE_CPP
#define GRAPHANALYZER_EDGE_CPP
#include <Edge.hpp>
namespace GRAPHANALYZER{

    Edge::Edge(const Node* source,const Node* target,const float costSourceTarget,const float costTargetSource):m_cost_source_target(costSourceTarget),m_cost_target_source(costTargetSource),m_source(source),m_target(target){
        if(costSourceTarget==costTargetSource)
            this->m_is_bidirectional=true;
        else this->m_is_bidirectional=false;
    }

    const float Edge::getCostSourceTarget()const{
        return this->m_cost_source_target;
    }

    const float Edge::getCostTargetSource()const{
        return this->m_cost_target_source;
    }

    const bool Edge::isBidirectional() const{
        return this->m_is_bidirectional;
    }

    const Node* Edge::getSource() const{
        return this->m_source;
    }

    const Node* Edge::getTarget() const{
        return this->m_target;
    }
    void Edge::setCostTargetSource(float cost){
      this->m_cost_target_source=cost;
      this->m_is_bidirectional=true;
    }
    const bool Edge::equals(const Edge* other)const{
        bool result=true;
        if(this->m_target->equals(other->getTarget())){
            //Same Direction
            result&=this->m_source->equals(other->getSource());
            result&=this->getCostSourceTarget()==other->getCostSourceTarget();
            result&=this->getCostTargetSource()==other->getCostTargetSource();
            return result;

        }else if(this->getTarget()->equals(other->getSource())){
            //Inverted
            result&=this->m_source->equals(other->getTarget());
            result&=this->getCostSourceTarget()==other->getCostTargetSource();
            result&=this->getCostTargetSource()==other->getCostTargetSource();
            return result;
        } else return false;
    }
}
#endif