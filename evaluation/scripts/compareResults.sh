workingDirectory=$PWD
evalDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIR=${BASH_SOURCE[0]}
evaluationA=$1
evaluationB=$2
time=14
inputFormat="-eval"
outputFormat="-dot"
output=$3
comparison="-ab"
echo "Comparing $evaluationA with $evaluationB"
$evalDirectory/../cli.php --testbed flocklab --evaluation graph-text --source $evaluationA --destination $evalDirectory/output/evalA.txt --graph-minute $time
$evalDirectory/../cli.php --testbed flocklab --evaluation graph-text --source $evaluationB --destination $evalDirectory/output/evalB.txt --graph-minute $time
if [ ! -f $evalDirectory/../GraphAnalyzer/graphanalyzer ]; then
    cd $evalDirectory/../GraphAnalyzer && make
    cd $workingDirectory
fi
$evalDirectory/../GraphAnalyzer/graphanalyzer $inputFormat $evalDirectory/output/evalA.txt $evalDirectory/output/evalB.txt $comparison $outputFormat $output
echo "Generating PNG file "${output%%.*}".png"
dot -Tpng -Kneato -n $output -o "${output%%.*}".png
$evalDirectory/../cli.php --destination $evalDirectory/output/ktc_m.png --evaluation graph-neighborhood --graph-minute $time --source $evaluationA --testbed flocklab
$evalDirectory/../cli.php --destination $evalDirectory/output/ktc_g.png --evaluation graph-neighborhood --graph-minute $time --source $evaluationB --testbed flocklab