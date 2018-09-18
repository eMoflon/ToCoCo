#!/bin/bash

#
# This script iterates over all subfolders of the current directory (i.e., ".") and 
# invokes the script compareResultsInOut.sh for each folder that contains a serial.csv file
#
# Author: Roland Kluge
# Date: 2018-09-18
#

scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
scriptForEachFolder="$scriptDirectory/compareResultsInOut.sh"
ruler="----------------------------------------------------------------------------------------------------"

for folder in $(/usr/bin/find . -maxdepth 1 -mindepth 1 -type d)
do
  serialFile="$folder/serial.csv"
  if [ -f "$serialFile" ]
  then
    echo $ruler
    echo "Processing folder $folder"
    echo $ruler
    $scriptForEachFolder "$serialFile"
    [ "$?" != "0" ] && exit 1
  else
    echo "Skipping folder $folder because no serial.csv file exists in it."
  fi
done

echo $ruler
echo "Success"
echo $ruler
