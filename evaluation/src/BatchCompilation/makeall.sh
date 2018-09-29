#!/bin/bash

#
# This script compiles binary images for all topology control algorithms of the cMoflon evaluation
# It can be easily adjusted by copying one of the subsequent blocks.
# 
# Author: Roland Kluge
# Date: 2017-03-02

cwd=$(pwd)

rootOfSource=../../../src
outputFolder=$rootOfSource/output/BatchCompilation
target="sky"
applicationConstantDefinitions="$rootOfSource/app-conf-constants.h"

# Adjust the following list according to the COMPONENT_TOPOLOGYCONTROL constants in $applicationConstantDefinitions
algorithmIdentifiers=(\
NULL \
AKTC \
LKTC \
LMST \
CMOFLONDEMOLANGUAGE_LSTARKTCALGORITHM \
CMOFLONDEMOLANGUAGE_LSTARKTCALGORITHM \
CMOFLONDEMOLANGUAGE_KTCALGORITHM)

mkdir -p $outputFolder
rm $outputFolder/*

for algorithmIdentifier in ${algorithmIdentifiers[@]};
do

  echo "================================================================================================================="
  echo "= Compiling for $algorithmIdentifier"
  echo "==="

  imageFile="$outputFolder/$algorithmIdentifier.$target"
  implementationFile=$(grep "COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_${algorithmIdentifier}" $applicationConstantDefinitions  | cut -d" " -f3)
  appConfSelector=$(grep "COMPONENT_TOPOLOGYCONTROL_${algorithmIdentifier}" $applicationConstantDefinitions | cut -d" " -f3)

  [ "" == "$implementationFile" ] && echo "WARN: Implementation file not found!"
  [ "" == "$appConfSelector" ] && echo "ERROR: Algorithm ID not found!" && exit

  cmd="make -C $rootOfSource clean TARGET=$target"
  echo "$cmd"
  $cmd

  cmd="make -C $rootOfSource app.$target TARGET=$target TOPOLOGYCONTROL_PREDEFINED_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector"
  echo "$cmd"
  $cmd

  [ "$?" == "0" ] && {
    echo "----> SUCCESS for $algorithmIdentifier<----"
    cp $rootOfSource/app.$target $imageFile
  } || {
    echo "----> FAILURE for $algorithmIdentifier <----"
    exit
  }
done

