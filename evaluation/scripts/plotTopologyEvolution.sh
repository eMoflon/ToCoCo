#!/bin/bash

#
# Plots the evolution of the topology for a given range of start and end time
#
# Usage: Pass the serial.csv file to analyze as first parameter
#

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
testbed="flocklab"
sourceAndTestbedStatement="--source $evaluation --testbed $testbed"
startTimeMinutes=1
endTimeMinutes=20

for ((i=startTimeMinutes; i<=endTimeMinutes; i++))
do
  echo "Plotting for minute $i"
  
  plotFile=$evalDirectory/TopologySnapshotAt_$i.png
  textFile=$evalDirectory/TopologySnapshotAt_$i.textgraph
  $cliScript $sourceAndTestbedStatement --evaluation graph-text --destination $textFile --graph-minute $i 
  $cliScript $sourceAndTestbedStatement --evaluation graph-neighborhood --destination $plotFile --graph-minute $i 
done
