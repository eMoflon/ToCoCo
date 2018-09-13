DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ -e filterLog ]
then
    rm filterLog
fi
cd ../GraphAnalyzer
rm -r *.o
if [ -e graphanalyzer ]
then
    rm graphanalyzer
fi