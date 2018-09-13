workingDirectory=$PWD
evalDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $evalDirectory
DIR=${BASH_SOURCE[0]}
evaluation=$1
inputFormat="-eval"
outputFormat="-dot"
output=$2
comparison="-io"
echo "Comparing $evaluation at minute 10 with $evaluation at minute 14"
$evalDirectory/../cli.php --testbed flocklab --evaluation graph-text --source $evaluation --destination $evalDirectory/output/evalA.txt --graph-minute 10
$evalDirectory/../cli.php --testbed flocklab --evaluation graph-text --source $evaluation --destination $evalDirectory/output/evalB.txt --graph-minute 14
if [ ! -f $evalDirectory/../GraphAnalyzer/graphanalyzer ]; then
    cd $evalDirectory/../GraphAnalyzer && make
    cd $workingDirectory
fi
$evalDirectory/../GraphAnalyzer/graphanalyzer $inputFormat $evalDirectory/output/evalA.txt $evalDirectory/output/evalB.txt $comparison $outputFormat $output
echo "Generating PNG file "${output%%.*}".png"
dot -Tpng -Kneato -n $output -o "${output%%.*}".png
$evalDirectory/../cli.php --destination $evalDirectory/output/ktc_before.png --evaluation graph-neighborhood --graph-minute 10 --source $evaluation --testbed flocklab
$evalDirectory/../cli.php --destination $evalDirectory/output/ktc_after.png --evaluation graph-neighborhood --graph-minute 14 --source $evaluation --testbed flocklab