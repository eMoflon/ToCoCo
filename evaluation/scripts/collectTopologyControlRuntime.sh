#!/bin/bash

# This script produces a CSV that summarizes the topology control runtime per mote
#
# Usage: Pass the serial.csv file to analyze as first parameter
#
# Author: Roland Kluge
#

# We use the sanitized version of the serial output file
inputFile="${1%.*}_excel.csv"

[ -f $1 ] || {
echo "Input file $inputFile does not exist"
exit 1
}


algorithm=$(grep "topologycontrol: " $inputFile | head -1 | sed -r "s/^.*topologycontrol: (.*)'.*/\1/g")
outputFolder="$(dirname $inputFile)/output"
mkdir -p $outputFolder
outputFile="$outputFolder/runtime_${algorithm}.csv"
parentFolderName="$(cd $(dirname $inputFile); basename $(pwd))"

echo "testId;algo;nodeId;runtimeMillis" > $outputFile
grep -a "TIME:" $inputFile | \
  cut -d";" -f3,4 | \
  /usr/bin/sort --numeric-sort | \
  sed -r 's/"\[topologycontrol\]: TIME: ([[:digit:]]+) .*\"/\1/g' >> $outputFile

#Prepend test ID and algorithm name to each row (starting with row 2)
sed -i "2,\$s|^|${parentFolderName};${algorithm};|" $outputFile

collectedRuntimesFile="$(dirname $inputFile)/../_collectedResults/runtime.csv"

[ -f "$collectedRuntimesFile" ] || {
  echo "Creating file for collected results."
  mkdir -p $(dirname $collectedRuntimesFile)
  echo "testId;algo;nodeId;runtimeMillis" > $collectedRuntimesFile
}

# Copy content of outputFile to collected results file and skip the header
tail -n +2 $outputFile >> $collectedRuntimesFile
