#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="n/a"
depDirName="mystborn_GenericDataStructures"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/meta-leap/GenericDataStructures/archive/refs/heads/master.zip
unzip .tmp.zip
rm -f .tmp.zip
mv GenericDataStructures-master $depDirName
