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

counter=1
for file in *.tar.gz;
do
  resultsParentFolder=./${file%.tar.gz}
  
  [ -d "$resultsParentFolder" ] && {
    echo "Skipped $file because it has been extracted already."
  } || {
    ruler="----------------------------------------------------------------------------------------------------"
    echo "[$counter] Extracting $file to $resultsParentFolder"
    ruler="----------------------------------------------------------------------------------------------------"
    
    mkdir -p $resultsParentFolder
    cp $file $resultsParentFolder
    tar -C $resultsParentFolder -xzf $file
    rm $resultsParentFolder/$file
    
    echo $resultsParentFolder
    
    resultsSubfolder=$(/usr/bin/find $resultsParentFolder -mindepth 1 -type d)
    
    echo $resultsSubfolder
    [ -d "$resultsSubfolder" ] || {
      echo "Unable to find folder containing untarred results. Stop." 
      exit
    }
    
    mv $resultsSubfolder/* $resultsParentFolder
    rm -rf $resultsSubfolder
    
    serialFile=$resultsParentFolder/serial.csv
    algorithm=$(grep -a "topologycontrol: " $serialFile | head -1 | sed -r "s/^.*topologycontrol: (.*)'.*/\1/g")
    echo "  Sanity check: topology control algorithm = '$algorithm'"
    degree=$(grep -a "DEGREE: " $serialFile | head -1)
    echo "  Sanity check: DEGREE = '$degree'"
    time=$(grep -a "TIME: " $serialFile | head -1)
    echo "  Sanity check: TIME = '$time'"
    errorCount=$(grep -a 'ERROR' $serialFile | wc -l)
    echo "  Sanity check: Number of ERROR lines = '$errorCount'"
    
    startedNodes=$(grep "STATUS" $serialFile | awk -F, '{print $2}' | /usr/bin/sort -n)
    stoppedNodes=$(grep "Ticks:" $serialFile | awk -F, '{print $2}' | /usr/bin/sort -n)
    nodesThatDidNotStop=$(diff -- <(echo "$startedNodes") <(echo "$stoppedNodes") | tr "\n" " ")
    [ "$nodesThatDidNotStop" != "" ] && {
      warningPrefix="!! ===>"
    } || {
      warningPrefix="  "
    }
    echo "${warningPrefix}Sanity check: Are there nodes that did not stop? '${nodesThatDidNotStop}'"
    
  }
  
  

  
  counter=$[$counter +1]
done