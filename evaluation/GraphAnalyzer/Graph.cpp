#ifndef GRAPHANALYZER_GRAPH_CPP
#define GRAPHANALYZER_GRAPH_CPP
#include <Graph.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
namespace GRAPHANALYZER{
    Graph::Graph(){
        this->m_nodes.clear();
        this->m_edges= std::map<int,std::map<int,Edge*>>();
    }
    void Graph::addNode(Node* node){
        if(this->m_nodes.find(node->getID())==this->m_nodes.end()){
            this->m_nodes.insert(std::pair< int,Node*>(node->getID(),node));
            this->m_edges.insert(std::pair< int,std::map< int,Edge*>>(node->getID(),std::map< int,Edge*>()));
        }
    }
    const std::vector<const Node*> Graph::getNodes() const{
        std::vector<const Node*> nodes;
        std::transform(this->m_nodes.begin(),this->m_nodes.end(),std::back_inserter(nodes),[](const std::map<int,const Node*>::value_type &pair){return pair.second;});
        return nodes;
    }
    const Node* Graph::getNode(const int nodeID)const{
        auto searchResult=this->m_nodes.find(nodeID);
        if(this->m_nodes.end()!=searchResult)
            return searchResult->second;
        else return nullptr;
    }
    const std::vector<const Edge*> Graph::getEdgesFrom(const Node* sourceNode)const{
        std::vector<const Edge*> result;
        std::transform(this->m_edges.find(sourceNode->getID())->second.begin(),this->m_edges.find(sourceNode->getID())->second.end(),std::back_inserter(result),[](const std::map<const int,const Edge*>::value_type &pair){return pair.second;});
        return result;
    }
    bool Graph::getEdgeFromTo(const Node* sourceNode,const Node* targetNode,Edge** result)const{
        auto it=this->m_edges.find(sourceNode->getID());
        if(it!=this->m_edges.end()){
            auto it2= it->second.find(targetNode->getID());
            if(it2!=it->second.end()){
                *result=it2->second;
                return true;
            }
            else return false;
        }
        return false;
    }

    void Graph::insertEdge(Edge* e){
        Edge* e2=nullptr;
        //Case where edge is already inserted
        if(this->getEdgeFromTo(e->getSource(),e->getTarget(),&e2)){
            return;
        }
        //Case where edge is already inserted but in inverse direction
        if(this->getEdgeFromTo(e->getTarget(),e->getSource(),&e2)){
            if(!(e2->getCostTargetSource()==e->getCostSourceTarget())){
                e2->setCostTargetSource(e->getCostSourceTarget());
            }
            return;
        }
        //Case when we need to insert an edge.
        auto it=this->m_edges.find(e->getSource()->getID());
        it->second.insert(std::pair<int,Edge*>(e->getTarget()->getID(),e));
        return;
    }
    bool Graph::contains(const Edge* e)const{
        Edge* e2=nullptr;
        if(this->getEdgeFromTo(e->getSource(),e->getTarget(),&e2)){
            if(e->equals(e2))
                return true;
            else return false;
        }
        else if(this->getEdgeFromTo(e->getTarget(),e->getSource(),&e2)){
                if(e->equals(e2))
                    return true;
                else return false;
            }
        else return false;
    }
    bool Graph::fromDot(const std::string filename,Graph* destination){
        //TODO: implement
        std::cout<<"Dot Input is currently not supported"<<std::endl;
        return false;
    }
    bool Graph::fromGraphMl(const std::string filename,Graph* destination){
        //TODO: implement
        std::cout<<"GraphMl Input is currently not supported"<<std::endl;
        return false;
    }

    bool Graph::fromEvalOutput(const std::string filename, Graph *destination){
        std::ifstream input(filename);
        std::string line;
        std::istringstream iss;
        while(!input.eof()){
            getline(input,line,'\n');
            iss.str(line);
            std::string type;
            iss>>type;
            if(type.compare("EDGE")==0){
                if(!destination->parseEdge(iss)){
                    break;
                }
            }else if(type.compare("NODE")==0){
                if(!destination->parseNode(iss)){
                    break;
                }
            }else break;
        }
        if(iss.str().empty())
            return true;
        std::cout<<"Error while reading the input"<<std::endl;
        return false;
    }

    bool Graph::toDot(const std::string filename) const{
        std::ofstream stream(filename.c_str());
        stream<<"digraph\n{"<<std::endl;
        for(const Node* n: this->getNodes()){
            stream<<"\t"<<this->drawNode(n)<<std::endl;
        }
        for(const Node* n: this->getNodes()){
            for(const Edge* e : this->getEdgesFrom(n)){
                stream<<"\t"<<this->drawEdge(e)<<std::endl;
            }
        }
        stream<<"}"<<std::endl;
        stream.close();
        return true;
    }

    bool Graph::toGraphML(const std::string filename)const{
        std::cout<<"GraphMlOutput is currently not supported"<<std::endl;
        return false;
    }

    bool Graph::parseEdge(std::istringstream &iss){
        int node1,node2,weight1,weight2;
        iss>>node1;
        iss>>node2;
        iss>>weight1;
        iss>>weight2;
        this->insertEdge(new Edge(this->getNode(node1),this->getNode(node2),weight1==0?-1:weight1,weight2==0?-1:weight2));
        return true;
    }

    bool Graph::parseNode(std::istringstream &iss){
        int id,posX,posY;
        iss>>id;
        iss>>posX;
        iss>>posY;
        this->addNode(new Node(id,posX,posY));
        return true;
    }
    std::string Graph::drawEdge(const Edge* e,const std::string colour)const{
        std::stringstream ss;
        ss<<e->getSource()->getID()<<" -> "<<e->getTarget()->getID()<<" [color=\""<<colour<<"\"";
        if(e->isBidirectional()){
            ss<<", dir=\"none\"";
        }
        if(colour.compare("gray")!=0&&colour.compare("black")!=0){
            ss<<", label=\"("<<e->getCostSourceTarget()<<", ";
            ss<<e->getCostTargetSource()<<")\"";
        }
        ss<<"];";
        return ss.str();
    }

    std::string Graph::drawNode(const Node* n)const{
        std::stringstream ss;
        ss<<n->getID()<<" [shape = circle, width = \"0.4\", fixedsize = true, pos=\""<<(int)n->getXPos()<<","<<(int)-n->getYPos()<<"\"];";
        return ss.str();
    }

    Graph* Graph::readGraph(INPUTFORMAT format, std::string fileAddr){
        Graph* result=new Graph();
        bool success;
        if(format==INPUTFORMAT_DOT){
            success=Graph::fromDot(fileAddr,result);
        }else if(format==INPUTFORMAT_EVAL){
            success= Graph::fromEvalOutput(fileAddr,result);
        }else{
            success= Graph::fromGraphMl(fileAddr,result);
        }
        if(success){
            return result;
        }
        else{
            std::cout<<"Reading Graph could not be completed"<<std::endl;
            exit(-1);
            return nullptr;
        }
    }

    bool Graph::writeGraph(OUTPUTFORMAT format,std::string fileAddr)const{
        if(format==OUTPUTFORMAT_DOT){
            return this->toDot(fileAddr);
        }else return this->toGraphML(fileAddr);
    }
}
#endif