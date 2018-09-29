#!/bin/bash

# Script for automatically installing PHP composer
# Derived from: https://www.tutorialspoint.com/articles/how-to-install-and-configure-the-composer-on-ubuntu-16-04
#
# Preliminaries
# Package dependencies can be resolved as follows: 
#   sudo apt-get install curl php5-cli git graphviz


installationDirectory="/usr/local/bin"
tempFile="/tmp/composer-setup.php"

sudo php -r "copy('https://getcomposer.org/installer', '$tempFile');"
sudo php $tempFile --install-dir=$installationDirectory --filename=composer
composer --version
sudo rm -rf $tempFile

echo "Now use 'composer install' (within a directory that contains a 'composer.json') to install all required dependencies"
