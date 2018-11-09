#/bin/bash

# This script takes a serial output file of the ToCoCo framework and produces a proper CSV file
# It wraps the raw output line per timestamp into quotes to ensure a correct import, e.g., in Excel.
#
# Author: Roland Kluge
# Date: 2018-09-18
#

[ -f $1 ] || {
echo "File $1 does not exist"
exit 1
}

inputFile=$1
outputFile="${1%.*}_excel.csv"

sed -i 's/^1541//g' $inputFile
startTime=$(head -2 $inputFile | tail -1 | awk -F, '{print $1}')

# Input Column format
# 1 timestamp
# 2 observer ID (skipped)
# 3 node ID
# 4 direction (skipped)
# 5 output (may contain commas, therefore, we read the rest of the line and enclose it with quotes)
# Output column format
# 1 timestamp
# 2 time in seconds (timestamp of row - timestamp of first row)
# 3 node ID
# 4 output (enclosed by ")
#
awkString="{printf(\"%s;%s;%s;\\\"\",\$1,\$1-${startTime},\$3); for (i=5; i<=NF; i++) printf (\"%s,\",\$i); print \"\\\"\";}"
awk -F, -- "$awkString" $inputFile > $outputFile

# Improve formatting of links. Node ids always have a trailing '.0' for some strange reason
sed -i -r 's/([[:digit:]]+)\.0[-~]>([[:digit:]]+)\.0/\1->\2/g' $outputFile
