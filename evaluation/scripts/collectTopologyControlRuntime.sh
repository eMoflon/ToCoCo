#!/bin/bash

[ -f $1 ] || {
echo "Input file $inputFile does not exist"
exit 1
}

inputFile="${1%.*}_excel.csv"

algorithm=$(grep "topologycontrol: " $inputFile | head -1 | sed -r "s/^.*topologycontrol: (.*)'.*/\1/g")
outputFolder="$(dirname $inputFile)/output"
mkdir -p $outputFolder
outputFile="$outputFolder/topologyControlRuntime.csv"

echo "00algo;nodeId;runtimeMillis" > $outputFile
grep -a "TIME:" $inputFile | cut -d";" -f2,3 | sed -r 's/".*: TIME: ([[:digit:]]+),"/\1/g;s/^([[:digit:]]);/0\1;/g' | sort >> $outputFile
sed -i 's/00algo/algo/' $outputFile
sed -i "2,\$s|^|${algorithm};|" $outputFile
