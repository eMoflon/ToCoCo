#!/bin/bash

#
# This creates a CSV table of the sensor images in the current folder
# The sensor image files must end with .sky
#
# Author: Roland Kluge
# Date: 2018-10-01
#

target="sky"
outputFolder=output
rawFile=$outputFolder/size_raw.txt
csvFile=$outputFolder/size.csv

[ "$(ls | grep '.$target')" == "" ] || {
  echo "No sensor images in $(pwd)"
  exit
}

mkdir -p $outputFolder

size *.$target > $rawFile
sed -r 's/^ *//;s/ *$//;s/\s+/;/g' < $rawFile > $csvFile
rm $rawFile
