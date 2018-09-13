#ifndef GRAPHANALYZER_ABDELTAGRAPH_CPP
#define GRAPHANALYZER_ABDELTAGRAPH_CPP
#include<ABDeltaGraph.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

namespace GRAPHANALYZER{
    bool ABDeltaGraph::toDot(const std::string filename) const{
        std::ofstream stream(filename.c_str());
        stream<<"digraph {"<<std::endl;
        std::cout<<this->m_unmarked_edges.size()<<" unmarked edges"<<std::endl;
        std::cout<<this->m_marked_edges_1.size()<<" edges that only exist in Graph A"<<std::endl;
        std::cout<<this->m_marked_edges_2.size()<<" edges that only exist in Graph B"<<std::endl;
        for(const Node* n: this->getNodes()){
            stream<<"\t"<<this->drawNode(n)<<std::endl;
        }
        for(const Edge* e: this->m_unmarked_edges){
            stream<<"\t"<<drawEdge(e,"gray")<<std::endl;
        }
        for(const Edge* e: this->m_marked_edges_1){
            stream<<"\t"<<drawEdge(e,"blue")<<std::endl;
        }
        for(const Edge* e: this->m_marked_edges_2){
            stream<<"\t"<<drawEdge(e,"red")<<std::endl;
        }
        stream<<"}"<<std::endl;
        stream.close();
        return true;
    }

    bool ABDeltaGraph::toGraphML(const std::string filename)const{
        //TODO:implement
        std::cout<<"GraphMl Output is currently not supported"<<std::endl;
        return false;
    }

    void ABDeltaGraph::calculateDeltaGraph(const Graph* a, const Graph* b){
        for(const Node* node : a->getNodes()){
            this->addNode(new Node(node->getID(),node->getXPos(),node->getYPos()));
        }
        for(const Node* node: this->getNodes()){
            for(const Edge* edge: a->getEdgesFrom(node)){
                Edge* e=new Edge(node,this->getNode(edge->getTarget()->getID()),edge->getCostSourceTarget(),edge->getCostTargetSource());
                //this->insertEdge(e);
                if(b->contains(edge)){
                    this->m_unmarked_edges.push_back(e);
                }else{
                    this->m_marked_edges_1.push_back(e);
                }
            }
        }
        for(const Node* node: this->getNodes()){
            for(const Edge* edge: b->getEdgesFrom(node)){
                Edge* e=new Edge(node,this->getNode(edge->getTarget()->getID()),edge->getCostSourceTarget(),edge->getCostTargetSource());
                //this->insertEdge(e);
                if(!a->contains(edge)){
                    this->m_marked_edges_2.push_back(e);
                }
            }
        }
    }

}
#endif