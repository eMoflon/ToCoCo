#!/bin/bash

#
# This script iterates over all subfolders of the current directory and 
# invokes each script in the array $scriptsForEachFolder with serial.csv as single parameter
#
# Author: Roland Kluge
# Date: 2018-09-18
#

scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ruler="----------------------------------------------------------------------------------------------------"

scriptsForEachFolder=(\
$scriptDirectory/compareResultsInOut.sh \
$scriptDirectory/makeSerialOutputProperCsv.sh \
$scriptDirectory/collectTopologyControlRuntime.sh \
$scriptDirectory/plotTopologyEvolution.sh \
)

bash $scriptDirectory/untarFlocklabResults.sh

for folder in $(/usr/bin/find . -maxdepth 1 -mindepth 1 -type d)
do
  serialFile="$folder/serial.csv"
  if [ -f "$serialFile" ]
  then
    echo $ruler
    echo "Processing folder $folder"
    echo $ruler
    
    for scriptForEachFolder in ${scriptsForEachFolder[@]}
    do
      cmd="$scriptForEachFolder $serialFile"
      echo $cmd
      $cmd
      [ "$?" != "0" ] && {
        echo "Failure during $cmd"
        exit 1
      }
    done
  else
    echo "Skipping folder $folder because no serial.csv file exists in it."
  fi
done

echo $ruler
echo "Success"
echo $ruler
