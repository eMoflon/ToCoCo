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

echo "00algo;nodeId;runtimeMillis" > $outputFile
grep -a "TIME:" $inputFile | cut -d";" -f2,3 | /usr/bin/sort --numeric-sort | sed -r 's/"\[topologycontrol\]: TIME: ([[:digit:]]+) .*\"/\1/g' >> $outputFile
sed -i 's/00algo/algo/' $outputFile
sed -i "2,\$s|^|${algorithm};|" $outputFile
