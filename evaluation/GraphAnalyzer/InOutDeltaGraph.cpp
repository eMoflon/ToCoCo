#ifndef GRAPHANALYZER_INOUTDELTAGRAPH_CPP
#define GRAPHANALYZER_INOUTDELTAGRAPH_CPP
#include<InOutDeltaGraph.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
namespace GRAPHANALYZER{
    bool InOutDeltaGraph::toDot(const std::string filename) const{
        std::ofstream stream(filename.c_str());
        stream<<"digraph {"<<std::endl;
        for(const Node* n: this->getNodes()){
            stream<<"\t"<<this->drawNode(n)<<std::endl;
        }
        for(const Edge* e: this->m_unmarked_edges){
            stream<<"\t"<<drawEdge(e,"gray")<<std::endl;
        }
        for(const Edge* e: this->m_marked_edges){
            stream<<"\t"<<drawEdge(e,"black")<<std::endl;
        }
        stream<<"}"<<std::endl;
        stream.close();
        return true;
    }

    bool InOutDeltaGraph::toGraphML(const std::string filename)const{
        //TODO: implement
        std::cout<<"Not yet implemented"<<std::endl;
        return false;
    }

    void InOutDeltaGraph::calculateDeltaGraph(const Graph* a, const Graph* b){
        for(const Node* node : a->getNodes()){
            this->addNode(new Node(node->getID(),node->getXPos(),node->getYPos()));
        }
        for(const Node* node: this->getNodes()){
            for(const Edge* edge: a->getEdgesFrom(node)){
                Edge* e= new Edge(node,this->getNode(edge->getTarget()->getID()),edge->getCostSourceTarget(),edge->getCostTargetSource());
                if(b->contains(e)){
                    this->m_marked_edges.push_back(e);
                }
                else this->m_unmarked_edges.push_back(e);
            }
        }
    }

}
#endif