#!/bin/bash

# This script produces topology before-after snapshots for two given points in time
#
# Usage: Pass the serial.csv file to analyze as first parameter

workingDirectory=$PWD
DIR=${BASH_SOURCE[0]}
scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

evaluation=$1

[ -f $1 ] || {
echo "Expect serial input file as first parameter, but was $1 in $(pwd)"
exit 1
}

evalDirectory=$(dirname $1)/output
echo "Evaluation directory: $evalDirectory"
mkdir -p $evalDirectory

cliScript=$scriptDirectory/../cli.php
inputFormat="-eval"
outputFormat="-dot"
output=03_TopologyDifference.dot
comparison="-io"
testbed="flocklab"
sourceAndTestbedStatement="--source $evaluation --testbed $testbed"
beforeTimeMinutes=10
afterTimeMinutes=20
topologyBeforeDotFile=$evalDirectory/01_TopologyBefore_Time${beforeTimeMinutes}.txt
topologyAfterDotFile=$evalDirectory/02_TopologyAfter_Time${afterTimeMinutes}.txt
topologyBeforePlotFile=$evalDirectory/01_TopologyBefore_Time${beforeTimeMinutes}.png
topologyAfterPlotFile=$evalDirectory/02_TopologyAfter_Time${afterTimeMinutes}.png

echo "Comparing $evaluation at minute $beforeTimeMinutes with $evaluation at minute $afterTimeMinutes"
$cliScript $sourceAndTestbedStatement --evaluation graph-text --destination $topologyBeforeDotFile  --graph-minute $beforeTimeMinutes
$cliScript $sourceAndTestbedStatement --evaluation graph-text --destination $topologyAfterDotFile --graph-minute $afterTimeMinutes

# The following code produces the delta topology
cd $scriptDirectory/../GraphAnalyzer
make
cd $workingDirectory
$scriptDirectory/../GraphAnalyzer/graphanalyzer $inputFormat $topologyBeforeDotFile $topologyAfterDotFile $comparison $outputFormat $evalDirectory/$output
echo "Generating PNG file "${output%%.*}".png"
dot -Tpng -Kneato -n -o "$evalDirectory/${output%%.*}.png" $evalDirectory/$output

$cliScript --destination $topologyBeforePlotFile --evaluation graph-neighborhood --graph-minute $beforeTimeMinutes $sourceAndTestbedStatement
$cliScript --destination $topologyAfterPlotFile --evaluation graph-neighborhood --graph-minute $afterTimeMinutes $sourceAndTestbedStatement

