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
$scriptDirectory/makeSerialOutputProperCsv.sh \
$scriptDirectory/compareResultsInOut.sh \
$scriptDirectory/collectTopologyControlRuntime.sh \
$scriptDirectory/plotTopologyEvolution.sh \
)

bash $scriptDirectory/untarFlocklabResults.sh


for scriptForEachFolder in ${scriptsForEachFolder[@]}
do
  for folder in $(/usr/bin/find . -maxdepth 1 -mindepth 1 -type d)
  do
    serialFile="$folder/serial.csv"
    if [ -f "$serialFile" ]
    then
      echo $ruler
      echo "Processing folder $folder"
      echo $ruler
      
        cmd="$scriptForEachFolder $serialFile"
        echo $cmd
        $cmd
        [ "$?" != "0" ] && {
          echo "Failure during $cmd"
          exit 1
        }
    else
      echo "Skipping folder $folder because no serial.csv file exists in it."
    fi
  done
done

echo $ruler
echo "Success"
echo $ruler
