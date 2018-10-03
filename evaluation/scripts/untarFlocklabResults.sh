#!/bin/bash

#
# This script extracts all FlockLab results archives that are contained in the current working directory
# For each such archive, the script creates a subfolder named after the basename of the archive
#
# Author: Roland Kluge
# Date: 2018-10-01
#

ruler="----------------------------------------------------------------------------------------------------"

areThereTarGzFiles=$(ls -la | grep .tar.gz)
[ "$areThereTarGzFiles" == "" ] && {
  echo "No .tar.gz files in this folder. Stopping."
  exit
}

for file in *.tar.gz;
do
  resultsParentFolder=${file%.tar.gz}
  ruler="----------------------------------------------------------------------------------------------------"
  echo "Extracting $file to $resultsParentFolder"
  ruler="----------------------------------------------------------------------------------------------------"
  
  mkdir -p $resultsParentFolder
  cp $file $resultsParentFolder
  tar -C $resultsParentFolder -xzf $file
  rm $resultsParentFolder/$file
  
  resultsSubfolder=./$(/usr/bin/find $resultsParentFolder -mindepth 1 -maxdepth 1 -type d)
  
  echo $resultsSubfolder
  [ -d "$resultsSubfolder" ] || {
    echo "Unable to find results folder" 
    exit
  }
  
  mv $resultsSubfolder/* $resultsParentFolder
  rm -rf $resultsSubfolder
  
  serialFile=$resultsParentFolder/serial.csv
  algorithm=$(grep "topologycontrol: " $serialFile | head -1 | sed -r "s/^.*topologycontrol: (.*)'.*/\1/g")
  echo "  Sanity check: topology control algorithm = '$algorithm'"
  degree=$(grep "DEGREE: " $serialFile | head -1)
  echo "  Sanity check: DEGREE = '$degree'"
  time=$(grep "TIME: " $serialFile | head -1)
  echo "  Sanity check: TIME = '$time'"
  errorCount=$(grep 'ERROR' $serialFile | wc -l)
  echo "  Number of ERROR lines: '$errorCount'"
done