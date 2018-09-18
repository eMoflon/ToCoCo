workingDirectory=$PWD
DIR=${BASH_SOURCE[0]}
scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

evaluation=$1

[ -f $1 ] || {
echo "Expect Serial input file as first parameter"
exit 1
}

evalDirectory=$(dirname $1)/output
echo "Evaluation directory: $evalDirectory"
mkdir -p $evalDirectory

inputFormat="-eval"
outputFormat="-dot"
output=03_TopologyDifference.dot
comparison="-io"
testbed="flocklab"
beforeTimeMinutes=10
afterTimeMinutes=14

echo "Comparing $evaluation at minute $beforeTimeMinutes with $evaluation at minute $afterTimeMinutes"
$scriptDirectory/../cli.php --testbed $testbed --evaluation graph-text --source $evaluation --destination $evalDirectory/evalA.txt --graph-minute $beforeTimeMinutes
$scriptDirectory/../cli.php --testbed $testbed --evaluation graph-text --source $evaluation --destination $evalDirectory/evalB.txt --graph-minute $afterTimeMinutes

cd $scriptDirectory/../GraphAnalyzer
make
cd $workingDirectory

$scriptDirectory/../GraphAnalyzer/graphanalyzer $inputFormat $evalDirectory/evalA.txt $evalDirectory/evalB.txt $comparison $outputFormat $evalDirectory/$output

echo "Generating PNG file "${output%%.*}".png"
dot -Tpng -Kneato -n -o "$evalDirectory/${output%%.*}.png" $evalDirectory/$output

$scriptDirectory/../cli.php --destination $evalDirectory/01_TopologyBefore.png --evaluation graph-neighborhood --graph-minute 10 --source $evaluation --testbed $testbed
$scriptDirectory/../cli.php --destination $evalDirectory/02_TopologyAfter.png --evaluation graph-neighborhood --graph-minute 14 --source $evaluation --testbed $testbed
