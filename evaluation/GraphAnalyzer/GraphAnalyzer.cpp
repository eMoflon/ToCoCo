#ifndef GRAPHANALYZER_GRAPHANALYZER_CPP
#define GRAPHANALZYER_GRAPHANALZYER_CPP
#include <cstdlib>
#include <iostream>
#include <GraphAnalyzer.hpp>
#include <Graph.hpp>
#include <ABDeltaGraph.hpp>
#include <InOutDeltaGraph.hpp>
namespace GRAPHANALYZER{
    COMPARISONTYPE getComparisonType(std::string formatString){
        if(formatString.compare("-ab")==0){
            return COMPARISONTYPE_ABCOMPARISON;
        }else if(formatString.compare("-io")==0){
            return COMPARISONTYPE_INOUTCOMPARISON;
        }else{
            std::cout<<"Unknown comparison operator either comparison of two different output from one input topology (\"-ab\") or comparison of an input to an output topology(\"-io\") is supported"<<std::endl;
            exit(-1);
        }
    }

    INPUTFORMAT getInputFormat(std::string formatString){
        if(formatString.compare("-dot")==0){
            return INPUTFORMAT_DOT;
        }else if(formatString.compare("-graphML")==0){
            return INPUTFORMAT_GRAPHML;
        }else if(formatString.compare("-eval")==0){
            return INPUTFORMAT_EVAL;
        }else{
            std::cout<<"Unknown input format, either dot (\"-dot\"), GraphMl (\"-graphML\") or evaluation output (\"-eval\")are supported."<<std::endl;
            exit(-1);
        }
    }

    OUTPUTFORMAT getOutputFormat(std::string formatString){
        if(formatString.compare("-dot")==0){
            return OUTPUTFORMAT_DOT;
        }else if(formatString.compare("-graphML")==0){
            return OUTPUTFORMAT_GRAPHML;
        }else{
            std::cout<<"Unknown output format, either dot (\"-dot\") or GraphMl (\"-graphML\") are supported."<<std::endl;
            exit(-1);
        }
    }
}
using namespace GRAPHANALYZER;
int main(int argc, char*argv[]){
    if(argc!=7){
        std::cout<<"Not enough arguments: Expected 6, but got: "<<argc<<std::endl;
        return -1;
    }
    else{
        INPUTFORMAT inFormat=getInputFormat(std::string(argv[1]));
        COMPARISONTYPE compType=getComparisonType(std::string(argv[4]));
        OUTPUTFORMAT outputFormat=getOutputFormat(std::string(argv[5]));
        const Graph* a= Graph::readGraph(inFormat,std::string(argv[2]));
        const Graph* b= Graph::readGraph(inFormat, std::string(argv[3]));
        DeltaGraph* result;
        if(compType==COMPARISONTYPE_ABCOMPARISON){
            result= new ABDeltaGraph();
        }else{
            result=new InOutDeltaGraph();
        }
        std::cout<<"Calculating DeltaGraph"<<std::endl;
        result->calculateDeltaGraph(a,b);
        std::cout<<"Writing DeltaGraph to "<<argv[6]<<std::endl;
        result->writeGraph(outputFormat,std::string(argv[6]));
        return 1;
    }
}
#endif