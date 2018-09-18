#/bin/bash

# This script takes a serial output file of the ToCoCo framework and produces a proper CSV file
# It wraps the raw output line per timestamp into quotes to ensure a correct import, e.g., in Excel.
#
# Author: Roland Kluge
# Date: 2018-09-18
#

[ -f $1 ] || {echo "File does not exist"; exit 1}

inputFile=$1
outputFile="$1_excel.csv"

# Columns
# 1 timestamp
# 2 observer ID (skipped)
# 3 node ID
# 4 direction (skipped)
# 5 output (may contain commas, therefore, we read the rest of the line and enclose it with quotes)
#
awk -F, -- '{printf "%s,%s,\"",$1,$3; for (i=5; i<=NF; i++) printf ("%s,",$i); print "\"";}' $inputFile > $outputFile

