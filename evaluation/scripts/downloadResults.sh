#!/bin/bash

# Downloads all results files for the XML configurations in the current folder
# The script extracts the test ID (currently a five-digit number) from the XML filename and the archive name corresponds to the XML file but with extension tar.gz.
# The FlockLab username and password are either prompted interactively or can be set permanently via the env. vars. FLOCKLAB_USER and FLOCKLAB_PASSWORD.
#
# Author: Roland Kluge
# Date: 2018-11-06
#

rulerSmall="----------"
flocklabWebdavPrefix=https://www.flocklab.ethz.ch/user/webdav/

for xmlFile in *.xml;
do
  echo ""
  echo "Processing $xmlFile."
  echo $rulerSmall
  
  testId=$(echo $xmlFile | sed -r 's/^([[:digit:]]+).*/\1/')
  [ "$testId" == "" ] && {
    echo "Could not extract a test ID from the file name $xmlFile."
    continue
  }
  
  archiveFilename="${xmlFile%.xml}.tar.gz"
  [ -f "$archiveFilename" ] && {
    echo "Archive file $archiveFilename already exists."
  } || {
        
    if [ "$flocklabUser" == "" ];
    then
      [ "$FLOCKLAB_USER" == "" ] && {
        read -p "FlockLab user please: " flocklabUser
      } || {
        flocklabUser=$FLOCKLAB_USER
      }

      [ "$FLOCKLAB_PASSWORD" == "" ] && {
        read -s -p "FlockLab password please: " flocklabPassword
        echo ""
      } || {
        flocklabPassword=$FLOCKLAB_PASSWORD
      }
    fi
    
    flocklabWebdavResultsUrl=$flocklabWebdavPrefix$testId/results.tar.gz
    # Query size of file without downloading the file
    archiveFileSize=$(curl -sI --user $flocklabUser:$flocklabPassword $flocklabWebdavResultsUrl | grep -i content-length | awk '{print $2}')
    [ "$archiveFileSize" == "" ] && {
      echo "No results available for $xmlFile yet."
    } || {    
      # Perform the actual download
      echo "curl https://www.flocklab.ethz.ch/user/webdav/$testId/results.tar.gz --output $archiveFilename"
      curl --user $flocklabUser:$flocklabPassword $flocklabWebdavResultsUrl --output $archiveFilename
    }
    
  }
done

echo ""
echo "Done."
echo $rulerSmall