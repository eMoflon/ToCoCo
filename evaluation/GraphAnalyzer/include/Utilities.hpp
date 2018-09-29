#ifndef GRAPHANALYZER_UTILITIES_HPP
#define GRAPHANALYZER_UTILITIES_HPP
namespace GRAPHANALYZER{
    enum INPUTFORMAT{
        INPUTFORMAT_DOT,
        INPUTFORMAT_GRAPHML,
        INPUTFORMAT_EVAL
    };

    enum OUTPUTFORMAT{
        OUTPUTFORMAT_DOT,
        OUTPUTFORMAT_GRAPHML
    };

    enum COMPARISONTYPE{
        COMPARISONTYPE_ABCOMPARISON,
        COMPARISONTYPE_INOUTCOMPARISON
    };
}
#endif