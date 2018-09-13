#ifndef GRAPHANALYER_GRAPHANALYZER_HPP
#define GRAPHANALYER_GRAPHANALYZER_HPP
#include <Utilities.hpp>
namespace GRAPHANALYZER{
    OUTPUTFORMAT getOutputFormat(std::string formatString);
    INPUTFORMAT getInputFormat(std::string formatString);
    COMPARISONTYPE getComparisonType(std::string formatString);
}
#endif