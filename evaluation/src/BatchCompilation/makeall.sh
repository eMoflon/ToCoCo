#!/bin/bash

#
# This script compiles binary images for all topology control algorithms of the cMoflon evaluation
# It can be easily adjusted by copying one of the subsequent blocks.
# 
# Author: Roland Kluge
# Date: 2017-03-02

cwd=$(pwd)

rootOfSource=../../../src
cd $rootOfSource

target="sky"
# 3rd algorithm

imageFile="lmstGen.$target"
implementationFile="topologycontrol-cMoflonDemoLanguage-LmstAlgorithm.c"
appConfSelector="7" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target

# 1st algorithm

imageFile="kTCGen.$target"
implementationFile="topologycontrol-cMoflonDemoLanguage-KtcAlgorithm.c"
appConfSelector="8" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target

# 2nd algorithm

imageFile="lkTCGen.$target"
implementationFile="topologycontrol-cMoflonDemoLanguage-LStarKtcAlgorithm.c"
appConfSelector="6" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target
exit
# 4th algorithm

imageFile="lmstMan.$target"
implementationFile="topologycontrol-lmst.c"
appConfSelector="4" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target

# 5th algorithm

imageFile="lktcMan.$target"
implementationFile="topologycontrol-lktc.c"
appConfSelector="3" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target

# 3rd algorithm

imageFile="ktcMan.$target"
implementationFile="topologycontrol-aktc.c"
appConfSelector="2" # According to app-conf-constants.h

echo "================================================================================================================="
echo "= Compiling for $imageFile"
echo "==="
make clean TARGET=$target
make app.$target TARGET=$target TC_IMPL_FILE=$implementationFile TOPOLOGYCONTROL_PREDEFINED_ALGORITHM=$appConfSelector
[ "$?" == "0" ] && echo "----> SUCCESS <----" || echo "----> FAILURE <----"
[ -f "app.$target" ] && cp app.$target $imageFile && rm app.$target


cd $cwd
